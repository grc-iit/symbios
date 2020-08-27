
/*
 * An application to generate different workloads to test a distributed storage system
 * */

#include <iostream>
#include <string>
#include <mpi.h>
#include <benchmark/benchmark.h>
#include <benchmark/rng.h>

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

void io_file_workload(IOClientPtr &fs, BenchmarkArgs &args)
{
    std::string path = args.GetStringOpt("-path");
    float rfrac = args.GetFloatOpt("-rfrac");
    float wfrac = args.GetFloatOpt("-wfrac");
    size_t block_size = args.GetSizeOpt("-bs");
    size_t file_size = args.GetSizeOpt("-fs");
    size_t total_io = args.GetSizeOpt("-tot");
    DistributionPtr dist;

    FilePtr fp = fs->Open(path, "r+");
    void *buffer = std::calloc(block_size, 1);

    dist = create_dist(args, file_size, block_size);
    double write_io = wfrac*total_io;
    for(size_t i = 0; i < write_io; i += block_size) {
        fp->Write(buffer, block_size);
        fp->Seek(dist->GetSize());
    }

    dist = create_dist(args, file_size, block_size);
    double read_io = rfrac*total_io;
    for(size_t i = 0; i < read_io; i += block_size) {
        fp->Read(buffer, block_size);
        fp->Seek(dist->GetSize());
    }

    free(buffer);
}

void md_fs_workload(IOClientPtr &fs, BenchmarkArgs &args)
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
        FilePtr fp = fs->Open(newfile, "w");
    }
    fs->Rmdir("/md-ex");
}

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    BenchmarkArgs args(argc, argv);
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int storage_service = args.GetIntOpt("-s");
    IOClientPtr io = IOClientFactory::Get(static_cast<IOClientType>(storage_service));
    io->Connect(args.GetStringOpt("-caddr"), args.GetIntOpt("-cport"));

    int workload = args.GetIntOpt("-w");
    switch(static_cast<WorkloadType>(workload)) {
        case WorkloadType::kIoOnlyFs: {
            io_file_workload(io, args);
            break;
        }
        case WorkloadType::kMdFs: {
            md_fs_workload(io, args);
            break;
        }
    }

    MPI_Finalize();
}


