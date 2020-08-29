//
// Created by Jie on 8/27/20.
//

#ifndef SYMBIOS_ENUMERATIONS_H
#define SYMBIOS_ENUMERATIONS_H

/*
 *  Enumerate IO Client Error Status, just for test
 */
typedef enum IOClientErrorStatus{
    IO_CLIENT_SUCCESS = 0,
    IO_CLIENT_FAILED = 1
} IOClientErrorStatus;

/**
 * IOClientType defines various types of IO Clients supported by Symbios
 */
typedef enum IOClientType{
    FILE_IO = 0,
    REDIS_IO = 1,
    MONGO_IO = 2
} IOClientType;

/**
 * DataPlacementPolicy defines various of DataPlacement policies supported by Symbios
 */
typedef enum DataDistributionPolicy{
    RANDOM_POLICY = 0,
    ROUND_ROBIN_POLICY = 1,
    HEURISTICS_POLICY = 2,
    DYNAMIC_PROGRAMMING_POLICY = 3
} DataDistributionPolicy;

#endif //SYMBIOS_ENUMERATIONS_H

