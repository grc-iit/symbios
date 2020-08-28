//
// Created by mani on 8/24/2020.
// It is the implementation of Object Store IO
//

#ifndef SYMBIOS_OBJECT_STORE_IO_H
#define SYMBIOS_OBJECT_STORE_IO_H

#include <symbios/io_clients/io.h>

class ObjectStoreIOClient: public IOClient{
public:
    /*
     * Constructor
     */
    ObjectStoreIOClient(){}

    /*
     * Reads data from source into destination buffer while respecting the position_
     */
    void Read(Data &source, Data &destination) override {};

    /*
     * Write data to from source into destination buffer while respecting the position_
     */
    void Write(Data &source, Data &destination) override {};
};

#endif //SYMBIOS_OBJECT_STORE_IO_H
