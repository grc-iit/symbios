//
// Created by jaime on 8/28/2020.
//
#include <symbios/client/client.h>
#include "dlfcn.h"

#define SYMBIOS_FORWARD_DECL(name, ret, args) typedef ret(*__real_t_##name)args;
#define MAP_OR_FAIL(func)                                                      \
    __real_t_##func __real_##func;                                         \
    __real_##func = (__real_t_##func)dlsym(RTLD_NEXT,#func);

SYMBIOS_FORWARD_DECL(fopen,FILE *,(const char *filename, const char *mode));
SYMBIOS_FORWARD_DECL(fclose,int,(FILE *stream));
SYMBIOS_FORWARD_DECL(fseek,int,(FILE *stream, long int offset, int origin));
SYMBIOS_FORWARD_DECL(fread,size_t,(void *ptr, size_t size, size_t count, FILE *stream));
SYMBIOS_FORWARD_DECL(fwrite,size_t,(const void *ptr, size_t size, size_t count, FILE *stream));

FILE *fopen(const char *filename, const char *mode) {
    FILE* fh;
    auto client = basket::Singleton<symbios::Client>::GetInstance();

    MAP_OR_FAIL(fopen);
    fh=__real_fopen(filename,mode);  // TODO: Issue, this creates the actual file
    if(strncmp(filename, SYMBIOS_CONF->POSIX_MOUNT_POINT.c_str(), SYMBIOS_CONF->POSIX_MOUNT_POINT.size()) == 0){
        //intercept
        client->addOrUpdateFileDescriptorPool(fh);
        return 0;
    return fh;
  }
}

int fclose(FILE *stream) {
    auto client = basket::Singleton<symbios::Client>::GetInstance();

    if(client->isFileDescriptorTracked(stream)){
        //intercept
        client->deleteFileDescriptorFromPool(stream);
        return 0;
    }
    MAP_OR_FAIL(fclose);
    return __real_fclose(stream);
}

int fseek(FILE *stream, long int offset, int origin) {
    auto client = basket::Singleton<symbios::Client>::GetInstance();

    if(client->isFileDescriptorTracked(stream)){
        //intercept
        return 0;
    }
    MAP_OR_FAIL(fseek);
    return __real_fseek(stream, offset, origin);
}

size_t fread(void *ptr, std::size_t size, std::size_t count, FILE *stream) {
    auto client = basket::Singleton<symbios::Client>::GetInstance();

    if(client->isFileDescriptorTracked(stream)){
//        CharStruct id_; // for file io, the "id_" is the filename; for object store io, the "id_" is the key.
//        long position_; // read/write start position
//        void* buffer_;  // data content
//        long data_size_; // data size need to read/write
//        IOClientType io_client_type_; // io client type
        auto data = Data();
        //get the filename
        auto filename = client->getFileNameFromMap(stream);
        if(filename.first) data.id_ = filename.second;
        else return filename.first;
        //set the other variables
        data.data_size_= size;
        //TODO: set the position in case of a previous fseek;
        client->LocateRequest(data);
        return data.data_size_;
    }
    MAP_OR_FAIL(fread);
    return __real_fread(ptr,size,count,stream);
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
    auto client = basket::Singleton<symbios::Client>::GetInstance();

    if(client->isFileDescriptorTracked(stream)){
        auto data = Data();
        //get the filename
        auto filename = client->getFileNameFromMap(stream);
        if(filename.first) data.id_ = filename.second;
        else return filename.first;
        //set the other variables
        data.data_size_=size;
        data.buffer_=(void*)ptr;
        client->StoreRequest(data);
        return 0;
    }
    MAP_OR_FAIL(fwrite);
    return __real_fwrite(ptr,size,count,stream);
}