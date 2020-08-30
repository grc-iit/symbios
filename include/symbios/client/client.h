#ifndef SYMBIOS_CLIENT_H
#define SYMBIOS_CLIENT_H

#include <basket.h>
#include <iostream>
#include <mpi.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/data_structure.h>

//#define SYMBIOS_FORWARD_DECL(name, ret, args) typedef ret(*__real_t_##name)args;
//#define MAP_OR_FAIL(func)                                                      \
//    __real_t_##func __real_##func;                                         \
//    __real_##func = (__real_t_##func)dlsym(RTLD_NEXT,#func);

namespace symbios{
        class Client {
            private:
                std::shared_ptr<RPC> rpc;
            public:
                Client();
                void StoreRequest(Data &request);
                void LocateRequest(Data &request);
            };
    }


#endif //SYMBIOS_CLIENT_H