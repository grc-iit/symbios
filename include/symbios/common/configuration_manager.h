#ifndef SYMBIOS_CONFIGURATION_MANAGER_H
#define SYMBIOS_CONFIGURATION_MANAGER_H

#include <basket/common/singleton.h>
#include <basket/common/typedefs.h>
#include <symbios/common/enumerations.h>
#include <symbios/common/data_structure.h>
#include <basket/common/data_structures.h>
#include <basket/common/macros.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h>
#include <regex>
#include <boost/filesystem/operations.hpp>
#include <symbios/common/error_codes.h>

#define SYMBIOS_CONF basket::Singleton<symbios::ConfigurationManager>::GetInstance()
namespace symbios {
    class ConfigurationManager {

    private:
        static std::string replaceEnvVariable(std::string temp_variable){

            std::string pattern("(\\$\\{.*?\\})");
            auto regexp = regex(pattern);
            smatch m;
            regex_search(temp_variable, m, regexp);
            auto variables=std::set<std::string>();
            for(unsigned i=0; i<m.size(); ++i) {
                auto extracted_val = m[i].str();
                //if(extracted_val.find("{") == std::string::npos) continue;
                auto val = m[i].str().substr(2, m[i].str().size() - 3);
                variables.insert(val);
            }
            for(auto variable:variables){
                auto unrolled = std::getenv(variable.c_str());
                if(unrolled==NULL) throw ErrorException(UNDEFINED_ENV_VARIABLE,variable.c_str());
                temp_variable = regex_replace(temp_variable, regexp, unrolled);
            }
            return temp_variable;
        }
        template <typename T>
        void config(T &doc, const char *member, uint16_t &variable) {
            if(!doc.HasMember(member)) return;
            assert(doc[member].IsInt());
            variable = atoi(replaceEnvVariable(std::to_string(doc[member].GetInt())).c_str());
        }
        template <typename T>
        void config(T &doc, const char *member, really_long &variable) {
            if(!doc.HasMember(member)) return;
            assert(doc[member].IsUint64());
            variable = atoll(replaceEnvVariable(std::to_string(doc[member].GetUint64())).c_str());
        }

        template <typename T>
        void config(T &doc, const char *member, std::string &variable) {
            if(!doc.HasMember(member)) return;
            assert(doc[member].IsString());
            std::string temp_variable = doc[member].GetString();
            variable = replaceEnvVariable(temp_variable);
        }
        template <typename T>
        void config(T &doc, const char *member, CharStruct &variable) {
            if(!doc.HasMember(member)) return;
            assert(doc[member].IsString());
            std::string temp_variable = doc[member].GetString();
            variable = CharStruct(replaceEnvVariable(temp_variable));
        }

        void config(rapidjson::Document &doc, const char *member,
                    std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>>&variable) {
            variable = std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>>();
            if(!doc.HasMember(member)) return;
            rapidjson::Value& results = doc[member];
            assert(results.IsArray());
            for (rapidjson::SizeType i = 0; i < results.Size(); i++) {
                std::shared_ptr<StorageSolution> ss;

                if(results[i]["TYPE"] == "FILE_IO"){
                    std::string mount;
                    config(results[i],"MOUNT",mount);
                    ss = static_cast<const shared_ptr<StorageSolution>>(new FileStorageSolution(mount));
                }
                else if(results[i]["TYPE"] == "REDIS_IO"){
                    std::string ip;
                    std::string port;
                    config(results[i],"IP",ip);
                    config(results[i],"PORT",port);
                    ss = static_cast<const shared_ptr<StorageSolution>>(new RedisSS (ip,port));
                }
                else if(results[i]["TYPE"] == "MONGO_IO"){
                    std::string ip,database,collection;
                    config(results[i],"IP",ip);
                    config(results[i],"DATABASE",database);
                    config(results[i],"COLLECTION",collection);
                    ss = static_cast<const shared_ptr<StorageSolution>>(new MongoSS(ip,database,collection));
                }
                else{
                    std::cerr << "Incorrect configuration on Storage Solutions" << std::endl;
                }

                variable.insert(std::pair<uint16_t, std::shared_ptr<StorageSolution>>(i, ss));
            }
        }

