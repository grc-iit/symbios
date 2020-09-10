//
// Created by mani on 8/24/2020.
// It is the interface for all IO clients defined
//

#ifndef SYMBIOS_IO_H
#define SYMBIOS_IO_H

#include <symbios/common/data_structure.h>
#include <symbios/common/configuration_manager.h>

/*
 * An abstract class which provides io storage interface for other modules to access the io storage
 */
class IOClient {
protected:
    std::shared_ptr<StorageSolution> solution;
public:
    IOClient(uint16_t storage_index){
        auto iter = SYMBIOS_CONF->STORAGE_SOLUTIONS.find(storage_index);
        if(iter != SYMBIOS_CONF->STORAGE_SOLUTIONS.end()) solution = iter->second;
        else{
            //TODO: throw exception
            throw ErrorException(INVALID_STORAGE_INDEX);
        }
    }
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
    /**
     * Remove interface
     */
    virtual bool Remove(Data &source) = 0;
    /**
     * Size interface
     */
    virtual size_t Size(Data &source) = 0;
};


#endif //SYMBIOS_IO_H
