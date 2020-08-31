
/*
 * An application to generate different workloads to test a distributed storage system
 *
 * */

#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>
#include <common/rng.h>
#include <common/debug.h>
#include "benchmark.h"

void time_stats(double local_time_spent, int nprocs, double &avg_msec, double &std_msec, double &min_msec, double &max_msec)
{
    double local_std_msec;
    MPI_Allreduce(&local_time_spent, &avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    avg_msec = avg_msec/nprocs;
    local_std_msec = pow(local_time_spent - avg_msec, 2);
    MPI_Reduce(&local_std_msec, &std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time_spent, &min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time_spent, &max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    std_msec = std::sqrt(std_msec);
}

void io_stats(
        BenchmarkArgs &args, int rank, double local_time_spent,
        int nprocs, size_t read_per_proc, size_t write_per_proc,
        size_t block_size, DistributionType ap)
{
    //Get average time spent in application
    double avg_msec, std_msec, min_msec, max_msec;
    time_stats(local_time_spent, nprocs, avg_msec, std_msec, min_msec, max_msec);

    //Get average bandwidth and throughput
    size_t tot_read = read_per_proc * nprocs;
    size_t tot_write = write_per_proc * nprocs;
    size_t tot_bytes = tot_read + tot_write;
    double bw_read = ((double)tot_read)/avg_msec;
    double bw_write = ((double)tot_write)/avg_msec;
    double bw_kbps = ((double)tot_bytes)/avg_msec;

    //Write to output CSV in root process
    if(rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,block_size,ap,tot_read,tot_write,tot_bytes,bw_read,bw_write,bw_kbps" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            block_size << "," <<
            static_cast<int>(ap) << "," <<
            tot_read << "," <<
            tot_write << "," <<
            tot_bytes << "," <<
            bw_read << "," <<
            bw_write << "," <<
            bw_kbps << std::endl;
    }
}

void md_stats(BenchmarkArgs &args, int rank, double local_time_spent, int nprocs, size_t md_fcnt_per_proc, size_t md_dir_per_proc)
{
    //Get average time spent in application
    double avg_msec, std_msec, min_msec, max_msec;
    time_stats(local_time_spent, nprocs, avg_msec, std_msec, min_msec, max_msec);

    //Get average bandwidth and throughput
    size_t md_fcnt = md_fcnt_per_proc * nprocs;
    size_t md_dir = md_dir_per_proc * nprocs;
    size_t tot_ops = md_fcnt + md_dir;
    double thrpt_kiops = ((double)tot_ops)/avg_msec;

    //Write to output CSV in root process
    if(rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,md_fcnt,md_dir,tot_ops,thrpt_kiops" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            md_fcnt << "," <<
            md_dir << "," <<
            tot_ops << "," <<
            thrpt_kiops << std::endl;
    }
}

DistributionPtr create_dist(BenchmarkArgs &args, size_t file_size, size_t block_size, DistributionType &access_pattern)
{
    access_pattern = DistributionType::kNone;
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

void prealloc(IOClientPtr &fs, int rank, int nprocs, BenchmarkArgs &args)
{
    std::string path = args.GetStringOpt("-path") + std::to_string(rank);
    size_t file_size = args.GetSizeOpt("-fs");
    size_t block_size = (1<<24);

    FilePtr fp = fs->Open(path, FileMode::kWrite | FileMode::kCreate);
    void *buffer = std::calloc(block_size, 1);
    size_t file_size_per_proc = file_size / nprocs;

    for(size_t i = 0; i < file_size_per_proc; i += block_size) {
        size_t write_size = (i+block_size)<=file_size_per_proc ? block_size : file_size_per_proc - i;
        fp->Write(buffer, write_size);
    }
}

void io_file_workload(IOClientPtr &fs, int rank, int nprocs, BenchmarkArgs &args)
{
    common::debug::Timer t;
    std::string path = args.GetStringOpt("-path") + std::to_string(rank);
    float rfrac = args.GetFloatOpt("-rfrac");
    float wfrac = args.GetFloatOpt("-wfrac");
    size_t block_size = args.GetSizeOpt("-bs");
    size_t file_size_per_proc = args.GetSizeOpt("-fs")/nprocs;
    size_t tot_bytes = args.GetSizeOpt("-tot");
    DistributionType access_pattern;
    DistributionPtr dist;

    int direct = args.OptIsSet("-direct") ? FileMode::kDirect : 0;
    FilePtr fp = fs->Open(path, FileMode::kRead | FileMode::kWrite | direct);
    void *buffer = std::calloc(block_size, 1);

    //Start Time
    t.startTime();

    //Write to file
    MPI_Barrier(MPI_COMM_WORLD);
    dist = create_dist(args, file_size_per_proc, block_size, access_pattern);
    size_t write_per_proc = wfrac*tot_bytes/nprocs;
    for(size_t i = 0; i < write_per_proc; i += block_size) {
        fp->Seek(dist->GetSize());
        fp->Write(buffer, block_size);
    }

    //Read from file
    MPI_Barrier(MPI_COMM_WORLD);
    dist = create_dist(args, file_size_per_proc, block_size, access_pattern);
    size_t read_per_proc = rfrac*tot_bytes/nprocs;
    for(size_t i = 0; i < read_per_proc; i += block_size) {
        fp->Seek(dist->GetSize());
        fp->Read(buffer, block_size);
    }

    //End Time
    double local_time_spent = t.endTime();
    free(buffer);

    //Get stats
    io_stats(args, rank, local_time_spent, nprocs, read_per_proc, write_per_proc, block_size, access_pattern);
}

void md_fs_workload(IOClientPtr &fs, int rank, int nprocs, BenchmarkArgs &args)
{
    int fcnt = args.GetSizeOpt("-md_fcnt")/nprocs;
    int depth = args.GetIntOpt("-md_depth");
    std::string path = args.GetStringOpt("-path");

    //Start Time
    common::debug::Timer t;
    t.startTime();

    std::string dirstr = "/md-ex" + std::to_string(rank);
    std::string newdir = path;
    for(int i = 0; i < depth; ++i) {
        newdir += dirstr;
        fs->Mkdir(newdir);
    }
    for(int i = 0; i < fcnt; ++i) {
        std::string newfile = newdir + "/file" + std::to_string(i);
        FilePtr fp = fs->Open(newfile, FileMode::kWrite | FileMode::kCreate);
    }
    fs->Rmdir(path + dirstr);

    //End Time
    double local_time_spent = t.endTime();

    //Get stats
    md_stats(args, rank, local_time_spent, nprocs, fcnt, depth);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    BenchmarkArgs args(argc, argv);
    int rank, nprocs = 1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    //Create connection to database or whatever...
    int storage_service = args.GetIntOpt("-s");
    IOClientPtr io = IOClientFactory::Get(static_cast<IOClientType>(storage_service));
    std::string addr = args.GetStringOpt("-caddr");
    int port = args.GetIntOpt("-cport");
    io->Connect(addr, port);

    //Run workloads
    int workload = args.GetIntOpt("-w");
    switch(static_cast<WorkloadType>(workload)) {
        case WorkloadType::kPrealloc: {
            prealloc(io, rank, nprocs, args);
            break;
        }
        case WorkloadType::kIoOnlyFs: {
            io_file_workload(io, rank, nprocs, args);
            break;
        }
        case WorkloadType::kMdFs: {
            md_fs_workload(io, rank, nprocs, args);
            break;
        }
    }

    MPI_Finalize();
}


