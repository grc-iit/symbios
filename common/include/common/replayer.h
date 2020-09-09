//
// Created by anthony on 3/1/17.
//

#ifndef COMMON_REPLAYER_H
#define COMMON_REPLAYER_H

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <mpi.h>
#include <iomanip>
#include <common/debug.h>
#include <mpi.h>
#include <common/iris.h>

class trace_replayer {
public:
    static int replay_trace(std::string traceFile,
                            std::string filename, int repetitions,
                            int rank, IOLib mode, uint16_t stor_type, uint16_t chunk_size, std::string symbios_conf="") {
        /*Initialization of some stuff*/
        FILE* trace;
        FILE *file;
        char* line = NULL;
        char *writebuf;
        char *readbuf;
        int comm_size;
        size_t len=0;
        ssize_t readsize;
        std::string operation;
        long offset = 0;
        long request_size = 0;
        char* word;
        LibHandler lh = LibHandler(filename, mode, stor_type, chunk_size, false, symbios_conf);
        line = (char*) malloc(128);
        int i;
        for (i = 0; i < 128; i++) {
            line[i] = '\0';
        }
        std::vector<double> timings;
        double average=0;
        int rep = repetitions;

        /* Do the I/O and comparison*/
        while(rep) {
            /*Opening the trace file*/
            trace = std::fopen(traceFile.c_str(), "r");
            if (trace==NULL) {
                return 0;
            }
            /*While loop to read each line from the trace and create I/O*/
            common::debug::Timer globalTimer = common::debug::Timer();
            globalTimer.startTime();
            auto now = globalTimer.getTimeElapsed();
            std::cout << traceFile << "," << now << std::endl;
            int lineNumber=0;
            while ((readsize = getline(&line, &len, trace)) != -1) {
                if (readsize < 4) {
                    break;
                }
                operation.clear();
                word = strtok(line, ",");
                operation = word;
                word = strtok(NULL, ",");
                offset = atol(word);
                word = strtok(NULL, ",");
                request_size = atol(word);
                common::debug::Timer operationTimer = common::debug::Timer();
                operationTimer.startTime();
                if (operation == "FOPEN") {
                    std::cout << "open" << std::endl;
                    if (mode == IOLib::POSIX) {
                        file = fopen((filename+std::to_string(rank)).c_str(), "w+");
                    }
                    else {
                        lh.run(OPType::FOPEN, 0, 0, NULL);
                    }
                } else if (operation == "FCLOSE") {
                    std::cout << "close" << std::endl;
                    if (mode == IOLib::POSIX) {
                        fclose(file);
                    }
                    else {
                        lh.run(OPType::FCLOSE, 0, 0, NULL);
                    }
                } else if (operation == "WRITE") {
                    std::cout << "write" << std::endl;
                    writebuf = (char *)randstring(request_size);
                    if (mode == IOLib::POSIX) {
                        fseek(file, (size_t) offset, SEEK_SET);
                        fwrite(writebuf, sizeof(char), (size_t) request_size, file);
                    }
                    else {
                        lh.run(OPType::WRITE, offset, request_size, writebuf);
                    }
                    free(writebuf);
                } else if (operation == "READ") {
                    std::cout << "read" << std::endl;
                    if (mode == IOLib::POSIX) {
                        readbuf = (char*) malloc((size_t) request_size + 1);
                        fseek(file, (size_t) offset, SEEK_SET);
                        fread(readbuf, sizeof(char), (size_t) request_size, file);
                        free(readbuf);
                    }
                    else {
                        lh.run(OPType::READ, offset, request_size, readbuf);
                    }
                } else if (operation == "LSEEK") {
                    std::cout << "seek" << std::endl;
                    if (mode == IOLib::POSIX) {
                        fseek(file, (size_t) offset, SEEK_SET);
                    }
                    else {
                        lh.run(OPType::LSEEK, 0, 0, NULL);
                    }
                }
                operationTimer.endTime();
                lineNumber++;
            }
            std::cout << "Symbios,";
            timings.emplace_back(globalTimer.endTime());
            rep--;
            std::fclose(trace);
        }
        for(auto timing:timings){
            average +=timing;
        }
        average=average/repetitions;
        double global_time;
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Allreduce(&average, &global_time, 1, MPI_DOUBLE, MPI_SUM,
                      MPI_COMM_WORLD);
        double mean = global_time / comm_size;

        if(rank == 0) {
            printf("Time : %lf\n",mean);
            std::cout << "Symbios,"
                      << "average,"
                      << std::setprecision(6)
                      << average/repetitions
                      << "\n";
        }
        free(line);

        /*if( remove( "/home/anthony/temp/" ) != 0 )
          perror( "Error deleting file" );*/
        return 0;
    }

    static int prepare_data(std::string traceFile, std::string filename, int repetitions, int rank, IOLib mode, uint16_t stor_type, uint16_t chunk_size, std::string symbios_conf="") {
      FILE* trace;
      FILE *file;
      char* line = NULL;
      char *writebuf;
      size_t len=0;
      ssize_t readsize;
      std::string operation;
      long offset = 0;
      long request_size = 0;
      char* word;
      LibHandler lh = LibHandler(filename, mode, stor_type, chunk_size, false, symbios_conf);
      line = (char*) malloc(128);
      int lineNumber=0;

      /* putting down the data, file for PFS and objects for Hyperdex*/

      trace = fopen(traceFile.c_str(), "r");

      long max_offset = 0;
      while ((readsize = getline(&line, &len, trace)) != -1){
          lineNumber++;
          word = strtok(line, ",");
          operation = word;
          word = strtok(NULL, ",");
          offset = atol(word);
          word = strtok(NULL, ",");
          request_size = atol(word);

          if (operation == "FOPEN") {

          } else if (operation == "FCLOSE") {

          } else if (operation == "WRITE") {

          } else if (operation == "READ") {
              // prepare trace file information by writing to filename for every read in tracefile
              writebuf = randstring(request_size);
              if (mode == IOLib::POSIX) {
                  if (offset + request_size > max_offset) {
                      max_offset = offset + request_size;
                  }
              }
              else {
                  lh.run(OPType::WRITE, offset, request_size, writebuf);
              }
              free(writebuf);
          } else if (operation == "LSEEK") {

          }

          lineNumber++;
      }
      fclose(trace);

      if (mode == IOLib::POSIX) {
          file = fopen((filename+std::to_string(rank)).c_str(), "w+");
          writebuf = randstring(max_offset);
          fwrite(writebuf, sizeof(char), (size_t) max_offset, file);
          free(writebuf);
      }

      free(line);

      return 0;
  }

private:
    static char *randstring(long length) {
        int n;
        static char charset[] =
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
        char *randomString = NULL;
        if (length) {
            randomString = (char *) malloc(sizeof(char) * (length + 1));
            if (randomString) {
                for (n = 0; n < length; n++) {
                    int key = rand() % (int) (sizeof(charset) - 1);
                    randomString[n] = charset[key];
                }
                randomString[length] = '\0';
            }
        }
        return randomString;
    }
};


#endif //COMMON_REPLAYER_H
