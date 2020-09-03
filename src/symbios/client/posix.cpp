////
//// Created by jaime on 8/28/2020.
////
#include <symbios/client/posix.h>
#include <basket/common/macros.h>
#include <symbios/client/client.h>
#include <symbios/common/configuration_manager.h>
#include <symbios/common/enumerations.h>


bool symbios::Posix::UpdateOnOpen(FILE *fh, PosixStat &stat) {
    auto map_locator = fileDescriptorMap_.find(fh);
    if (map_locator != fileDescriptorMap_.end())
        fileDescriptorMap_.erase(map_locator);
    auto client = basket::Singleton<symbios::Client>::GetInstance();
    Data source;
    source.id_=stat.filename_;
    source.storage_index_=stat.storage_index;
    if(stat.mode_== "w" || stat.mode_== "w+" || stat.mode_== "r" || stat.mode_== "r+")
        stat.file_pointer_=0;

    stat.file_size_ = client->Size(source);
    if(stat.mode_== "w+"){
        stat.file_size_=0;
        bool success = client->Delete(source);
    }
    if(stat.mode_== "a+" || stat.mode_== "a"){
        stat.file_pointer_=stat.file_size_;
    }
    return fileDescriptorMap_.insert_or_assign(fh,stat).second;
}

bool symbios::Posix::isFileDescriptorTracked(FILE *fh) {
    auto map_locator = fileDescriptorMap_.find(fh);
    if (map_locator != fileDescriptorMap_.end()) return true;
    else return false;
}

bool symbios::Posix::UpdateOnClose(FILE *fh) {
    auto map_locator = fileDescriptorMap_.find(fh);
    if (map_locator != fileDescriptorMap_.end()) {
        fileDescriptorMap_.erase(map_locator);
        return true;
    }
    return false;
}

std::pair<bool, PosixStat> symbios::Posix::GetStat(FILE* fh) {
    auto iter = fileDescriptorMap_.find(fh);
    if (iter != fileDescriptorMap_.end()) return {true, iter->second};
    else return {false, PosixStat()};
}

int symbios::Posix::UpdateOnSeek(FILE* fh,long int offset, int origin) {
    auto iter = fileDescriptorMap_.find(fh);
    if (iter != fileDescriptorMap_.end()){
        if(iter->second.mode_== "a+" || iter->second.mode_== "a"){
            return iter->second.file_size_;
        }else{
            if(origin == SEEK_CUR){
                iter->second.file_pointer_+=offset;
            }else if(origin == SEEK_SET){
                iter->second.file_pointer_=offset;
            }else if(origin == SEEK_END){
                iter->second.file_pointer_=iter->second.file_size_ - offset;
            }
        }
        return 0;
    }
    else return -1;
}

bool symbios::Posix::UpdateStat(FILE *fh, PosixStat &stat) {
    return fileDescriptorMap_.insert_or_assign(fh,stat).second;
}


FILE *fopen(const char *filename, const char *mode) {
    printf(filename);
    MAP_OR_FAIL(fopen);
    FILE *fh;
    if(strncmp(filename, SYMBIOS_CONF->CONFIGURATION_FILE.c_str(), SYMBIOS_CONF->CONFIGURATION_FILE.size()) == 0)
        return __real_fopen(filename, mode);
    auto posix = basket::Singleton<symbios::Posix>::GetInstance();
    for (auto solutions:SYMBIOS_CONF->STORAGE_SOLUTIONS) {
        if (solutions.second->io_client_type_ == IOClientType::FILE_IO) {
            if (strncmp(filename, solutions.second->end_point_.c_str(), solutions.second->end_point_.size()) == 0) {
                //intercept
                fh = fmemopen(NULL, FILE_BUFFER_CAPACITY, mode);
                PosixStat stat;
                stat.filename_=filename;
                stat.mode_=mode;
                stat.storage_index=solutions.first;
                posix->UpdateOnOpen(fh, stat);
                return fh;
            }
        }
    }
    return __real_fopen(filename, mode);;
}

int fclose(FILE *stream) {
    auto posix = basket::Singleton<symbios::Posix>::GetInstance();
    if (posix->isFileDescriptorTracked(stream)) {
        posix->UpdateOnClose(stream);
        return 0;
    }
    MAP_OR_FAIL(fclose);
    return __real_fclose(stream);
}

int fseek(FILE *stream, long int offset, int origin) {
    auto posix = basket::Singleton<symbios::Posix>::GetInstance();

    if (posix->isFileDescriptorTracked(stream)) {
        return posix->UpdateOnSeek(stream, offset, origin);
    }
    MAP_OR_FAIL(fseek);
    return __real_fseek(stream, offset, origin);
}

size_t fread(void *ptr, std::size_t size, std::size_t count, FILE *stream) {
    auto posix = basket::Singleton<symbios::Posix>::GetInstance();
    if (posix->isFileDescriptorTracked(stream)) {
        auto client = basket::Singleton<symbios::Client>::GetInstance();
        auto data = Data();
        //get the filename
        auto data_size = size * count;
        auto stat = posix->GetStat(stream);
        if (stat.first){
            if(stat.second.file_size_ < stat.second.file_pointer_ +  data_size) return -1;
            data.id_ = stat.second.filename_;
            data.position_ = stat.second.file_pointer_;
        }
        else return stat.first;
        data.data_size_=data_size;
        data.buffer_= static_cast<char *>(malloc(data_size));
        client->LocateRequest(data);
        if (data.data_size_ != data_size) return data.data_size_;
        memcpy(ptr, data.buffer_, data.data_size_);
        return data.data_size_;
    }
    MAP_OR_FAIL(fread);
    return __real_fread(ptr, size, count, stream);
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
    auto posix = basket::Singleton<symbios::Posix>::GetInstance();
    if (posix->isFileDescriptorTracked(stream)) {
        auto client = basket::Singleton<symbios::Client>::GetInstance();
        auto data = Data();
        //get the filename
        auto data_size = size * count;
        auto stat = posix->GetStat(stream);
        if (stat.first){
            if(stat.second.file_size_ < stat.second.file_pointer_ +  data_size) return -1;
            data.id_ = stat.second.filename_;
            data.position_ = stat.second.file_pointer_;
        }
        else return stat.first;
        //set the other variables
        data.data_size_=data_size;
        data.buffer_= static_cast<char *>(malloc(data_size));
        memcpy(data.buffer_, ptr, data_size);
        client->StoreRequest(data);
        if(data_size + stat.second.file_pointer_ > stat.second.file_size_){
            stat.second.file_size_ = data_size + stat.second.file_pointer_;
            posix->UpdateStat(stream,stat.second);
        }
        return data_size;
    }
    MAP_OR_FAIL(fwrite);
    return __real_fwrite(ptr, size, count, stream);
}
