//
// Created by mani on 8/24/2020.
//

#include <symbios/client/client.h>
#include <common/debug.h>

int main(int argc, char * argv[]){
    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);

    int ioMode, ioOperation, requestSize, requestNumber;
    std::string distributionMode;

    if(argc > 1){
        SYMBIOS_CONF->CONFIGURATION_FILE=argv[1];
        ioMode = std::stoi(argv[2]);
        ioOperation = std::stoi(argv[3]);
        distributionMode = argv[4];
        requestSize = std::stoi(argv[5]);
        requestNumber = std::stoi(argv[6]);
    }
    BASKET_CONF->BACKED_FILE_DIR=SYMBIOS_CONF->SERVER_DIR;
    auto client = symbios::Client();
    printf("Client Setup\n");
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Changing Conf\n");
    auto solution = SYMBIOS_CONF->STORAGE_SOLUTIONS[ioMode];
    auto newSSMap = std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>>();
    auto new_SS = std::pair<uint16_t, std::shared_ptr<StorageSolution>>(0, solution);
    newSSMap.insert(new_SS);
    SYMBIOS_CONF->STORAGE_SOLUTIONS = newSSMap;

    if(distributionMode == "RANDOM_POLICY") SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY=RANDOM_POLICY;
    else if(distributionMode == "ROUND_ROBIN_POLICY") SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY=ROUND_ROBIN_POLICY;
    else if(distributionMode == "HEURISTICS_POLICY") SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY=HEURISTICS_POLICY;
    else if(distributionMode == "DYNAMIC_PROGRAMMING_POLICY") SYMBIOS_CONF->DATA_DISTRIBUTION_POLICY=DYNAMIC_PROGRAMMING_POLICY;
    else std::cerr << "Incorrect configuration on Data Distribution Policy" << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);
    printf("Done Conf\n");
    auto val = std::string(requestSize, '*');
    auto data = Data();
    data.id_="filename2";
    data.position_=0;
    data.buffer_ = val.data();
    data.buffer_[ val.size()]='\0';
    data.data_size_= val.size()+1;
    data.storage_index_=0;
    client.Delete(data);
    for(int i=0; i < requestNumber; i++){

        if(ioOperation == 0 || ioOperation == 2){
            printf("Sending Data\n");
            client.StoreRequest(data);
            printf("Data Sent %s\n",data.buffer_);
        }
        data.buffer_ = static_cast<char *>(malloc(data.data_size_));;
        data.storage_index_=1;
        if(ioOperation == 0 || ioOperation == 2){
            printf("Reading Data\n");
            client.LocateRequest(data);
            printf("Data recieved %s\n",data.buffer_);
            client.Delete(data);
        }
        COMMON_DBGVAR(data.buffer_);
    }
    MPI_Finalize();
    return 0;
}