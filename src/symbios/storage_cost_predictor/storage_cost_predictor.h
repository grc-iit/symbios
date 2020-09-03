//
// Created by mani on 8/24/2020.
//

//TODO: use configuration manager to get storage index

#ifndef SYMBIOS_STORAGE_COST_PREDICTOR_H
#define SYMBIOS_STORAGE_COST_PREDICTOR_H

#include <iostream>
#include <fstream>
#include <mpi.h>
#include <dlib/optimization.h>
#include <boost/filesystem/operations.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <symbios/server/daemon.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <math.h>
#include <common/debug.h>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>

#define LINREG_NPARAMS 3
#define MAX_METRIC_QUEUE 1024

class LinRegModel {
public:
    typedef std::array<double, LINREG_NPARAMS> CoeffArray;
    typedef boost::lockfree::spsc_queue<CoeffArray, boost::lockfree::capacity<1>> CoeffQueue;
private:
    typedef dlib::matrix<double, LINREG_NPARAMS, 1> parameter_vector;
    typedef dlib::matrix<double, LINREG_NPARAMS, 1> input_vector;
    typedef std::pair<input_vector, double> Observation;
    typedef boost::lockfree::spsc_queue<Observation, boost::lockfree::capacity<MAX_METRIC_QUEUE>> ObservationQueue;
    typedef std::vector<Observation> ObservationVec;
    size_t window_off_ = 0;

    ObservationQueue dataset_;
    CoeffQueue coeffs_q_;
    CoeffArray coeffs_[2];
    std::atomic_bool buffer_ = false;

    double Residual(const Observation &data, const parameter_vector &params) {
        const input_vector &x = data.first;
        double y = data.second;
        double sum = 0;
        int i = 0;
        for(i = 0; i < LINREG_NPARAMS; ++i) { sum += params(i) * x(i); }
        return std::pow(y - sum, 2);
    }

    ObservationVec GetWindow() {
        ObservationVec datavec;
        Observation obs;
        while(dataset_.pop(obs)) { datavec.emplace_back(std::move(obs)); }
        return std::move(datavec);
    }

    void UpdateCoeffs(const CoeffArray &coeffs) {
        bool buffer = !buffer_;
        for(int i = 0; i < LINREG_NPARAMS; ++i) { coeffs_[buffer][i] = (coeffs_[buffer][i] + coeffs[i])/2; }
        coeffs_q_.push(coeffs);
        buffer_ = buffer;
    }

public:
    LinRegModel() = default;

    void Fit(void) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("LinRegModel::FitData");
        ObservationVec datavec = GetWindow();
        if (datavec.size() > 0) {
            parameter_vector params(LINREG_NPARAMS);
            std::function<double(const Observation &, const parameter_vector &)> residual(std::bind(&LinRegModel::Residual, this, std::placeholders::_1, std::placeholders::_2));
            dlib::solve_least_squares_lm(
                    dlib::objective_delta_stop_strategy(1e-7),
                    residual,
                    dlib::derivative(residual),
                    datavec,
                    params);
            CoeffArray coeffs;
            for(int i = 0; i < LINREG_NPARAMS; ++i) { coeffs[i] = params(i); }
            UpdateCoeffs(coeffs);
        }
    }

    void Feedback(double bw, double nprocs, double read_bytes, double write_bytes) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("LinRegModel::Feedback");
        input_vector iv;
        iv(0) = nprocs;
        iv(1) = read_bytes;
        iv(2) = write_bytes;
        dataset_.push(std::move(Observation(iv, bw)));
    }

    double Predict(double nprocs, double read_bytes, double write_bytes) {
        bool buffer = buffer_;
        return coeffs_[buffer][0]*nprocs + coeffs_[buffer][1]*read_bytes + coeffs_[buffer][2]*write_bytes;
    }

    void UpdateCoeffs(double nprocs_coeff, double tot_read_coeff, double tot_write_coeff) {
        CoeffArray coeffs = {nprocs_coeff, tot_read_coeff, tot_write_coeff};
        UpdateCoeffs(coeffs);
        window_off_++;
    }

    CoeffQueue &GetCoeffQueue() {
        return coeffs_q_;
    }

    size_t &GetWindowOff() {
        return window_off_;
    }
};

