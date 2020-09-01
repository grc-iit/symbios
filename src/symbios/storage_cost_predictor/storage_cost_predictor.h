//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_STORAGE_COST_PREDICTOR_H
#define SYMBIOS_STORAGE_COST_PREDICTOR_H

#include <iostream>
#include <fstream>
#include <mpi.h>
#include <dlib/optimization.h>
#include <boost/filesystem/operations.hpp>
#include <array>
#include <vector>
#include <unordered_map>
#include <math.h>
#include <common/debug.h>

struct IOMetric {
    double avg_msec_;
    double std_msec_;
    double min_msec_;
    double max_msec_;
    int nprocs_;
    size_t block_size_;
    int ap_;
    size_t tot_read_;
    size_t tot_write_;
    size_t tot_bytes_;
    double bw_read_;
    double bw_write_;
    double bw_kbps_;
    std::string conf_;

    IOMetric(double avg_msec,  double std_msec, double min_msec, double max_msec,
             int nprocs, size_t block_size, int ap, size_t tot_read, size_t tot_write, size_t tot_bytes,
             double bw_read, double bw_write, double bw_kbps, std::string conf) :

            avg_msec_(avg_msec), std_msec_(std_msec), min_msec_(min_msec), max_msec_(max_msec),
            nprocs_(nprocs), block_size_(block_size), ap_(ap),
            tot_read_(tot_read), tot_write_(tot_write), tot_bytes_(tot_bytes),
            bw_read_(bw_read), bw_write_(bw_write), bw_kbps_(bw_kbps), conf_(conf) {}
};

struct ModelMetric {
    double tot_read_;
    double tot_write_;
    double bw_pred_;
    double bw_act_;
    double accuracy_;
    ModelMetric(double bw_pred, double bw_act, double tot_read, double tot_write) :
    bw_pred_(bw_pred), bw_act_(bw_act), tot_read_(tot_read), tot_write_(tot_write) {
        accuracy_ = 1 - std::abs(bw_act_ - bw_pred_)/bw_act_;
        accuracy_ = accuracy_ < 0 ? 0 : accuracy_;
    }
};

class LinRegModel {
public:
    typedef std::array<double, 2> CoeffArray;
private:
    typedef dlib::matrix<double, 2, 1> parameter_vector;
    typedef dlib::matrix<double, 2, 1> input_vector;
    typedef std::pair<input_vector, double> Observation;
    typedef std::vector<Observation> ObservationList;

    ObservationList dataset_;
    CoeffArray coeffs_;

    static double Residual(const Observation &data, const parameter_vector &params) {
        const input_vector &x = data.first;
        double y = data.second;
        double sum = 0;
        int i = 0;
        for(i = 0; i < 2; ++i) { sum += params(i) * x(i); }
        return std::pow(y - sum, 2);
    }

public:
    LinRegModel() {}

    void Fit(void) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("LinRegModel::FitData");
        if (dataset_.size() > 0) {
            parameter_vector params = dlib::randm(2,1);
            dlib::solve_least_squares_lm(
                    dlib::objective_delta_stop_strategy(1e-7),
                    Residual,
                    dlib::derivative(Residual),
                    dataset_,
                    params);
            for (int i = 0; i < 2; ++i) {
                coeffs_[i] = params(i);
            }
        }
    }

    void Feedback(double bw, double read_bytes, double write_bytes) {
        common::debug::AutoTrace trace = common::debug::AutoTrace("LinRegModel::Feedback");
        input_vector iv;
        iv(0) = read_bytes;
        iv(1) = write_bytes;
        dataset_.emplace_back(iv, bw);
    }

    double Predict(double read_bytes, double write_bytes) {
        return coeffs_[0]*read_bytes + coeffs_[1]*write_bytes;
    }
};

