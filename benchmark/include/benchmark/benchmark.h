//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_BENCHMARK_H
#define SYMBIOS_BENCHMARK_H

#include <iostream>
#include <benchmark/arguments.h>
#include <benchmark/file.h>
#include <benchmark/rng.h>
#include <benchmark/io_client.h>
#include <benchmark/io_client_factory.h>

enum class WorkloadType {
    kIoOnlyFs, kIoOnlyKvs, kMdFs, kMdFile, kMdKvs
};

class BenchmarkArgs : public ArgMap {
private:
    void VerifyArgs(void) {
        if(!OptIsSet("-s")) {
            std::cout << "Must set storage service" << std::endl;
            Usage();
            exit(1);
        }
    }

public:
    void Usage(void) {
        std::cout << "Usage: ./benchmark -[param-id] [value] ..." << std::endl;
        std::cout << "" << std::endl;

        std::cout << "-w [string]: Which workload to run" << std::endl;
        std::cout << "   io-only" << std::endl;
        std::cout << "   md-fs" << std::endl;
        std::cout << "   md-kvs" << std::endl;

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
        std::cout << "-p [size]: Preallocate a file of this size" << std::endl;
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
        std::cout << "-md_fcnt [int]: The number of files to create in the deepest directory" << std::endl;
        std::cout << "-md_iter [int]: The number of seeks to perform on files" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "md-kvs workload parameters" << std::endl;
        std::cout << "-md_iter [int]: The number of times to add/check/delete key" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "Uniform Distribution Parameters" << std::endl;
        std::cout << "-seed [int]" << std::endl;
        std::cout << "" << std::endl;
    }

    BenchmarkArgs(int argc, char **argv) {
        AddOpt("-w", ArgType::kStringMap);
        AddStringMapVal("-w", "io-only", static_cast<int>(WorkloadType::kIoOnlyFs));
        AddStringMapVal("-w", "md-fs", static_cast<int>(WorkloadType::kMdFs));
        AddStringMapVal("-w", "md-kvs", static_cast<int>(WorkloadType::kMdKvs));

        AddOpt("-s", ArgType::kStringMap);
        AddStringMapVal("-s", "orangefs", static_cast<int>(IOClientType::kOrangefs));
        AddStringMapVal("-s", "mongodb", static_cast<int>(IOClientType::kMongo));
        AddStringMapVal("-s", "redis", static_cast<int>(IOClientType::kRedis));

        AddOpt("-caddr", ArgType::kString);
        AddOpt("-cport", ArgType::kInt);
        AddOpt("-p", ArgType::kSize);
        AddOpt("-path", ArgType::kString);
        AddOpt("-rfrac", ArgType::kFloat);
        AddOpt("-wfrac", ArgType::kFloat);
        AddOpt("-bs", ArgType::kSize);
        AddOpt("-fs", ArgType::kSize);
        AddOpt("-tot", ArgType::kSize);

        AddOpt("-ap", ArgType::kStringMap);
        AddStringMapVal("-ap", "seq", static_cast<int>(DistributionType::kNone));
        AddStringMapVal("-ap", "uniform", static_cast<int>(DistributionType::kUniform));

        AddOpt("-md_depth", ArgType::kInt);
        AddOpt("-md_fcnt", ArgType::kInt);
        AddOpt("-md_iter", ArgType::kInt);

        ArgIter(argc, argv);
        VerifyArgs();
    }
};

#endif //SYMBIOS_BENCHMARK_H
