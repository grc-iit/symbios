//
// Created by lukemartinlogan on 8/27/20.
//

#ifndef SYMBIOS_KVS_H
#define SYMBIOS_KVS_H

#include <string>

class KVS {
public:
    virtual void SetKey(std::string key, std::string value="") = 0;
    virtual bool HasKey(std::string key) = 0;
    virtual void GetKey(std::string key, std::string &value) = 0;
    virtual void RemoveKey(std::string key) = 0;
};

#endif //SYMBIOS_KVS_H
