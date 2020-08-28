//
// Created by mani on 8/24/2020.
// It is the interface for all IO clients defined
//

#ifndef SYMBIOS_IO_H
#define SYMBIOS_IO_H

#include <symbios/common/data_structure.h>

class IOClient {
public:
    /**
     * Virtual Methods
     */
     /**
      * Reads data from source into destination buffer while respecting the position_
      */
     virtual void Read(Data &source, Data &destination) = 0;

     /**
      * Writes data from source into destination buffer while respecting the position_
      *
      */
     virtual void Write(Data &source, Data &destination) = 0;
};


#endif //SYMBIOS_IO_H
