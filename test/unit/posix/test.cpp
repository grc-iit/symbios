//
// Created by jaime on 9/2/2020.
//

#include <cstdio>
#include <symbios/common/configuration_manager.h>
#include <symbios/client/posix.h>

void basic_tests(){
    size_t result;
    char * buffer = (char*) malloc (sizeof(char)*100);

    ////Standard Open
    auto client_file = fopen(SYMBIOS_CONF->CLIENT_LISTS.c_str(), "r");
    assert(client_file!=nullptr);
    //Test we can write to the server_dir and that files can be created.
    auto server_dir = fopen((SYMBIOS_CONF->SERVER_DIR + "/test").c_str(), "w+");
    assert(server_dir!=nullptr);

    ////Standard Read
    result = fread (buffer,1,4,client_file);
    assert(result == 4);
    assert(strncmp(buffer, "loca", 4));

    ////Standard fseek
    result = fseek(server_dir, 0, SEEK_SET);
    assert(result==0);
    assert(ftell(server_dir)==0);

    ////Standard Write
    result = fwrite(buffer, 1,4, server_dir);
    assert(result == 4);

    ////Middle fseek
    result = fseek(server_dir, 2, SEEK_SET);
    assert(result==0);
    assert(ftell(server_dir)==2);
    result = fread (buffer,1,2,server_dir);
    assert(result == 2);
    assert(strncmp(buffer, "ca", 2) == 0);

    ////Standard close
    fclose(client_file);
    fclose(server_dir);

    ////Open Append
    auto new_server_dir = fopen((SYMBIOS_CONF->SERVER_DIR + "/test").c_str(), "a+");
    result = fwrite(buffer, 1,2, new_server_dir);
    assert(result == 2);
    result = fseek(new_server_dir, 0, SEEK_SET);
    assert(result==0);
    assert(ftell(new_server_dir)==0);
    result = fread (buffer,1,6,new_server_dir);
    assert(result == 4);
    assert(strncmp(buffer, "locaca", 4));
    fclose(new_server_dir);
}

void intercept_test(char* mount_point){
    auto posix = basket::Singleton<symbios::Posix>::GetInstance();
    size_t result;
    char* buffer = (char*) std::string("testingInterception").c_str();

    ////Intercept Open
    auto test_file = fopen(mount_point, "w+");
    assert(test_file!=nullptr);
    auto stat = posix->GetStat(test_file);
    assert(stat.first);
    assert(strncmp(mount_point, stat.second.filename_.c_str(), strlen(mount_point)) == 0);
    assert(stat.second.file_size_ == 0);
    assert(stat.second.file_pointer_ == 0);

    ////Standard Write
    result = fwrite(buffer, 1,19, test_file);
    assert(result == 19);
    stat = posix->GetStat(test_file);
    assert(stat.first);
    assert(stat.second.file_size_ == 19);
    assert(stat.second.file_pointer_ == 19);

    ////Standard fseek
    result = fseek(test_file, 0, SEEK_SET);
    assert(result == 0);
    stat = posix->GetStat(test_file);
    assert(stat.first);
    assert(stat.second.file_size_ == 19);
    assert(stat.second.file_pointer_ == 0);

    ////Standard Read
    result = fread (buffer,1,19,test_file);
    assert(result == 19);
    assert(strncmp(buffer, "testingInterception", 19));
    stat = posix->GetStat(test_file);
    assert(stat.first);
    assert(stat.second.file_size_ == 19);
    assert(stat.second.file_pointer_ == 19);

    ////Middle fseek
    result = fseek(test_file, 5, SEEK_SET);
    assert(result == 0);
    stat = posix->GetStat(test_file);
    assert(stat.first);
    assert(stat.second.file_size_ == 19);
    assert(stat.second.file_pointer_ == 5);

    result = fread (buffer,1,5,test_file);
    assert(result == 0);
    assert(strncmp(buffer, "ngInt", 2));
    stat = posix->GetStat(test_file);
    assert(stat.first);
    assert(stat.second.file_size_ == 19);
    assert(stat.second.file_pointer_ == 10);

    ////Standard close
    fclose(test_file);

    ////Open Append
    buffer = (char*) std::string("testingInterception").c_str();
    auto new_server_dir = fopen((SYMBIOS_CONF->SERVER_DIR + "/test").c_str(), "a+");
    stat = posix->GetStat(new_server_dir);
    assert(stat.first);
    assert(stat.second.file_size_ == 19);
    assert(stat.second.file_pointer_ == 19);

    result = fwrite(buffer, 1,7, new_server_dir);
    assert(result == 7);
    stat = posix->GetStat(new_server_dir);
    assert(stat.first);
    assert(stat.second.file_size_ == 26);
    assert(stat.second.file_pointer_ == 26);

    result = fseek(new_server_dir, 19, SEEK_SET);
    assert(result==0);
    stat = posix->GetStat(new_server_dir);
    assert(stat.first);
    assert(stat.second.file_size_ == 26);
    assert(stat.second.file_pointer_ == 19);

    result = fread (buffer,1,7,new_server_dir);
    assert(result == 7);
    assert(strncmp(buffer, "testing", 7));
    stat = posix->GetStat(new_server_dir);
    assert(stat.first);
    assert(stat.second.file_size_ == 26);
    assert(stat.second.file_pointer_ == 26);

    fclose(new_server_dir);
}

int main(int argc, char * argv[]){
    char* mount_point;
    if(argc > 1){
        mount_point = argv[1];
    }
    basic_tests();
    intercept_test(mount_point);
    return 0;
}