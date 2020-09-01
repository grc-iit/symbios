//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_STORAGE_COST_PREDICTOR_H
#define SYMBIOS_STORAGE_COST_PREDICTOR_H

#include <dlib/optimization.h>
#include <array>
#include <vector>
#include <math.h>
#include <boost/filesystem/operations.hpp>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <common/debug.h>

template<size_t nparams>
class StorageCostPredictor {
private:
    typedef dlib::matrix<double, nparams, 1> input_vector, parameter_vector;
    typedef std::pair<input_vector, double> LsqArg;
    typedef std::list<LsqArg> LsqArgVec;
    typedef std::array<double, nparams> CoeffArray, ObservationArray;

private:
    LsqArgVec dataset_;
    CoeffArray coeffs_;

    //Least Squares
    static double Residual(const LsqArg &data, const parameter_vector &params) {
        const input_vector &x = data.first;
        double y = data.second;
        double sum = 0;
        for(size_t i = 0; i < params.size(); ++i) {
            sum += params(i) * x[i];
        }
        return std::pow(y - sum, 2);
    }

public:
    StorageCostPredictor() {}

    void LoadMetrics() {
    }

    void CommitMetrics() {
    }

    void Fit(void) {
        AutoTrace trace = AutoTrace("StorageCostPredictor::FitData");
        if (observations.size() > 0) {
            dlib::parameter_vector params(nparams);
            dlib::solve_least_squares_lm(
                    dlib::objective_delta_stop_strategy(1e-7),
                    Residual,
                    dlib::derivative(Residual),
                    dataset_,
                    params);
            for (int i = 0; i < nparams; ++i) {
                coeffs_[i] = params(0);
            }
        }
    }

    template<typename ...Args>
    void Feedback(double y, Args ...args) {
        AutoTrace trace = AutoTrace("StorageCostPredictor::Feedback");
        input_vector observation = {args...};
    }
};


#endif //SYMBIOS_STORAGE_COST_PREDICTOR_H
