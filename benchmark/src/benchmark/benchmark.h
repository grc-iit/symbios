//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_BENCHMARK_H
#define SYMBIOS_BENCHMARK_H

#include <iostream>
#include <common/arguments.h>
#include <common/rng.h>
#include "io_client.h"
#include "io_client_factory.h"
#include "file.h"

enum class WorkloadType {
    kPrealloc, kIoOnlyFs, kMdFs
};

class BenchmarkArgs : public common::args::ArgMap {
private:
    void VerifyArgs(void) {
        AssertOptIsSet("-s");
        int storage_service = GetIntOpt("-s");
        switch(static_cast<IOClientType>(storage_service)) {
            case IOClientType::kOrangefs: {
                AssertOptIsSet("-caddr");
                break;
            }
            case IOClientType::kRedis:
            case IOClientType::kMongo: {
                AssertOptIsSet("-caddr");
                AssertOptIsSet("-cport");
            }
        }

        AssertOptIsSet("-w");
        int workload = GetIntOpt("-w");
        std::cout << "-w " << workload <<  std::endl;
        switch(static_cast<WorkloadType>(workload)) {
            case WorkloadType::kPrealloc: {
                AssertOptIsSet("-path");
                AssertOptIsSet("-fs");
                break;
            }
            case WorkloadType::kIoOnlyFs: {
                AssertOptIsSet("-path");
                AssertOptIsSet("-rfrac");
                AssertOptIsSet("-wfrac");
                AssertOptIsSet("-bs");
                AssertOptIsSet("-fs");
                AssertOptIsSet("-tot");
                break;
            }
            case WorkloadType::kMdFs: {
                AssertOptIsSet("-md_depth");
                AssertOptIsSet("-md_fcnt");
                AssertOptIsSet("-path");
                break;
            }
        }
    }

public:
    void Usage(void) {
        std::cout << "Usage: ./benchmark -[param-id] [value] ..." << std::endl;
        std::cout << "" << std::endl;

        std::cout << "-w [string]: Which workload to run" << std::endl;
        std::cout << "   prealloc" << std::endl;
        std::cout << "   io-only" << std::endl;
        std::cout << "   md-fs" << std::endl;

        std::cout << "-s [string]: Which storage service to use for the workload" << std::endl;
        std::cout << "   orangefs" << std::endl;
        std::cout << "   mongodb" << std::endl;
        std::cout << "   redis" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "OrangeFS Parameters" << std::endl;
        std::cout << "-caddr [string]: The local directory where the OrangeFS client is mounted" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "Redis/MongoDB Parameters" << std::endl;
        std::cout << "-caddr [string]: The IP address of the MongoDB server." << std::endl;
        std::cout << "-cport [string]: The port the MongoDB server runs on." << std::endl;
        std::cout << "" << std::endl;

        std::cout << "io-only workload parameters" << std::endl;
        std::cout << "-path [string]: The path to the file. Default: /symbios-test-file.bin." << std::endl;
        std::cout << "-rfrac [float]: The percent of I/O to dedicate to reads" << std::endl;
        std::cout << "-wfrac [float]: The percent of I/O to dedicate to writes" << std::endl;
        std::cout << "-bs [size]: The unit of I/O ops" << std::endl;
        std::cout << "-fs [size]: The size of the file to perform I/O with" << std::endl;
        std::cout << "-tot [size]: The total amount of I/O to perform" << std::endl;
        std::cout << "-ap [string]: The access pattern for I/O ops" << std::endl;
        std::cout << "   seq" << std::endl;
        std::cout << "   uniform" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "md-fs workload parameters" << std::endl;
        std::cout << "-md_depth [int]: The number of directories to nest" << std::endl;
        std::cout << "-md_fcnt [size]: The number of files to open/close in the deepest directory" << std::endl;
        std::cout << "-path [string]: The directory to begin creating files/directories" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "Uniform Distribution Parameters" << std::endl;
        std::cout << "-seed [int]" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "Additional Params" << std::endl;
        std::cout << "-direct: A flag that indicates whether or not to use direct I/O" << std::endl;
        std::cout << "-out: The file to output statistics from tests to" << std::endl;
    }

    BenchmarkArgs(int argc, char **argv) {
        AddOpt("-w", common::args::ArgType::kStringMap);
        AddStringMapVal("-w", "prealloc", static_cast<int>(WorkloadType::kPrealloc));
        AddStringMapVal("-w", "io-only", static_cast<int>(WorkloadType::kIoOnlyFs));
        AddStringMapVal("-w", "md-fs", static_cast<int>(WorkloadType::kMdFs));
        AddOpt("-s", common::args::ArgType::kStringMap);
        AddStringMapVal("-s", "orangefs", static_cast<int>(IOClientType::kOrangefs));
        AddStringMapVal("-s", "mongodb", static_cast<int>(IOClientType::kMongo));
        AddStringMapVal("-s", "redis", static_cast<int>(IOClientType::kRedis));
        AddOpt("-caddr", common::args::ArgType::kString);
        AddOpt("-cport", common::args::ArgType::kInt, 0);
        AddOpt("-p", common::args::ArgType::kSize);
        AddOpt("-path", common::args::ArgType::kString);
        AddOpt("-rfrac", common::args::ArgType::kFloat);
        AddOpt("-wfrac", common::args::ArgType::kFloat);
        AddOpt("-bs", common::args::ArgType::kSize);
        AddOpt("-fs", common::args::ArgType::kSize);
        AddOpt("-tot", common::args::ArgType::kSize);
        AddOpt("-ap", common::args::ArgType::kStringMap);
        AddStringMapVal("-ap", "seq", static_cast<int>(DistributionType::kNone));
        AddStringMapVal("-ap", "uniform", static_cast<int>(DistributionType::kUniform));
        AddOpt("-md_depth", common::args::ArgType::kInt);
        AddOpt("-md_fcnt", common::args::ArgType::kSize);
        AddOpt("-direct", common::args::ArgType::kNone);
        AddOpt("-out", common::args::ArgType::kString);
        AddOpt("-seed", common::args::ArgType::kInt);
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

#endif //SYMBIOS_BENCHMARK_H
