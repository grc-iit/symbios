//
// Created by mani on 9/1/2020.
//

#ifndef SYMBIOS_POSIX_H
#define SYMBIOS_POSIX_H

#include <cstdio>
#include <basket/common/data_structures.h>
#include "dlfcn.h"

#define FILE_BUFFER_CAPACITY 4096

#define SYMBIOS_FORWARD_DECL(name, ret, args) typedef ret(*__real_t_##name)args;
#define MAP_OR_FAIL(func)                                                      \
    __real_t_##func __real_##func;                                         \
    __real_##func = (__real_t_##func)dlsym(RTLD_NEXT,#func);

SYMBIOS_FORWARD_DECL(fopen,FILE *,(const char *filename, const char *mode));
SYMBIOS_FORWARD_DECL(fclose,int,(FILE *stream));
SYMBIOS_FORWARD_DECL(fseek,int,(FILE *stream, long int offset, int origin));
SYMBIOS_FORWARD_DECL(fread,size_t,(void *ptr, size_t size, size_t count, FILE *stream));
SYMBIOS_FORWARD_DECL(fwrite,size_t,(const void *ptr, size_t size, size_t count, FILE *stream));
struct PosixStat{
    CharStruct filename_;
    size_t file_pointer_;
    size_t file_size_;
    CharStruct mode_;
    uint16_t storage_index;
    /*Define the default, copy and move constructor*/
    PosixStat(): filename_(), file_pointer_(), file_size_(),storage_index(),mode_(){}
    PosixStat(const PosixStat &other): filename_(other.filename_),
                                       file_pointer_(other.file_pointer_), file_size_(other.file_size_), storage_index(other.storage_index),mode_(other.mode_){}
    PosixStat(PosixStat &other): filename_(other.filename_),
                                 file_pointer_(other.file_pointer_), file_size_(other.file_size_), storage_index(other.storage_index),mode_(other.mode_){}

    /*Define Assignment Operator*/
    PosixStat &operator=(const PosixStat &other){
        filename_ = other.filename_;
        file_pointer_ = other.file_pointer_;
        file_size_ = other.file_size_;
        mode_=other.mode_;
        storage_index = other.storage_index;
        return *this;
    }
};
namespace symbios{
    class Posix{

        std::unordered_map<FILE*,PosixStat> fileDescriptorMap_;
    private:
    public:
        Posix(): fileDescriptorMap_(){}
        bool UpdateOnOpen(FILE* fh, PosixStat &stat);
        bool isFileDescriptorTracked(FILE* fh);
        bool UpdateOnClose(FILE* fh);
        int UpdateOnSeek(FILE* fh,long int offset, int origin);
        std::pair<bool, PosixStat> GetStat(FILE* fh);
        bool UpdateStat(FILE* fh,PosixStat &stat);
    };
}
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *stream);
int fseek(FILE *stream, long int offset, int origin);
size_t fread(void *ptr, std::size_t size, std::size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
#endif //SYMBIOS_POSIX_H