class StorageCostPredictor {
private:
    std::string metrics_file_path_, model_file_path_;
    MPI_File metrics_file_ = nullptr, model_file_ = nullptr;
    int rank_ = 0, nprocs_ = 1;
    bool commit_metrics_ = true, csv_header_ = true;
    size_t window_size_ = 256;
    size_t window_off_ = 0;
    std::unordered_map<std::string, LinRegModel> storage_models_;
    std::list<std::string> storage_configs_;
    std::list<ModelMetric> model_metrics_;
    std::vector<IOMetric> metrics_;

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

    size_t LoadCSV(std::string &path, MPI_File &file, size_t &nrows, std::string &buf) {
        MPI_Status status;
        size_t filesz = 0;
        bool exists = boost::filesystem::exists(path);
        if (exists) { filesz = boost::filesystem::file_size(path); }
        MPI_File_open(MPI_COMM_WORLD, path.data(), MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &file); //TODO: Error handling?
        if (!exists && csv_header_ && rank_ == 0) {
            csv_header_ = false; //TODO: write schema to file instead;
            return 0;
        }
        buf.reserve(filesz);
        MPI_File_read_at_all(file, 0, buf.data(), filesz, MPI_CHAR, &status); //TODO: Error handling?
        return filesz;
    }

    void SaveCSV(std::string &serial, MPI_File &file) {
        if(file == nullptr) {
            std::cout << "Saving to an invalid CSV" << std::endl;
            throw 1;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Status status;
        size_t off = 0;
        std::vector<int> offsets(nprocs_);
        size_t serial_size = serial.size();
        MPI_Alltoall(&serial_size, 1, MPI_UNSIGNED_LONG, &offsets[0], 1, MPI_UNSIGNED_LONG, MPI_COMM_WORLD);
        for(int i = 0; i < rank_; ++i) {
            off += offsets[i];
        }
        MPI_File_seek(file, off, MPI_SEEK_END);
        MPI_File_write_all(file, serial.data(), serial.size(), MPI_CHAR, &status);
    }

    void CloseCSV(MPI_File &file) {
        if(file != nullptr) {
            MPI_File_close(&file);
        }
    }

    void ParseMetricsRow(std::string &input, size_t &pos_in_file, size_t filesz) {
        double avg_msec = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double std_msec = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double min_msec = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double max_msec = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        int nprocs = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        size_t block_size = ParseUlong(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        int ap = ParseInt(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        size_t tot_read = ParseUlong(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        size_t tot_write = ParseUlong(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        size_t tot_bytes = ParseUlong(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double bw_read = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double bw_write = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        double bw_kbps = ParseDouble(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        std::string conf = ParseString(input, pos_in_file, filesz); NextToken(input, pos_in_file, filesz);
        if(storage_models_.find(conf) == storage_models_.end()) {
            storage_configs_.emplace_back(conf);
        }
        storage_models_[conf].Feedback(bw_kbps, tot_read, tot_write);
        metrics_.emplace_back(
                avg_msec, std_msec, min_msec, max_msec, nprocs, block_size, ap,
                tot_read, tot_write, tot_bytes, bw_read, bw_write, bw_kbps, conf);
    }

    void LoadMetricsCSV() {
        size_t nrows = 0, pos_in_file = 0;
        std::string buf;
        size_t filesz = LoadCSV(metrics_file_path_, metrics_file_, nrows, buf);
        if(csv_header_) {
            NextLine(buf, pos_in_file, filesz);
        }
        for (; pos_in_file < filesz;) {
            ParseMetricsRow(buf, pos_in_file, filesz);
        }
    }

    void SaveMetricsCSV() {
        std::stringstream ss;
        for(int row_id = window_off_; row_id < metrics_.size(); ++row_id) {
            IOMetric &io_metric = metrics_[row_id];
            ss <<
            io_metric.avg_msec_ << "," <<
            io_metric.std_msec_ << "," <<
            io_metric.min_msec_ << "," <<
            io_metric.max_msec_ << "," <<
            io_metric.nprocs_ << "," <<
            io_metric.block_size_ << "," <<
            io_metric.ap_ << "," <<
            io_metric.tot_read_ << "," <<
            io_metric.tot_write_ << "," <<
            io_metric.tot_bytes_ << "," <<
            io_metric.bw_read_ << "," <<
            io_metric.bw_write_ << "," <<
            io_metric.bw_kbps_ << "," <<
            io_metric.conf_ <<
            std::endl;
        }
        std::string serial = ss.str();
        SaveCSV(serial, metrics_file_);
    }

    void SaveModelCSV() {
        std::stringstream ss;
        for(int row_id = window_off_; row_id < metrics_.size(); ++row_id) {
            IOMetric &io_metric = metrics_[row_id];
            ss <<
               io_metric.avg_msec_ << "," <<
               io_metric.std_msec_ << "," <<
               io_metric.min_msec_ << "," <<
               io_metric.max_msec_ << "," <<
               io_metric.nprocs_ << "," <<
               io_metric.block_size_ << "," <<
               io_metric.ap_ << "," <<
               io_metric.tot_read_ << "," <<
               io_metric.tot_write_ << "," <<
               io_metric.tot_bytes_ << "," <<
               io_metric.bw_read_ << "," <<
               io_metric.bw_write_ << "," <<
               io_metric.bw_kbps_ << "," <<
               io_metric.conf_ <<
               std::endl;
        }
        std::string serial = ss.str();
        SaveCSV(serial, metrics_file_);
    }

public:
    StorageCostPredictor() {}
    ~StorageCostPredictor() {
        CommitMetrics();
        CloseCSV(metrics_file_);
        CloseCSV(model_file_);
    }

    void Init(bool commit_metrics=true, bool csv_header=true, size_t window_size=256) {
        commit_metrics_ = commit_metrics;
        csv_header_ = csv_header_;
        window_size_ = window_size;
    }

    void LoadMetrics(int rank, int nprocs, std::string metrics_file_path, std::string model_file_path) {
        rank_ = rank;
        nprocs_ = nprocs;
        metrics_file_path_ = metrics_file_path;
        model_file_path_ = model_file_path;
        LoadMetricsCSV();
        Fit();
        window_off_ = metrics_.size();
    }

    void CommitMetrics() {
        if(!commit_metrics_) { return; }
        SaveMetricsCSV();
        SaveModelCSV();
        window_off_ = metrics_.size();
    }

    void Fit() {
        for(std::string &storage_config : storage_configs_) {
            LinRegModel &model = storage_models_[storage_config];
            model.Fit();
        }
    }

    void Feedback(double bw_measured, double read_bytes, double write_bytes, std::string &config) {
        metrics_.emplace_back(
                0, 0, 0, 0, nprocs_, 0, 0,
                read_bytes, write_bytes, read_bytes+write_bytes, 0, 0, bw_measured, config);
        storage_models_[config].Feedback(bw_measured, write_bytes, read_bytes);
        if(window_off_ % window_size_ == 0 && window_off_ > 0) {
            Fit();
            CommitMetrics();
        }
    }

    void Feedback(double bw_measured, double bw_pred, double read_bytes, double write_bytes, std::string &config) {
        Feedback(bw_measured, read_bytes, write_bytes, config);
        model_metrics_.emplace_back(bw_measured, bw_pred, read_bytes, write_bytes);
    }

    double Predict(double read_bytes, double write_bytes, std::string &config) {
        double bw_max_pred = 0;
        for(std::string &storage_config : storage_configs_) {
            LinRegModel &model = storage_models_[storage_config];
            double bw_pred = model.Predict(read_bytes, write_bytes);
            if(bw_pred > bw_max_pred) {
                bw_max_pred = bw_pred;
                config = storage_config;
            }
            return bw_pred;
        }
    }
};

#endif //SYMBIOS_STORAGE_COST_PREDICTOR_H
