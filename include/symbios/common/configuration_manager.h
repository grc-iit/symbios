#ifndef SYMBIOS_CONFIGURATION_MANAGER_H
#define SYMBIOS_CONFIGURATION_MANAGER_H

#include <basket/common/singleton.h>
#include <basket/common/typedefs.h>
#include <basket/common/data_structures.h>
#include <basket/common/macros.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h>

#define SYMBIOS_CONF basket::Singleton<symbios::ConfigurationManager>::GetInstance()
namespace symbios{
        class ConfigurationManager{
        
            private:
                void config(rapidjson::Document &doc, const char* member, uint16_t &variable){
                        assert(doc.HasMember(member));
                        assert(doc[member].IsInt());
                        variable = doc[member].GetInt();
                    }
        
                void config(rapidjson::Document &doc, const char* member, really_long &variable){
                        assert(doc.HasMember(member));
                        assert(doc[member].IsUint64());
                        variable = doc[member].GetUint64();
                    }
        
                void config(rapidjson::Document &doc, const char* member, CharStruct &variable){
                        assert(doc.HasMember(member));
                        assert(doc[member].IsString());
                        variable = doc[member].GetString();
                    }
                int CountServers(CharStruct server_list_path){
                        fstream file;
                        int total=0;
                        file.open(server_list_path.c_str(), ios::in);
                        if (file.is_open()) {
                                std::string file_line;
                                std::string server_node_name;
                                int count;
                                while (getline(file, file_line)) {
                                        if (!file_line.empty()) {
                                                int split_loc = file_line.find(':');  // split to node and net
                                                if (split_loc != std::string::npos) {
                                                        server_node_name = file_line.substr(0, split_loc);
                                                        count = atoi(file_line.substr(split_loc, std::string::npos).c_str());
                                                    } else {
                                                        // no special network
                                                                server_node_name = file_line;
                                                        count = 1;
                                                    }
                                                // server list is list of network interfaces
                                                        for(int i=0;i<count;i){
                                                        total;
                                                    }
                                            }
                                    }
                            } else {
                                printf("Error: Can't open server list file %s\n", server_list_path.c_str());
                                exit(EXIT_FAILURE);
                            }
                        file.close();
                        return total;
                    }
        
            public:
                CharStruct SERVER_LISTS, CLIENT_LISTS;
                uint16_t SYMBIOS_PORT;
                uint16_t SERVER_RPC_THREADS;
                CharStruct SERVER_DIR;
                CharStruct CONFIGURATION_FILE;
                int SERVER_COUNT;
//            std::string redis_cluster_host = "127.0.0.1";
//            int redis_cluster_port = 6379;
//            std::string mongodb_cluster_url = "mongodb://localhost:27017";
//            std::string mongodb_cluster_database = "mydb";
//            std::string mongodb_cluster_collection = "test";

                ConfigurationManager(): SERVER_LISTS("/home/hdevarajan/projects/chronolog/server_lists/journal"),
                                        CLIENT_LISTS("/home/hdevarajan/projects/chronolog/server_lists/chronoplayer_client"),
                                        SYMBIOS_PORT(8000),
                                        SERVER_RPC_THREADS(4),
                                        SERVER_DIR("/dev/shm/hari/journal"),
                                        CONFIGURATION_FILE("/home/hdevarajan/projects/chronolog/conf/config/chronolog.json"),
                                        SERVER_COUNT(1){}
        
                void LoadConfiguration(){
                        using namespace rapidjson;
            
                                FILE *outfile = fopen(CONFIGURATION_FILE.c_str(), "r");
                        if(outfile == NULL){
                                std::cout << "HLog configuration not found" << std::endl;
                                exit(EXIT_FAILURE);
                            }
                        char buf[65536];
                        FileReadStream instream(outfile, buf, sizeof(buf));
                        Document doc;
                        doc.ParseStream<kParseStopWhenDoneFlag>(instream);
                        if(!doc.IsObject()) {
                                std::cout << "HLog - Canfiguration JSON is invalid" << std::endl;
                                fclose(outfile);
                                exit(EXIT_FAILURE);
                            }
                        config(doc, "SERVER_LISTS", SERVER_LISTS);
                        config(doc, "CLIENT_LISTS", CLIENT_LISTS);
                        config(doc, "SYMBIOS_PORT", SYMBIOS_PORT);
                        config(doc, "SERVER_RPC_THREADS", SERVER_RPC_THREADS);
                        config(doc, "SERVER_DIR", SERVER_DIR);
                        config(doc, "CONFIGURATION_FILE", CONFIGURATION_FILE);
                        SERVER_COUNT = CountServers(SERVER_LISTS);
                    }
                void ConfigureSymbiosClient(){
                        LoadConfiguration();
                        BASKET_CONF->ConfigureDefaultClient(SERVER_LISTS.c_str());
                        BASKET_CONF->RPC_PORT=SYMBIOS_PORT;
                    }
                void ConfigureSymbiosServer(){
                        LoadConfiguration();
                        BASKET_CONF->RPC_THREADS=SERVER_RPC_THREADS;
                        BASKET_CONF->MEMORY_ALLOCATED=1024ULL * 1024ULL * 1024ULL * 1ULL;
                        BASKET_CONF->ConfigureDefaultServer(SERVER_LISTS.c_str());
                        BASKET_CONF->RPC_PORT=SYMBIOS_PORT;
                    }
            };
    }
#endif //SYMBIOS_CONFIGURATION_MANAGER_H