class StorageCostPredictor {
private:
    std::string model_file_path_;
    MPI_File model_file_ = nullptr;
    int rank_ = 0, nprocs_ = 1;
    bool commit_metrics_ = true, csv_header_ = false;
    size_t window_size_ = 1024, window_tick_ = 0;
    std::unordered_map<std::string, LinRegModel> storage_models_;
    std::list<std::string> storage_configs_;
    std::thread worker_thread_;
    std::promise<void> terminate_worker_;

    int ParseInt(std::string &input, size_t &pos_in_file, size_t filesz) {
        std::string substr(input.data() + pos_in_file, filesz - pos_in_file);
        size_t cnt = 0;
        int i = std::stoi(substr, &cnt);
        pos_in_file += cnt;
        return i;
    }

    size_t ParseUlong(std::string &input, size_t &pos_in_file, size_t filesz) {
        std::string substr(input.data() + pos_in_file, filesz - pos_in_file);
        size_t cnt = 0;
        size_t ul = std::stoul(substr, &cnt);
        pos_in_file += cnt;
        return ul;
    }

    double ParseDouble(std::string &input, size_t &pos_in_file, size_t filesz) {
        std::string substr(input.data() + pos_in_file, filesz - pos_in_file);
        size_t cnt = 0;
        double d = std::stod(substr, &cnt);
        pos_in_file += cnt;
        return d;
    }

    std::string ParseString(std::string &input, size_t &pos_in_file, size_t filesz) {
        std::string substr(input.data() + pos_in_file, filesz - pos_in_file);
        size_t orig_pos = pos_in_file;
        for (; pos_in_file < filesz; ++pos_in_file) {
            if (input[pos_in_file] == ',' || input[pos_in_file] == '\n') {
                return substr.substr(0, pos_in_file - orig_pos);
            }
        }
        return substr.substr(0, pos_in_file - orig_pos);
    }

    void NextToken(std::string &input, size_t &pos_in_file, size_t filesz) {
        if (pos_in_file >= filesz) {
            return;
        }
        for (; pos_in_file < filesz; ++pos_in_file) {
            if (input[pos_in_file] == ',' || input[pos_in_file] == '\n') {
                ++pos_in_file;
                return;
            }
        }
        throw 1;
    }

    void NextLine(std::string &input, size_t &pos_in_file, size_t filesz) {
        if (pos_in_file >= filesz) {
            return;
        }
        for (; pos_in_file < filesz; ++pos_in_file) {
            if (input[pos_in_file] == '\n') {
                ++pos_in_file;
                return;
            }
        }
    }

