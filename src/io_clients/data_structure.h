//
// Created by Jie on 8/26/20.
// Define the data structures used in
//

#ifndef SYMBIOS_DATA_STRUCTURE_H
#define SYMBIOS_DATA_STRUCTURE_H

const size_t MAX_STRING_LENGTH=256;

typedef struct CharStruct{
private:
    char value[MAX_STRING_LENGTH];
public:
    CharStruct(){}
    CharStruct(std::string data_){
        sprintf(this->value,"%s",data_.c_str());
    }
    CharStruct(char* data_, size_t size){
        sprintf(this->value,"%s",data_);
    }
    const char* c_str() const{
        return value;
    }

    char* data(){
        return value;
    }
    const size_t size() const{
        return strlen(value);
    }
    /* equal operator for comparing two Matrix. */
    bool operator==(const CharStruct &o) const {
        return strcmp(value,o.value)==0;
    }
} CharStruct;

typedef struct Data{
    CharStruct id_; // for file_io, the id_ is the filename; for Key-Value store io, the id_ is the key.
    long position_; // read or write start position
    void* buffer_;  // data
    long data_size_;

    /*Define the default, copy and move constructor*/
    Data():id_(),position_(0),buffer_(),data_size_(0){}
    Data(const Data &other): id_(other.id_), position_(other.position_),buffer_(other.buffer_),data_size_(other.data_size_){}
    Data(Data &other): id_(other.id_), position_(other.position_), buffer_(other.buffer_), data_size_(other.data_size_){}

    /*Define Assignment Operator*/
    Data &operator=(const Data &other){
        id_ = other.id_;
        position_ = other.position_;
        buffer_ = other.buffer_;
        data_size_ = other.data_size_;
    }
} Data;


#endif //SYMBIOS_DATA_STRUCTURE_H
