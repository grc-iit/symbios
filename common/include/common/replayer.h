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

class trace_replayer {
public:
  enum operation_type{
    NONE        =0,
    READ_OPER   =1,
    WRITE_OPER  =2,
    OPEN_OPER   =3,
    CLOSE_OPER  =4,
    SEEK_OPER   =5
  };

  static int replay_trace(std::string traceFile,
                          std::string filename, int repetitions,
                          int rank, int mode) {
      /*Initialization of some stuff*/
      FILE* trace;
      FILE* file = nullptr;
      char* line = NULL;
      int comm_size;
      size_t len=0;
      ssize_t readsize;
      std::string operation;
      long offset = 0;
      long request_size = 0;
      char* word;
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
                  file = tropen((filename+std::to_string(rank)).c_str(), "w+");
              } else if (operation == "FCLOSE") {
                  trclose(file);
              } else if (operation == "WRITE") {
                  void* writebuf = randstring(request_size);
                  trseek(file, (size_t) offset);
                  trwrite(writebuf, file, (size_t) request_size);
                  if(writebuf) free(writebuf);
              } else if (operation == "READ") {
                  char* readbuf = (char*)malloc((size_t) request_size);
                  trseek(file, (size_t) offset);
                  trread(readbuf, file, (size_t) request_size);
                  if(readbuf) free(readbuf);
              } else if (operation == "LSEEK") {
                  trseek(file, (size_t) offset);
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
      if (line) free(line);

      /*if( remove( "/home/anthony/temp/" ) != 0 )
        perror( "Error deleting file" );*/
      return 0;
  }

  static int prepare_data(std::string traceFile) {
      FILE* trace;
      char* line = NULL;
      size_t len=0;
      ssize_t readsize;
      std::string operation;
      long offset = 0;
      long request_size = 0;
      char* word;
      line = (char*) malloc(128);
      int lineNumber=0;

      /* putting down the data, file for PFS and objects for Hyperdex*/

      trace = fopen(traceFile.c_str(), "r");


      while ((readsize = getline(&line, &len, trace)) != -1){
          lineNumber++;
          word = strtok(line, ",");
          operation = word;
          word = strtok(NULL, ",");
          offset = atol(word);
          word = strtok(NULL, ",");
          request_size = atol(word);


          /* if (operation == "FOPEN") { */

          /* } else if (operation == "FCLOSE") { */

          /* } else if (operation == "WRITE") { */

          /* } else if (operation == "READ") { */
              /* char* writebuf = randstring(request_size); */
              /* uint32_t hash = CityHash32(std::to_string(offset+request_size).c_str(), */
              /*                            std::to_string(offset+request_size) */
              /*                            .length()); */
              /* iris::put(std::to_string(hash), writebuf, (size_t) request_size); */
              /* if (writebuf) free(writebuf); */
          /* } else if (operation == "LSEEK") { */

          /* } */

          lineNumber++;
      }
      fclose(trace);
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
  static FILE * tropen(const char *name, const char *mode) {
      return std::fopen(name,mode);
  }
  static int trclose(FILE *fh) {
      std::fclose(fh);
      return 0;
  }
  static long trread(void* ptr, FILE *fh, size_t amount) {
      return std::fread(ptr, sizeof(char), amount, fh);
  }
  static long trwrite(void* ptr, FILE *fh, size_t amount) {
      return std::fwrite(ptr, sizeof(char), amount, fh);
  }
  static int trseek(FILE *fh, size_t amount) {
      std::fseek(fh, (amount), SEEK_SET);
      return 0;
  }
};


#endif //COMMON_REPLAYER_H
