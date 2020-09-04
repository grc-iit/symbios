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
#include "serializers.h"

#define LINREG_NPARAMS 3
#define MAX_METRIC_QUEUE 1024
#define RECORD_SIZE (3*sizeof(float) + sizeof(int))
#define MB (1<<20)

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

    static double Residual(const Observation &data, const parameter_vector &params) {
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
            dlib::solve_least_squares_lm(
                    dlib::objective_delta_stop_strategy(1e-7),
                    Residual,
                    dlib::derivative(Residual),
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
    FILE *model_file_ = nullptr;
    int rank_ = 0, nprocs_ = 1;
    bool commit_metrics_ = true;
    size_t window_size_ = 256, window_tick_ = 0;
    std::unordered_map<int, LinRegModel> storage_models_;
    std::list<int> storage_configs_;
    std::thread worker_thread_;
    std::promise<void> terminate_worker_;
    size_t buffer_size_ = 2*MB;

    void SaveCSV(char *serial, size_t serial_size, FILE *file) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::SaveCSV");
        if(file == nullptr) {
            std::cout << "Saving to an invalid CSV" << std::endl;
            throw 1;
        }
        fwrite(serial, 1, serial_size, model_file_);
    }

    void CloseCSV(FILE *file) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::CloseCSV");
        if(file != nullptr) {
            fclose(file);
        }
    }

    void ParseModelRow(BinaryDeserializer &deserializer) {
        int conf;
        float nprocs_coeff, tot_read_coeff, tot_write_coeff;
        deserializer.ParseFloat(nprocs_coeff);
        deserializer.ParseFloat(tot_read_coeff);
        deserializer.ParseFloat(tot_write_coeff);
        deserializer.ParseInt(conf);
        if(storage_models_.find(conf) == storage_models_.end()) {
            storage_configs_.emplace_back(conf);
        }
        storage_models_[conf].UpdateCoeffs(nprocs_coeff, tot_read_coeff, tot_write_coeff);
    }

    bool LoadModelCSV_part(std::string path, bool required) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::LoadModelCSV_part");
        BinaryDeserializer deserializer(buffer_size_);
        model_file_ = std::fopen(path.c_str(), "r+");
        if(model_file_ == nullptr) {
            if(required) {
                std::cout << "Could not open " << model_file_path_ << std::endl;
                throw 1;
            }
            return false;
        }
        while(int bytes = std::fread(deserializer.GetBuf(), 1, RECORD_SIZE, model_file_)) {
            int nrows = bytes/RECORD_SIZE;
            for(int i = 0; i < nrows; ++i) {
                ParseModelRow(deserializer);
            }
        }
        return true;
    }

    void LoadModelCSV() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::LoadModelCSV");
        BinaryDeserializer deserializer(buffer_size_);
        model_file_ = std::fopen((model_file_path_ + std::to_string(rank_)).c_str(), "r+");
        if(model_file_ == nullptr) {
            model_file_ = std::fopen((model_file_path_ + std::to_string(rank_)).c_str(), "w");
        }
        for(int i = 0; i < nprocs_; ++i) {
            LoadModelCSV_part(model_file_path_ + std::to_string(i), false);
        }
        LoadModelCSV_part(model_file_path_, true);
    }

    void SaveModelCSV() {
        BinarySerializer serializer(buffer_size_);
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::SaveModelCSV");
        for(const int &storage_config : storage_configs_) {
            LinRegModel &model = storage_models_[storage_config];
            LinRegModel::CoeffQueue &q = model.GetCoeffQueue();
            LinRegModel::CoeffArray coeffs;
            while(q.pop(coeffs)) {
                serializer.WriteFloat(coeffs[0]);
                serializer.WriteFloat(coeffs[1]);
                serializer.WriteFloat(coeffs[2]);
                serializer.WriteInt(storage_config);
            }
        }
        SaveCSV(serializer.GetBuf(), serializer.GetSize(), model_file_);
    }

    void CommitMetrics() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::CommitMetrics");
        if(!commit_metrics_) { return; }
        SaveModelCSV();
    }

    void Fit() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::Fit");
        for(int &storage_config : storage_configs_) {
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

    void Init(int rank, int nprocs, std::string model_file_path) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::LoadMetrics");
        rank_ = rank;
        nprocs_ = nprocs;
        model_file_path_ = model_file_path;
        LoadModelCSV();
        worker_thread_ = std::thread(&StorageCostPredictor::Run, this, terminate_worker_.get_future());
    }

    void Finalize() {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::~StorageCostPredictor");
        terminate_worker_.set_value();
        worker_thread_.join();
        CloseCSV(model_file_);
    }

    void Feedback(double bw_measured, double read_bytes, double write_bytes, int storage_index) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::Feedback");
        storage_models_[storage_index].Feedback(bw_measured, nprocs_, write_bytes, read_bytes);
        ++window_tick_;
    }

    double Predict(double read_bytes, double write_bytes, int storage_index) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("StorageCostPredictor::Predict");
        return storage_models_[storage_index].Predict(nprocs_, read_bytes, write_bytes);
    }
};

#undef LINREG_NPARAMS
#endif //SYMBIOS_STORAGE_COST_PREDICTOR_H
