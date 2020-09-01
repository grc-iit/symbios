//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_STORAGE_COST_PREDICTOR_H
#define SYMBIOS_STORAGE_COST_PREDICTOR_H

#include <dlib/optimization.h>
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
    typedef dlib::matrix<double, 1, 1> input_vector, parameter_vector;
    typedef std::pair<input_vector, parameter_vector> LSQ_ARG;
    typedef std::vector<LSQ_ARG> LSQ_ARG_VEC;
    typedef std::list<std::array<double, nparams + 1>> ObservationList;
    typedef std::unique_ptr<std::list<std::array<double, nparams>>> CoeffArrayPtr;

private:
    LSQ_ARG_VEC dataset;

    //Least Squares
    static double Residual(const std::pair<input_vector, double> &data, const parameter_vector &params) {
        double c1 = params(0);
        input_vector &x = data.first;
        float y = data.second;
        for(size_t i = 0; i < params.size(); ++i) {

        }
        return std::pow(c1 * S - tc, 2);
    }

public:
    StorageCostPredictor() {}

    //y = c1*x1 + c2*x2 + ...
    CoeffArrayPtr Fit(ObservationList &observations) {
        CoeffArrayPtr coeffs;
        AutoTrace trace = AutoTrace("StorageCostPredictor::FitData");
        if (observations.size() > 0) {
            dlib::parameter_vector params(nparams);
            dlib::solve_least_squares_lm(
                    dlib::objective_delta_stop_strategy(1e-7),
                    Residual,
                    dlib::derivative(Residual),
                    dataset,
                    params);
            for (int i = 0; i < nparams; ++i) {
                coeffs->set(params(0));
            }
        }
        return std::move(coeffs);
    }

    void Feedback(void) {
        AutoTrace trace = AutoTrace("StorageCostPredictor::Feedback");

    }
};


#endif //SYMBIOS_STORAGE_COST_PREDICTOR_H
