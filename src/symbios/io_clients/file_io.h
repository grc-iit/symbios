//
// Created by mani on 8/24/2020.
// It is the implementation of File IO Client.
//

#ifndef SYMBIOS_FILE_IO_H
#define SYMBIOS_FILE_IO_H

#include <symbios/io_clients/io.h>

/*
 * An subclass which inherits from IOClient class
 * 1) implement the interface to access file io storage
 */
class FileIOClient: public IOClient  {
public:
    /**
     * Constructor
     * 1) Checking if the file directory exists.
     *    If the file directory is non exist, throw an exception.
     */
    FileIOClient(uint16_t storage_index):IOClient(storage_index){
        auto file_solution = std::static_pointer_cast<FileSS>(solution);
        if(!boost::filesystem::exists(file_solution->end_point_.c_str())){
            throw ErrorException(FILE_PATH_NON_EXISTANCE);
        }
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

    /*
     * Remove the data from the file io
     */
    bool Remove(Data &source) override;

    /*
     * Get the source file size
     */
    size_t Size(Data &source) override;

};


#endif //SYMBIOS_FILE_IO_H
