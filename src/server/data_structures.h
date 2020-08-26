//
// Created by jaime on 8/26/2020.
//

#ifndef SYMBIOS_DATA_STRUCTURES_H
#define SYMBIOS_DATA_STRUCTURES_H
#include <basket.h>

typedef struct Request{
    CharStruct path;
    size_t offset;
    size_t event_size;
    bool operator==(const Request &o) const {
        return o.path == path && o.offset==offset && o.event_size==event_size;
    }
    Request():path(),offset(0),event_size(0){}
    Request(const Request &other) : path(other.path),
                                      offset(other.offset),
                                      event_size(other.event_size){} /* copy constructor*/
    Request(Request &&other) :path(other.path),
                                offset(other.offset),
                                event_size(other.event_size){} /* move constructor*/
    Request &operator=(const Request &other) {
        path = other.path;
        offset = other.offset;
        event_size = other.event_size;
        return *this;
    }
} Location;

#endif //SYMBIOS_DATA_STRUCTURES_H
