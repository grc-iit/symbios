//
// Created by Jie on 8/28/20.
//

#ifndef SYMBIOS_ERROR_CODES_H
#define SYMBIOS_ERROR_CODES_H

#include <symbios/common/error_definition.h>

/*
 * Errorcodes used in symbios
 */
const ErrorCode SUCCESSFUL = {0, "SUCCESSFUL"};

// io_clients module error code
const ErrorCode OPEN_FILE_FAILED = {1000, "Open file failed"};
const ErrorCode SEEK_FILE_FAILED = {1001, "Seek the position from file failed"};
const ErrorCode READ_DATA_FROM_FILE_FAILED = {1002, "Read data from file failed"};
const ErrorCode WRITE_DATA_TO_FILE_FAILED = {1003, "Write data to file failed"};
const ErrorCode REDIS_SERVER_SIDE_FAILED = {1004, "Redis server maybe failed"};
const ErrorCode READ_REDIS_POSITION_OR_SIZE_FAILED = {1005, "Error read position or data size when reading data from Redis Cluster"};
const ErrorCode READ_REDIS_DATA_FAILED = {1006, "Read data from Redis Cluster failed"};
const ErrorCode WRITE_REDIS_DATA_FAILED = {1007, "Write data to Redis Cluster failed"};
const ErrorCode READ_DATA_FROM_MONGODB_FAILED = {1008, "Read data from Mongodb failed"};
const ErrorCode WRITE_DATA_TO_MONGODB_FAILED = {1009, "Write data to Mongodb failed"};

#endif //SYMBIOS_ERROR_CODES_H
