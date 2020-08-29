//
// Created by mani on 8/29/2020.
//

#include <symbios/common/data_structure.h>

std::ostream &operator<<(std::ostream &os, CharStruct const &m){     return os   << "{TYPE:CharStruct," << "value:" << m.c_str()<<"}"; }