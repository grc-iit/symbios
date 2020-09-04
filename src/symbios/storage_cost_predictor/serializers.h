//
// Created by lukemartinlogan on 9/3/20.
//

#ifndef SYMBIOS_SERIALIZERS_H
#define SYMBIOS_SERIALIZERS_H

#include <string>
#include <cstring>

class BinaryDeserializer {
private:
    char *data_ = nullptr;
    size_t buf_off_ = 0, buf_size_ = 0;
public:
    BinaryDeserializer(size_t buf_size) : buf_size_(buf_size) {
        data_ = (char*)std::malloc(buf_size);
    }
    ~BinaryDeserializer() { free(data_); }

    bool ParseInt(int &num) {
        if(buf_off_ + sizeof(int) > buf_size_) { return false; }
        memcpy(&num, data_ + buf_off_, sizeof(int));
        buf_off_ += sizeof(int);
        return true;
    }

    size_t ParseUlong(size_t &num) {
        if(buf_off_ + sizeof(size_t) > buf_size_) { return false; }
        memcpy(&num, data_ + buf_off_, sizeof(size_t));
        buf_off_ += sizeof(size_t);
        return true;
    }

    float ParseFloat(float &num) {
        if(buf_off_ + sizeof(float) > buf_size_) { return false; }
        memcpy(&num, data_ + buf_off_, sizeof(float));
        buf_off_ += sizeof(float);
    }

    char *GetBuf() { return data_; }
    size_t GetSize() { return buf_off_; }
};

class BinarySerializer {
private:
    char *data_ = nullptr;
    size_t buf_off_ = 0, buf_size_;
public:
    BinarySerializer(size_t buf_size) : buf_size_(buf_size) {
        data_ = (char*)std::malloc(buf_size);
    }
    ~BinarySerializer() { free(data_); }

    bool WriteInt(int num) {
        if(buf_off_ + sizeof(int) > buf_size_) { return false; }
        std::memcpy(data_ + buf_off_ , &num, sizeof(int));
        buf_off_ += sizeof(int);
        return true;
    }

    bool WriteUlong(size_t num) {
        if(buf_off_ + sizeof(size_t) > buf_size_) { return false; }
        memcpy(data_, &num, sizeof(size_t));
        buf_off_ += sizeof(size_t);
        return true;
    }

    bool WriteFloat(float num) {
        if(buf_off_ + sizeof(float) > buf_size_) { return false; }
        memcpy(data_, &num, sizeof(float));
        buf_off_ += sizeof(float);
        return true;
    }

    char *GetBuf() { return data_; }
    size_t GetSize() { return buf_off_; }
};

#endif //SYMBIOS_SERIALIZERS_H
