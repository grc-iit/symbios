//
// Created by jaime on 8/28/2020.
//
#include <symbios/client/client.h>

SYMBIOS_FORWARD_DECL(fopen,FILE *,(const char *filename, const char *mode));
SYMBIOS_FORWARD_DECL(fclose,int,(FILE *stream));
SYMBIOS_FORWARD_DECL(fseek,int,(FILE *stream, long int offset, int origin));
SYMBIOS_FORWARD_DECL(fread,size_t,(void *ptr, size_t size, size_t count, FILE *stream));
SYMBIOS_FORWARD_DECL(fwrite,size_t,(const void *ptr, size_t size, size_t count, FILE *stream));

FILE *fopen(const char *filename, const char *mode) {
    return nullptr;
}

int fclose(FILE *stream) {
    return 0;
}

int fseek(FILE *stream, long int offset, int origin) {
    return 0;
}

size_t fread(void *ptr, std::size_t size, std::size_t count, FILE *stream) {
    return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
    return 0;
}