//
// Created by lukemartinlogan on 8/25/20.
//

#ifndef SYMBIOS_BENCHMARK_H
#define SYMBIOS_BENCHMARK_H

#include <iostream>
#include <benchmark/arguments.h>
#include <benchmark/file.h>
#include <benchmark/io_client.h>
#include <benchmark/io_client_factory.h>

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
        std::cout << "\tio-only-fs" << std::endl;
        std::cout << "\tio-only-kvs" << std::endl;
        std::cout << "\tmd-fs" << std::endl;
        std::cout << "\tmd-file" << std::endl;
        std::cout << "\tmd-kvs" << std::endl;

        std::cout << "-s [string]: Which storage service to use for the workload" << std::endl;
        std::cout << "\torangefs" << std::endl;
        std::cout << "\tmongodb" << std::endl;
        std::cout << "\tredis" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "OrangeFS Parameters" << std::endl;
        std::cout << "-caddr [string]: The local directory where the OrangeFS client is mounted" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "Redis/MongoDB Parameters" << std::endl;
        std::cout << "-caddr [string]: The IP address of the MongoDB server." << std::endl;
        std::cout << "-cport [string]: The port the MongoDB server runs on." << std::endl;
        std::cout << "" << std::endl;

        std::cout << "io-only-fs workload parameters" << std::endl;
        std::cout << "-p [size]: Preallocate a file of this size" << std::endl;
        std::cout << "-rfrac [float]: The percent of I/O to dedicate to reads" << std::endl;
        std::cout << "-wfrac [float]: The percent of I/O to dedicate to writes" << std::endl;
        std::cout << "-bs [size]: The unit of I/O ops" << std::endl;
        std::cout << "-fs [size]: The size of the file to perform I/O with" << std::endl;
        std::cout << "-tot [size]: The total amount of I/O to perform" << std::endl;
        std::cout << "-ap [string]: The access pattern for I/O ops" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "md-fs workload parameters" << std::endl;
        std::cout << "-md_depth [int]: The number of directories to nest" << std::endl;
        std::cout << "-md_fcnt [int]: The number of files to create in the deepest directory" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "md-file workload parameters" << std::endl;
        std::cout << "-md_iter [int]: The number of times to fopen/fseek/fclose file" << std::endl;
        std::cout << "" << std::endl;

        std::cout << "md-kvs workload parameters" << std::endl;
        std::cout << "-md_iter [int]: The number of times to add/check/delete key" << std::endl;
        std::cout << "" << std::endl;
    }

    BenchmarkArgs(int argc, char **argv) {
        AddOpt("-w", ArgType::kString);
        AddOpt("-s", ArgType::kString);
        AddOpt("-caddr", ArgType::kString);
        AddOpt("-cport", ArgType::kInt);
        AddOpt("-p", ArgType::kSize);
        AddOpt("-rfrac", ArgType::kFloat);
        AddOpt("-wfrac", ArgType::kFloat);
        AddOpt("-bs", ArgType::kSize);
        AddOpt("-fs", ArgType::kSize);
        AddOpt("-tot", ArgType::kSize);
        AddOpt("-ap", ArgType::kString);
        AddOpt("-md_depth", ArgType::kInt);
        AddOpt("-md_fcnt", ArgType::kInt);
        AddOpt("-md_iter", ArgType::kInt);
        ArgIter(argc, argv);
        VerifyArgs();
    }
};

#endif //SYMBIOS_BENCHMARK_H
