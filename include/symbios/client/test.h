#ifndef SYMBIOS_TEST_H
#define SYMBIOS_TEST_H

#include <basket.h>
#include <iostream>
#include <mpi.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/data_structure.h>
#include "dlfcn.h"

namespace symbios{
        class Client {
            private:
                std::shared_ptr<RPC> rpc;
//                basket::unordered_map<FILE*,CharStruct> fileDescriptorMap;
            public:
                Client();
                void StoreRequest(Data &request);
                void LocateRequest(Data &request);

                bool addOrUpdateFileDescriptorPool(FILE* fh, CharStruct filename);
                bool isFileDescriptorTracked(FILE* fh);
                bool deleteFileDescriptorFromPool(FILE* fh);
                std::pair<bool, CharStruct> getFileNameFromMap(FILE* fh);
            };
    }


#endif //SYMBIOS_TEST_H