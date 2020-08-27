//
// Created by mani on 8/24/2020.
// It is the implementation of Key-Value Store IO
//

#ifndef SYMBIOS_KEY_VALUE_STORE_IO_H
#define SYMBIOS_KEY_VALUE_STORE_IO_H

#include "io.h"

class KeyValueStoreIOClient: public IOClient{
public:
    /*
     * Constructor
     */
    KeyValueStoreIOClient(){}

    /*
     * Reads data from source into destination buffer while respecting the position_
     */
    void Read(Data &source, Data &destination) override {};

    /*
     * Write data to from source into destination buffer while respecting the position_
     */
    void Write(Data &source, Data &destination) override {};
};


#endif //SYMBIOS_KEY_VALUE_STORE_IO_H
