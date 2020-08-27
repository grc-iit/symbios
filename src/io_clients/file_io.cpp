//
// Created by mani on 8/24/2020.
//

#include "file_io.h"

void FileIOClient::Read(Data &source, Data &destination) {
    char* fileName = source.id_.c_str();
    FILE* fp = fopen(fileName, "r");
    fseek(fp, source.position_, SEEK_SET);
    size_t size = source.data_size_;
    fread(destination.buffer_, size, 1, fh);
    fclose(fp);
}

void FileIOClient::Write(Data &source, Data &destination) {
    char* destFileName = destination.id_.c_str();
    FILE* fp = fopen(destFileName, "r+");
    if(fp == NULL){
        fp = fopen(destFileName, "w+");
    }
    fseek(fp, destination.position_, SEEK_SET);
    fwrite(source.buffer_, source.data_size_, 1, fp);
    fclose(fp);
}