    size_t LoadCSV(std::string &path, MPI_File &file, std::string &buf) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::LoadCSV");
        int ec;
        MPI_Status status;
        MPI_Offset filesz;
        ec = MPI_File_open(MPI_COMM_WORLD, path.data(), MPI_MODE_RDWR, MPI_INFO_NULL, &file);
        if(ec != MPI_SUCCESS || file == nullptr) {
            std::cout << "Could not open CSV" << std::endl;
            throw 1;
        }
        MPI_File_get_size(file, &filesz);
        if (filesz==0 && csv_header_ && rank_ == 0) {
            csv_header_ = false; //TODO: write schema to file instead;
        }
        if(filesz == 0) {
            return 0;
        }
        buf.reserve(filesz);
        MPI_File_read_at_all(file, 0, buf.data(), filesz, MPI_CHAR, &status); //TODO: Error handling?
        return filesz;
    }

    void SaveCSV(std::string &serial, MPI_File &file) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::SaveCSV");
        if(file == nullptr) {
            std::cout << "Saving to an invalid CSV" << std::endl;
            throw 1;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Status status;
        size_t off = 0;
        std::vector<size_t> offsets(nprocs_);
        size_t serial_size = serial.size();
        MPI_Allgather(&serial_size, 1, MPI_UNSIGNED_LONG, &offsets[0], 1, MPI_UNSIGNED_LONG, MPI_COMM_WORLD);
        for(int i = 0; i < rank_; ++i) {
            off += offsets[i];
        }
        MPI_File_seek(file, off, MPI_SEEK_END);
        int ec = MPI_File_write_all(file, &serial[0], serial.size(), MPI_CHAR, &status);
        if(ec != MPI_SUCCESS) {
            std::cout << "Could not write to MPI file" << std::endl;
            return;
        }
    }

    void CloseCSV(MPI_File &file) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::CloseCSV");
        if(file != nullptr) {
            MPI_File_close(&file);
        }
    }

    void ParseModelRow(std::string &input, size_t &pos_in_file, size_t filesz) {
        double nprocs_coeff = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double tot_read_coeff = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double tot_write_coeff = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        std::string conf = ParseString(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        std::string my_conf="rank"+std::to_string(BASKET_CONF->MPI_RANK);
        if(my_conf == conf && storage_models_.find(conf) == storage_models_.end()) {
            storage_configs_.emplace_back(conf);
            //printf("%s\n",conf.data());
        }
        storage_models_[conf].UpdateCoeffs(nprocs_coeff, tot_read_coeff, tot_write_coeff);
    }

    void LoadModelCSV() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::LoadModelCSV");
        size_t pos_in_file = 0;
        std::string buf;
        size_t filesz = LoadCSV(model_file_path_, model_file_, buf);
        if(csv_header_) {
            NextLine(buf, pos_in_file, filesz);
        }
        for (; pos_in_file < filesz;) {
            ParseModelRow(buf, pos_in_file, filesz);
        }
    }

    void SaveModelCSV() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::SaveModelCSV");
        std::stringstream ss;
        for(std::string &storage_config : storage_configs_) {
            LinRegModel &model = storage_models_[storage_config];
            LinRegModel::CoeffQueue &q = model.GetCoeffQueue();
            LinRegModel::CoeffArray coeffs;
            size_t window_off = model.GetWindowOff();
            while(q.pop(coeffs)) {
                if(window_off > 0) { --window_off; continue; }
                ss <<
                   coeffs[0] << "," <<
                   coeffs[1] << "," <<
                   coeffs[2] << "," <<
                   storage_config <<
                   std::endl;
            }
        }
        std::string serial = ss.str();
        SaveCSV(serial, model_file_);
    }

    void CommitMetrics() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::CommitMetrics");
        if(!commit_metrics_) { return; }
        SaveModelCSV();
    }

    void Fit() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::Fit");
        for(std::string &storage_config : storage_configs_) {
            LinRegModel &model = storage_models_[storage_config];
            model.Fit();
        }
    }

    void Run(std::future<void> loop_cond) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::AsyncFitCommit");
        do {
            if(window_tick_ >= window_size_) {
                Fit();
                CommitMetrics();
                window_tick_ = 0;
            }
        }
        while(loop_cond.wait_for(std::chrono::milliseconds(500))==std::future_status::timeout);
        if(window_tick_ >= window_size_) {
            Fit();
        }
        CommitMetrics();
    }

public:
    StorageCostPredictor() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::StorageCostPredictor");
    }
    ~StorageCostPredictor() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::~StorageCostPredictor");
    }

    void SetWindowSize(size_t window_size) { window_size_ = window_size; }
    void EnableMetricStorage(bool commit_metrics) { commit_metrics_ = commit_metrics; }

    void Init(int rank, int nprocs, std::string model_file_path="", bool csv_header = false) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::LoadMetrics");
        rank_ = rank;
        nprocs_ = nprocs;
        model_file_path_ = model_file_path;
        csv_header_ = csv_header_;
        LoadModelCSV();
        worker_thread_ = std::thread(&StorageCostPredictor::Run, this, std::move(terminate_worker_.get_future()));
    }

    void Finalize() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::~StorageCostPredictor");
        terminate_worker_.set_value();
        worker_thread_.join();
        CloseCSV(model_file_);
    }

    void Feedback(double bw_measured, double read_bytes, double write_bytes, std::string &config) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::Feedback");
        storage_models_[config].Feedback(bw_measured, nprocs_, write_bytes, read_bytes);
        ++window_tick_;
    }

    double Predict(double read_bytes, double write_bytes, std::string config) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::Predict");
        return storage_models_[config].Predict(nprocs_, read_bytes, write_bytes);
    }
};

#undef LINREG_NPARAMS
#endif //SYMBIOS_STORAGE_COST_PREDICTOR_H
