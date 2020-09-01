//
// Created by mani on 8/29/2020.
//

#include <symbios/common/data_structure.h>

std::ostream &operator<<(std::ostream &os, Data &m) {
    return os   << "id_:" << m.id_ << ","
                << "data_size_:" << m.buffer_.size() << ","
                << "position_:" << m.position_ << ","
                << "storage_index_:" << m.storage_index_ << "}";
}