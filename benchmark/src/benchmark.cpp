
/*
 * An application to generate different workloads to test a distributed storage system
 *
 * */

#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>
#include <benchmark/benchmark.h>
#include <benchmark/rng.h>
#include <debug.h>

void time_stats(double local_time_spent, int nprocs, double &avg_msec, double &std_msec, double &min_msec, double &max_msec)
{
    double local_std_msec;
    MPI_Allreduce(&local_time_spent, &avg_msec, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    local_std_msec = pow(local_time_spent - avg_msec, 2);
    MPI_Reduce(&local_std_msec, &std_msec, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time_spent, &min_msec, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time_spent, &max_msec, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    avg_msec = avg_msec/nprocs;
    std_msec = std::sqrt(std_msec);
}

void io_stats(BenchmarkArgs &args, int rank, double local_time_spent, int nprocs, size_t read_per_proc, size_t write_per_proc, size_t block_size)
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
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,block_size,tot_read,tot_write,tot_bytes,bw_read,bw_write,bw_kbps" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            block_size << "," <<
            tot_read << "," <<
            tot_write << "," <<
            tot_bytes << "," <<
            bw_read << "," <<
            bw_write << "," <<
            bw_kbps << std::endl;
    }
}

void md_stats(BenchmarkArgs &args, int rank, double local_time_spent, int nprocs, size_t ops_per_proc)
{
    //Get average time spent in application
    double avg_msec, std_msec, min_msec, max_msec;
    time_stats(local_time_spent, nprocs, avg_msec, std_msec, min_msec, max_msec);

    //Get average bandwidth and throughput
    size_t tot_ops = ops_per_proc * nprocs;
    double thrpt_kiops = ((double)tot_ops)/avg_msec;

    //Write to output CSV in root process
    if(rank == 0 && args.OptIsSet("-out")) {
        std::string output_path = args.GetStringOpt("-out");
        bool exists = boost::filesystem::exists(output_path);
        std::ofstream out(output_path, std::ofstream::out | std::ofstream::app);
        if(!exists) {
            out << "avg_msec,std_msec,min_msec,max_msec,nprocs,tot_ops,thrpt_kiops" << std::endl;
        }
        out <<
            avg_msec << "," <<
            std_msec << "," <<
            min_msec << "," <<
            max_msec << "," <<
            nprocs << "," <<
            tot_ops << "," <<
            thrpt_kiops << std::endl;
    }
}

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

void io_file_workload(IOClientPtr &fs, int rank, int nprocs, BenchmarkArgs &args)
{
    Timer t;
    t.startTime();

    std::string path = args.GetStringOpt("-path") + std::to_string(rank);
    float rfrac = args.GetFloatOpt("-rfrac");
    float wfrac = args.GetFloatOpt("-wfrac");
    size_t block_size = args.GetSizeOpt("-bs");
    size_t file_size = args.GetSizeOpt("-fs");
    size_t bytes_per_proc = args.GetSizeOpt("-tot");
    DistributionPtr dist;

    int direct = args.OptIsSet("-direct") ? FileMode::kDirect : 0;
    FilePtr fp = fs->Open(path, FileMode::kRead | FileMode::kWrite | FileMode::kCreate | direct);
    void *buffer = std::calloc(block_size, 1);

    dist = create_dist(args, file_size, block_size);
    size_t write_per_proc = wfrac*bytes_per_proc;
    for(size_t i = 0; i < write_per_proc; i += block_size) {
        fp->Seek(dist->GetSize());
        fp->Write(buffer, block_size);
    }

    dist = create_dist(args, file_size, block_size);
    size_t read_per_proc = rfrac*bytes_per_proc;
    for(size_t i = 0; i < read_per_proc; i += block_size) {
        fp->Read(buffer, block_size);
        fp->Seek(dist->GetSize());
    }

    //End Time
    double local_time_spent = t.endTime();
    free(buffer);

    //Get stats
    io_stats(args, rank, local_time_spent, nprocs, read_per_proc, write_per_proc, block_size);
}

void md_fs_workload(IOClientPtr &fs, int rank, int nprocs, BenchmarkArgs &args)
{
    //Start Time
    Timer t;
    t.startTime();

    int depth = args.GetIntOpt("-md_depth");
    int fcnt = args.GetIntOpt("-md_fcnt");
    size_t ops_per_proc = depth + fcnt;

    std::string dirstr = "/md-ex" + std::to_string(rank);
    std::string newdir;
    for(int i = 0; i < depth; ++i) {
        newdir += "/md-ex";
        fs->Mkdir(newdir);
    }
    for(int i = 0; i < fcnt; ++i) {
        std::string newfile = newdir + "/file" + std::to_string(i);
        FilePtr fp = fs->Open(newfile, FileMode::kWrite | FileMode::kCreate);
    }
    fs->Rmdir(dirstr);

    //End Time
    double local_time_spent = t.endTime();

    //Get stats
    md_stats(args, rank, local_time_spent, nprocs, ops_per_proc);
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


