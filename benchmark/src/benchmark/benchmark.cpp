
/*
 * An application to generate different workloads to test a distributed storage system
 *
 * TODO: Add timers. Add CSV.
 * */

#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>
#include <benchmark/benchmark.h>
#include "rng.h"
#include <common/debug.h>

DistributionPtr create_dist(BenchmarkArgs &args, size_t file_size, size_t block_size)
{
    DistributionType access_pattern = DistributionType::kNone;
    if(args.OptIsSet("-ap")) {
        access_pattern = static_cast<DistributionType>(args.GetIntOpt("-ap"));
    }

    DistributionPtr dist = DistributionFactory::Get(access_pattern);
    switch(access_pattern) {
        case DistributionType::kNone: {
            dist->Shape(block_size);
            break;
        }
        case DistributionType::kUniform: {
            dist->Shape(file_size - block_size);
            if(args.OptIsSet("-seed")) {
                dist->Seed(args.GetIntOpt("-seed"));
            }
            break;
        }
    }

    return std::move(dist);
}

size_t io_file_workload(IOClientPtr &fs, BenchmarkArgs &args)
{
    std::string path = args.GetStringOpt("-path");
    float rfrac = args.GetFloatOpt("-rfrac");
    float wfrac = args.GetFloatOpt("-wfrac");
    size_t block_size = args.GetSizeOpt("-bs");
    size_t file_size = args.GetSizeOpt("-fs");
    size_t total_io = args.GetSizeOpt("-tot");
    DistributionPtr dist;

    int direct = args.OptIsSet("-direct") ? FileMode::kDirect : 0;
    FilePtr fp = fs->Open(path, FileMode::kRead | FileMode::kWrite | FileMode::kCreate | direct);
    void *buffer = std::calloc(block_size, 1);

    dist = create_dist(args, file_size, block_size);
    double write_io = wfrac*total_io;
    for(size_t i = 0; i < write_io; i += block_size) {
        fp->Seek(dist->GetSize());
        fp->Write(buffer, block_size);
    }

    dist = create_dist(args, file_size, block_size);
    double read_io = rfrac*total_io;
    for(size_t i = 0; i < read_io; i += block_size) {
        fp->Read(buffer, block_size);
        fp->Seek(dist->GetSize());
    }

    free(buffer);

    return total_io;
}

int md_fs_workload(IOClientPtr &fs, BenchmarkArgs &args)
{
    int depth = args.GetIntOpt("-md_depth");
    int fcnt = args.GetIntOpt("-md_fcnt");

    std::string newdir;
    for(int i = 0; i < depth; ++i) {
        newdir += "/md-ex";
        fs->Mkdir(newdir);
    }
    for(int i = 0; i < fcnt; ++i) {
        std::string newfile = newdir + "/file" + std::to_string(i);
        FilePtr fp = fs->Open(newfile, FileMode::kWrite | FileMode::kCreate);
    }
    fs->Rmdir("/md-ex");

    return depth + fcnt;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    BenchmarkArgs args(argc, argv);
    int rank, nprocs = 1;
    int ops_per_proc = 0;
    size_t bytes_per_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    //Create connection to database or whatever...
    int storage_service = args.GetIntOpt("-s");
    IOClientPtr io = IOClientFactory::Get(static_cast<IOClientType>(storage_service));
    std::string addr = args.GetStringOpt("-caddr");
    int port = args.GetIntOpt("-cport");
    io->Connect(addr, port);

    //Run workloads
    common::debug::Timer t;
    t.startTime();
    int workload = args.GetIntOpt("-w");
    switch(static_cast<WorkloadType>(workload)) {
        case WorkloadType::kIoOnlyFs: {
            bytes_per_proc = io_file_workload(io, args);
            break;
        }
        case WorkloadType::kMdFs: {
            ops_per_proc = md_fs_workload(io, args);
            break;
        }
    }
    double local_end_time = t.endTime();
    double local_std_msec;

    //Get global statistics
    double avg_msec, std_msec, min_msec, max_msec;
    MPI_Allreduce(&local_end_time, &avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    local_std_msec = pow(local_end_time - avg_msec, 2);
    MPI_Reduce(&local_std_msec, &std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_end_time, &min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_end_time, &max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    avg_msec = avg_msec/nprocs;
    std_msec = std::sqrt(std_msec);

    //Get average bandwidth and throughput
    size_t tot_ops = ((size_t)ops_per_proc) * nprocs;
    size_t tot_bytes = bytes_per_proc * nprocs;
    double thrpt_kiops = ((double)tot_ops)/avg_msec;
    double bw_kbps = ((double)tot_bytes)/avg_msec;

    //Write to output CSV in root process
    if(rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,tot_ops,tot_bytes,thrpt_kiops,bw_kbps" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            tot_ops << "," <<
            tot_bytes << "," <<
            thrpt_kiops << "," <<
            bw_kbps << std::endl;
    }

    MPI_Finalize();
}