        void config(rapidjson::Document &doc, const char *member, DataDistributionPolicy &variable) {
            if(!doc.HasMember(member)) return;
            assert(doc[member].IsString());
            std::string distr_string = doc[member].GetString();
            if(distr_string == "RANDOM_POLICY") variable=RANDOM_POLICY;
            else if(distr_string == "ROUND_ROBIN_POLICY") variable=ROUND_ROBIN_POLICY;
            else if(distr_string == "HEURISTICS_POLICY") variable=HEURISTICS_POLICY;
            else if(distr_string == "DYNAMIC_PROGRAMMING_POLICY") variable=DYNAMIC_PROGRAMMING_POLICY;
            else std::cerr << "Incorrect configuration on Data Distribution Policy" << std::endl;
        }

        int CountServers(CharStruct server_list_path) {
            fstream file;
            int total = 0;
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
                        for (int i = 0; i < count; i++) {
                            total++;
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
        uint16_t SERVER_COUNT;
        uint16_t RANDOM_SEED;
        std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>> STORAGE_SOLUTIONS;
        DataDistributionPolicy DATA_DISTRIBUTION_POLICY;

        ConfigurationManager() : SERVER_LISTS("/home/user/symbios/conf/server_lists/single_node_symbios_server"),
                                 CLIENT_LISTS("/home/user/symbios/conf/server_lists/single_node_symbios_client"),
                                 SYMBIOS_PORT(8000),
                                 SERVER_RPC_THREADS(4),
                                 SERVER_DIR("/dev/shm/hari/single_node_symbios_server"),
                                 CONFIGURATION_FILE("/home/user/symbios/conf/base_symbios.conf"),
                                 SERVER_COUNT(1),
                                 RANDOM_SEED(100),
                                 STORAGE_SOLUTIONS(),
                                 DATA_DISTRIBUTION_POLICY(DataDistributionPolicy::RANDOM_POLICY){
            STORAGE_SOLUTIONS.insert({0, std::make_shared<FileStorageSolution>("./") });
            STORAGE_SOLUTIONS.insert({1, std::make_shared<RedisSS>("127.0.0.1", "6379") });
            STORAGE_SOLUTIONS.insert({2, std::make_shared<MongoSS>("mongodb://localhost:27017", "mydb", "test") });

        }

        void LoadConfiguration() {
            using namespace rapidjson;

            FILE *outfile = fopen(CONFIGURATION_FILE.c_str(), "r");
            if (outfile == NULL) {
                printf("Symbios configuration not found %s \n",CONFIGURATION_FILE.c_str());
                exit(EXIT_FAILURE);
            }
            char buf[65536];
            FileReadStream instream(outfile, buf, sizeof(buf));
            Document doc;
            doc.ParseStream<kParseStopWhenDoneFlag>(instream);
            if (!doc.IsObject()) {
                std::cout << "Symbios - Configuration JSON is invalid" << std::endl;
                fclose(outfile);
                exit(EXIT_FAILURE);
            }
            config(doc, "SERVER_LISTS", SERVER_LISTS);
            config(doc, "CLIENT_LISTS", CLIENT_LISTS);
            config(doc, "SYMBIOS_PORT", SYMBIOS_PORT);
            config(doc, "SERVER_RPC_THREADS", SERVER_RPC_THREADS);
            config(doc, "SERVER_DIR", SERVER_DIR);
            config(doc, "RANDOM_SEED", RANDOM_SEED);
            config(doc, "STORAGE_SOLUTIONS", STORAGE_SOLUTIONS);
            config(doc, "DATA_DISTRIBUTION_POLICY", DATA_DISTRIBUTION_POLICY);
            boost::filesystem::create_directories(SERVER_DIR.c_str());

            fclose(outfile);
        }

        void ConfigureSymbiosClient() {
            LoadConfiguration();
            BASKET_CONF->ConfigureDefaultClient(SERVER_LISTS.c_str());
            BASKET_CONF->RPC_PORT = SYMBIOS_PORT;
        }

        void ConfigureSymbiosServer() {
            LoadConfiguration();
            BASKET_CONF->RPC_THREADS = SERVER_RPC_THREADS;
            BASKET_CONF->MEMORY_ALLOCATED = 1024ULL * 1024ULL * 1ULL;
            BASKET_CONF->ConfigureDefaultServer(SERVER_LISTS.c_str());
            SERVER_COUNT = BASKET_CONF->NUM_SERVERS;
            BASKET_CONF->RPC_PORT = SYMBIOS_PORT;
        }
    };
}
#endif //SYMBIOS_CONFIGURATION_MANAGER_H