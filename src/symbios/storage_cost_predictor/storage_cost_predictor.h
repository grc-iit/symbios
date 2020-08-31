//
// Created by mani on 8/24/2020.
//

#ifndef SYMBIOS_STORAGE_COST_PREDICTOR_H
#define SYMBIOS_STORAGE_COST_PREDICTOR_H

#include <dlib/optimization.h>
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

template<size_t size>
class StorageCostPredictor {
private:
    typedef dlib::matrix<double, 1, 1> input_vector;
    typedef dlib::matrix<double, 1, 1> parameter_vector;
    typedef std::pair<input_vector, parameter_vector> LSQ_ARG;
    typedef std::vector<LSQ_ARG> LSQ_ARG_VEC;

    typedef struct JSONCompressionMetrics {
        int type;
        int alg;
        uint64_t S;
        double r;
        double tc;
    } JsonCompressionMetrics;

    typedef struct JSONDecompressionMetrics {
        int type;
        int alg;
        uint64_t S;
        double td;
    } JSONDecompressionMetrics;

private:
    double placement_meta[NUM_ARES_DATA_TYPES][NUM_ARES_LIBRARIES][3];
    LSQ_ARG_VEC tc_dataset[NUM_ARES_DATA_TYPES][NUM_ARES_LIBRARIES];
    LSQ_ARG_VEC r_dataset[NUM_ARES_DATA_TYPES][NUM_ARES_LIBRARIES];
    LSQ_ARG_VEC td_dataset[NUM_ARES_DATA_TYPES][NUM_ARES_LIBRARIES];
    std::unordered_map<unsigned,int64_t> compressed_sizes;
    std::vector<JSONCompressionMetrics> comp_metrics;
    std::vector<JSONDecompressionMetrics> decomp_metrics;
    size_t comp_metrics_commit;
    size_t decomp_metrics_commit;

    static double TCDResidual(const std::pair<input_vector, double> &data, const parameter_vector &params) {
        double c1 = params(0);
        double S = data.first(0);
        float tc = data.second;
        return pow(c1 * S - tc, 2);
    }

    static double RatioResidual(const std::pair<input_vector, double> &data, const parameter_vector &params) {
        double c1 = params(0);
        float r = data.second;
        return pow(c1 - r, 2);
    }

public:
    StorageCostPredictor() {}

    void AddCompressionMetrics(int64_t type, int64_t alg, uint64_t S, double r, double tc) {
        //std::cout << "CompressionMetricsManager::AddCompressionMetrics-{" << tc << "}-{" << r << "}-{" << S << "}-{" << alg << "}\n";
        //Comp metrics
        JSONCompressionMetrics metrics;
        metrics.type = type;
        metrics.alg = alg;
        metrics.S = S;
        metrics.r = r;
        metrics.tc = tc;
        comp_metrics.push_back(metrics);

        //tc problem
        LSQ_ARG tc_arg;
        tc_arg.first(0) = S;
        tc_arg.second = tc;
        tc_dataset[type][alg].push_back(tc_arg);

        //r problem
        LSQ_ARG r_arg;
        r_arg.first(0) = S;
        r_arg.second = r;
        r_dataset[type][alg].push_back(r_arg);
    }

    void AddCompressionMetrics(int64_t type, CompressionLibrary alg, uint64_t S, double r, double tc) {
        AddCompressionMetrics(type, static_cast<int>(alg), S, r, tc);
    }

    void AddDecompressionMetrics(int64_t type, int64_t alg, uint64_t S, double td) {
        //std::cout << "CompressionMetricsManager::AddDecompressionMetrics-{" << td << "}-{" << S << "}-{" << alg << "}\n";
        //Decompression metrics
        JSONDecompressionMetrics metrics;
        metrics.type = type;
        metrics.alg = alg;
        metrics.S = S;
        metrics.td = td;
        decomp_metrics.push_back(metrics);

        //td problem
        LSQ_ARG td_arg;
        td_arg.first(0) = S;
        td_arg.second = td;
        td_dataset[type][alg].push_back(td_arg);
    }

