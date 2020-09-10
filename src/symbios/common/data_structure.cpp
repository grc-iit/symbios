//
// Created by mani on 8/29/2020.
//

#include <symbios/common/data_structure.h>
/*
 * Implement << operator to print Data structure
 */
std::ostream &operator<<(std::ostream &os, Data &m) {
    return os << "{id_:" << m.id_ << ","
              << "data_size_:" << m.data_size_ << ","
              << "position_:" << m.position_ << ","
              << "storage_index_:" << m.storage_index_ << "}";
}

/*
 * Implement << operator to print Metadata structure
 */
std::ostream &operator<<(std::ostream &os, Metadata &m) {
    os << "{storage_index_:" << m.storage_index_ << ","
       << "is_link_:" << m.is_link_ << ","
       << "links_:{";
    int i = 0;
    for (auto link:m.links_) {
        os << "{ first:" << link.first << ", "
           << "second:" << link.first << "}";
        if (i < m.links_.size() - 1) {
            os << ",";
        }
        os << "}";
    }
    return os;
}

/*
 * Implement << operator to print DataDistribution structure
 */
std::ostream &operator<<(std::ostream &os, DataDistribution &m) {
    return os << "{source_data_:" << m.source_data_ << ","
              << "destination_data_:" << m.destination_data_ << ","
              << "storage_index_:" << m.storage_index_ << "}";
}