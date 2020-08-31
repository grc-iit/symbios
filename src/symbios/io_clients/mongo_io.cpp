//
// Created by Jie on 8/25/20.
//

#include <symbios/io_clients/mongo_io.h>
#include <basket/common/singleton.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/error_codes.h>
#include <cstring>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>


void MongoIOClient::Read(Data &source, Data &destination) {
    ReadInternal(source,destination);
}

void MongoIOClient::Write(Data &source, Data &destination) {

    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(mongo_solution->collection_.c_str());
    bool exists=false;
    Data read_source;
    std::string_view id;
    try{
        read_source.id_=destination.id_;
        id = ReadInternal(read_source,read_source);
        exists=true;
    }catch(const std::exception& e){
        exists=false;
    }
    if(exists){
        file.delete_many(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("key", std::string(destination.id_.c_str()))));
        /**
         * TODO check if deleted.
         */
    }
    auto document = bsoncxx::builder::basic::document{};
    using bsoncxx::builder::basic::kvp;
    std::string data((const char*)source.buffer_,source.data_size_);
    std::string keyName(destination.id_.c_str());
    //std::cout<<"KeyName :"<<keyName<<"\n";
    document.append(kvp("key",keyName),kvp("value",data));

    //adding the created key-value pair to the collection
    bsoncxx::document::view putView =  document.view();//get the view
    //retrieve the unique objectID from map
    auto add = file.insert_one(putView);//insert it to collection
    if (!add) {
        std::cout << "Unacknowledged write. No id available." << "\n";
    }
    if (add->inserted_id().type() == bsoncxx::type::k_oid) {
        bsoncxx::oid id = add->inserted_id().get_oid().value;
    } else std::cout << "Inserted id was not an OID type" << "\n";

}

std::string_view MongoIOClient::ReadInternal(Data &source, Data &destination) {

    mongocxx::collection file = client[mongo_solution->database_.c_str()].collection(mongo_solution->collection_.c_str());
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
            file.find_one(bsoncxx::builder::stream::document{} << "key" << std::string(source.id_.c_str()) << bsoncxx::builder::stream::finalize);
    if(maybe_result) {
        std::string_view value = maybe_result.value().view()["value"].get_value().get_utf8().value.to_string();
        destination.buffer_=malloc(value.size()+1);
        memcpy(destination.buffer_,value.data(),value.size()+1);
        destination.data_size_=value.size()+1;
        std::cout << bsoncxx::to_json(*maybe_result) << "\n";
        std::string_view id = maybe_result.value().view()["_id"].get_oid().value.to_string();
        return id;
    }else{
        throw ErrorException(READ_REDIS_DATA_FAILED);
    }
}