    void AddDecompressionMetrics(int64_t type, CompressionLibrary alg, uint64_t S, double td) {
        AddDecompressionMetrics(type, static_cast<int>(alg), S, td);
    }

    void LoadMetrics(std::string metric_file) {
        AutoTrace trace = AutoTrace("CompressionMetricsManager::LoadMetrics");

        //Open input file
        FILE *outfile = fopen(metric_file.c_str(), "r");
        if (outfile == NULL) {
            std::cout << "MetadataManager - Failed to open " << metric_file.c_str() << std::endl;
            return;
        }

        //Initialize read stream
        size_t filesz = boost::filesystem::file_size(metric_file.c_str());
        char buf[65536];
        rapidjson::FileReadStream instream(outfile, buf, sizeof(buf));

        //Create datasets for the different optimization problems.
        while (instream.Tell() < filesz) {
            //Get JSON object
            rapidjson::Document d;
            d.ParseStream<rapidjson::kParseStopWhenDoneFlag>(instream);
            if (d.HasParseError()) {
                break;
            }

            //Read object
            if (d.HasMember("c_time")) {
                int type = (int) d["type"].GetInt();
                int alg = (int) d["algo"].GetInt();
                uint64_t S = (uint64_t) d["uc_size"].GetUint64();
                double tc = (double) d["c_time"].GetDouble();
                double r = (double) d["c_ratio"].GetDouble();

                //Add compression metric
                AddCompressionMetrics(type, alg, S, r, tc);
            } else if (d.HasMember("d_time")) {
                int type = (int) d["type"].GetInt();
                int alg = (int) d["algo"].GetInt();
                uint64_t S = (uint64_t) d["uc_size"].GetUint64();
                double td = (double) d["d_time"].GetDouble();

                //Add decompression metric
                AddDecompressionMetrics(type, alg, S, td);
            }
        }
        //Initialize the no compression parameters
        for(int data_type = 0; data_type < NUM_ARES_DATA_TYPES; ++data_type) {
            placement_meta[data_type][0][0] = 0;
            placement_meta[data_type][0][1] = 1;
            placement_meta[data_type][0][2] = 0;
        }
        fclose(outfile);
    }

    void CommitMetrics(std::string metric_file) {
        AutoTrace trace = AutoTrace("CompressionMetricsManager::CommitMetrics");

        int i;
        FILE *metrics_file = fopen(metric_file.c_str(), "a");
        if (metrics_file == NULL)
            return;
        char buf[65536];

        //Commit compression metrics
        for (i = comp_metrics_commit; i < comp_metrics.size(); ++i) {
            rapidjson::Document d;
            rapidjson::FileWriteStream outstream(metrics_file, buf, sizeof(buf));
            rapidjson::Writer <rapidjson::FileWriteStream> writer(outstream);
            d.SetObject();
            d.AddMember("type", comp_metrics[i].type, d.GetAllocator());
            d.AddMember("algo", comp_metrics[i].alg, d.GetAllocator());
            d.AddMember("uc_size", comp_metrics[i].S, d.GetAllocator());
            d.AddMember("c_ratio", comp_metrics[i].r, d.GetAllocator());
            d.AddMember("c_time", comp_metrics[i].tc, d.GetAllocator());
            d.Accept(writer);
            fprintf(metrics_file, "\n");
        }
        comp_metrics_commit = i;

        //Commit decompression metrics
        for (i = decomp_metrics_commit; i < decomp_metrics.size(); ++i) {
            rapidjson::Document d;
            rapidjson::FileWriteStream outstream(metrics_file, buf, sizeof(buf));
            rapidjson::Writer <rapidjson::FileWriteStream> writer(outstream);
            d.SetObject();
            d.AddMember("type", decomp_metrics[i].type, d.GetAllocator());
            d.AddMember("algo", decomp_metrics[i].alg, d.GetAllocator());
            d.AddMember("uc_size", decomp_metrics[i].S, d.GetAllocator());
            d.AddMember("d_time", decomp_metrics[i].td, d.GetAllocator());
            d.Accept(writer);
            fprintf(metrics_file, "\n");
        }
        decomp_metrics_commit = i;

        fclose(metrics_file);
    }

