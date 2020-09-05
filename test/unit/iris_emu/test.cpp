//
// Created by neeraj on 9/1/20.
//

#include <common/iris.h>
#include <common/debug.h>
#include <symbios/client/client.h>

#define  MAX_OBJ_SIZE 4


std::ostream &operator<<(std::ostream &os, DataDescriptor &m) {
    return os << "{position_:" << m.position_ << ","
              << "size:" << m.size << "}";
}

int main(int argc, char * argv[]){
    auto tracer=common::debug::AutoTrace(std::string("iris_emu::main"));
    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);

    if (argc == 1) {
        std::cout<<"cmd: ./unit_iris_emu <conf> <raw_data> <file/mongo/redis> <file_name> <chunk_size>\n";
        exit(0);
    }

    std::string distributionMode;
    if(argc > 1){
        SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
    }

    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;

    const char *data = argc > 2 ? argv[2] : "Hello Symbios";  // argv[2]

    auto type_ = argc > 3 ?
            ( strcmp("file", argv[3]) == 0  ? IOClientType::FILE_IO :
            strcmp("mongo", argv[3]) == 0 ? IOClientType::MONGO_IO :
            strcmp("redis", argv[3]) == 0? IOClientType::REDIS_IO : IOClientType::FILE_IO ): IOClientType ::FILE_IO;

    auto file_ = argc > 4 ? argv[4] : "iris_emu_4";  // argv[4]
    auto max_obj_size = argc > 5 ? std::atoi(argv[5]) : MAX_OBJ_SIZE;  // argv[5]

    LibHandler lh = LibHandler(file_, IOLib::IRIS, type_, max_obj_size);
    char *input = strdup(data);
    lh.run(OPType::FOPEN, 0, 0, NULL);
    lh.run(OPType::WRITE, 0, strlen(data), input);
    char *result = (char *)malloc(strlen(data) + 1);
    lh.run(OPType::READ, 0, strlen(data), result);
    lh.run(OPType::FCLOSE, 0, 0, NULL);
    std::cout << result << std::endl;
    free(input);
    free(result);

    MPI_Finalize();
    return 0;
}
