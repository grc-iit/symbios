//
// Created by mani on 8/24/2020.
// It is the implementation of File IO Client.
//

#ifndef SYMBIOS_FILE_IO_H
#define SYMBIOS_FILE_IO_H

#include <symbios/io_clients/io.h>

class FileIOClient: public IOClient  {
public:
    /**
     * Constructor
     */
    FileIOClient(uint16_t storage_index):IOClient(storage_index){
    }

    /*
     * Methods
     */

    /*
     * Reads data from source into destination buffer while respecting the position_
     */
    void Read(Data &source, Data &destination) override;

    /*
     * Writes data from source into destination buffer while respecting the position_
     */
    void Write(Data &source, Data &destination) override;

};


#endif //SYMBIOS_FILE_IO_H
