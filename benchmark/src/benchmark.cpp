
/*
 * An application to generate different workloads to test a distributed storage system
 * */

#include <iostream>
#include <string>
#include <mpi.h>
#include <benchmark/benchmark.h>

void io_file_workload(IOClientPtr &fs, std::string path, float rfrac, float wfrac, size_t block_size, size_t file_size, size_t total_io, int access_pattern)
{
    FilePtr fp = fs->open(path, "r+");
    void *buffer = std::calloc(block_size, 1);

    double write_io = wfrac*total_io;
    for(size_t i = 0; i < write_io; i += block_size) {
        fp->write(buffer, block_size);
    }

    double read_io = rfrac*total_io;
    for(size_t i = 0; i < read_io; i += block_size) {
        fp->read(buffer, block_size);
    }
}

void io_kvs_workload(IOClientPtr &kvs, std::string path, float rfrac, float wfrac, size_t block_size, size_t file_size, size_t total_io, int access_pattern)
{
}

void md_fs_workload(IOClientPtr &fs, int depth, int fcnt)
{
    std::string newdir = "/ex";
    for(int i = 0; i < depth; ++i) {
        newdir += "/ex";
        fs->mkdir(newdir);
    }
    for(int i = 0; i < fcnt; ++i) {
        std::string newfile = newdir + "/file" + std::to_string(i);
        fs->open(newfile, "w");
    }
    fs->rmdir("/ex");
}

void md_file_workload(IOClientPtr &fs, std::string path)
{

}

void md_kvs_workload(IOClientPtr &kvs, std::string path)
{
}

int main(int argc, char **argv)
{
    MPI_Init(&argc,&argv);
    BenchmarkArgs args(argc, argv);
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    MPI_Finalize();
}