    void FitData(void) {
        AutoTrace trace = AutoTrace("CompressionMetricsManager::FitData");

        using namespace dlib;
        for (int type = 1; type < NUM_ARES_DATA_TYPES; type++) {
            for (int alg = 1; alg < NUM_ARES_LIBRARIES; alg++) {
                //tc ~ c1*S
                if (tc_dataset[type][alg].size() > 0) {
                    parameter_vector c1(1);
                    solve_least_squares_lm(objective_delta_stop_strategy(1e-7),
                                           TCDResidual,
                                           derivative(TCDResidual),
                                           tc_dataset[type][alg],
                                           c1);
                    placement_meta[type][alg][0] = c1(0);
                }
                //r ~ c2
                if (r_dataset[type][alg].size() > 0) {
                    parameter_vector c2(1);
                    solve_least_squares_lm(objective_delta_stop_strategy(1e-7),
                                           RatioResidual,
                                           derivative(RatioResidual),
                                           r_dataset[type][alg],
                                           c2);
                    placement_meta[type][alg][1] = c2(0);
                }
                //td ~ c3*S
                if (td_dataset[type][alg].size() > 0) {
                    parameter_vector c3(1);
                    solve_least_squares_lm(objective_delta_stop_strategy(1e-7),
                                           TCDResidual,
                                           derivative(TCDResidual),
                                           td_dataset[type][alg],
                                           c3);
                    placement_meta[type][alg][2] = c3(0);
                }
            }
        }
    }

    void GetCompressionLibraryFit(int data_type, int comp_lib, float &c1, float &c2, float &c3) {
        c1 = placement_meta[data_type][comp_lib][0];
        c2 = placement_meta[data_type][comp_lib][1];
        c3 = placement_meta[data_type][comp_lib][2];
    }

    int GetDatasetType(hid_t type) {
        if (H5Tequal(type, H5T_NATIVE_CHAR))
            return 1;
        if (H5Tequal(type, H5T_NATIVE_INT32))
            return 2;
        if (H5Tequal(type, H5T_NATIVE_UINT32))
            return 4;
        if (H5Tequal(type, H5T_NATIVE_FLOAT))
            return 6;
        if (H5Tequal(type, H5T_NATIVE_DOUBLE))
            return 7;
        return 0;
    }

    void PrintCompressionMetrics() {
        float net_tc=0, net_r=0, net_td=0;

        //Get total tc and avg r
        for(unsigned i = comp_metrics_commit; i < comp_metrics.size(); ++i) {
            net_tc += comp_metrics[i].tc;
            net_r += comp_metrics[i].r;
        }
        net_r /= comp_metrics.size() - comp_metrics_commit + 1;

        //Get total td
        for(unsigned i = decomp_metrics_commit; i < decomp_metrics.size(); ++i) {
            net_td += decomp_metrics[i].td;
        }

        printf("CompressionMetricsManager::PrintCompressionMetrics-{tc=%f}-{r=%f}-{td=%f}\n", net_tc, net_r, net_td);
    }

    void UpdateSize(unsigned key,int64_t size){
        auto iter=compressed_sizes.find(key);
        if(iter!=compressed_sizes.end()){
            iter->second+=size;
        }else compressed_sizes.insert(std::pair<unsigned ,int64_t >(key,size));
    }
    int64_t GetSize(unsigned key){
        auto iter=compressed_sizes.find(key);
        if(iter!=compressed_sizes.end()){
            return iter->second;
        }else 0;
    }*/
};


#endif //SYMBIOS_STORAGE_COST_PREDICTOR_H
