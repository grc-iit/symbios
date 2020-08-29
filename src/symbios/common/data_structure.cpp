//
// Created by mani on 8/29/2020.
//

#include <symbios/common/data_structure.h>

std::ostream &operator<<(std::ostream &os, Data const &data){
    return os   << "{TYPE:Data," << "id_:" << data.id_<<"}";
}