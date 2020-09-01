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

#define SYMBIOS_CONF basket::Singleton<symbios::ConfigurationManager>::GetInstance()
namespace symbios {
    class ConfigurationManager {

    private:
        static std::string replaceEnvVariable(std::string temp_variable){
            regex regexp("{.+}");
            smatch m;
            if (regex_match (temp_variable, regexp)) {
                regex_search(temp_variable, m, regexp);
                for(unsigned i=0; i<m.size(); ++i) {
                    auto unrolled = std::getenv(m[i].str().substr(1, m[i].str().size() - 2).c_str());
                    temp_variable.replace(m.position(i)-1, m.length(i), unrolled);
                }
            }
            return temp_variable;
        }

        void config(rapidjson::Document &doc, const char *member, uint16_t &variable) {
            assert(doc.HasMember(member));
            assert(doc[member].IsInt());
            variable = doc[member].GetInt();
        }

        void config(rapidjson::Document &doc, const char *member, really_long &variable) {
            assert(doc.HasMember(member));
            assert(doc[member].IsUint64());
            variable = doc[member].GetUint64();
        }

        void config(rapidjson::Document &doc, const char *member, CharStruct &variable) {
            assert(doc.HasMember(member));
            assert(doc[member].IsString());
            std::string temp_variable = doc[member].GetString();
            std::cout << "Input string from conf: " << temp_variable << std::endl;
            variable = CharStruct(replaceEnvVariable(temp_variable));
            std::cout << "Output string from conf: " << variable << std::endl;
        }

        void config(rapidjson::Document &doc, const char *member,
                    std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>>&variable) {
            assert(doc.HasMember(member));
            rapidjson::Value& results = doc[member];
            assert(results.IsArray());
            for (rapidjson::SizeType i = 0; i < results.Size(); i++) {
                std::shared_ptr<StorageSolution> ss;

                if(results[i].GetString() == "FILE_IO"){
                    auto mount = replaceEnvVariable(results[i]["MOUNT"].GetString());
                    ss = static_cast<const shared_ptr<StorageSolution>>(new FileStorageSolution(mount));
                }
                else if(results[i].GetString() == "REDIS_IO"){
                    ss = static_cast<const shared_ptr<StorageSolution>>(new RedisSS (
                            results[i]["IP"].GetString(),
                            results[i]["PORT"].GetInt()));
                }
                else if(results[i].GetString() == "MONGO_IO"){
                    ss = static_cast<const shared_ptr<StorageSolution>>(new MongoSS(
                            results[i]["IP"].GetString(),
                             results[i]["DATABASE"].GetString(),
                            results[i]["COLLECTION"].GetString()));
                }
                else{
                    std::cerr << "Incorrect configuration on Storage Solutions" << std::endl;
                }

                variable.insert(std::pair<uint16_t, std::shared_ptr<StorageSolution>>(i, ss));
            }
        }

        void config(rapidjson::Document &doc, const char *member, DataDistributionPolicy &variable) {
            assert(doc.HasMember(member));
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
        CharStruct POSIX_MOUNT_POINT;
        uint16_t SERVER_COUNT;
        uint16_t RANDOM_SEED;
        std::unordered_map<uint16_t, std::shared_ptr<StorageSolution>> STORAGE_SOLUTIONS;
        DataDistributionPolicy DATA_DISTRIBUTION_POLICY;

        ConfigurationManager() : SERVER_LISTS("/tmp/tmp.BUKlhPiLxF/conf/server_lists/single_node_symbios_server"),
                                 CLIENT_LISTS("/tmp/tmp.BUKlhPiLxF/conf/server_lists/single_node_symbios_client"),
                                 SYMBIOS_PORT(8000),
                                 SERVER_RPC_THREADS(4),
                                 SERVER_DIR("/dev/shm/hari/single_node_symbios_server"),
                                 CONFIGURATION_FILE("/tmp/tmp.BUKlhPiLxF/conf/base_symbios.conf"),
                                 POSIX_MOUNT_POINT("/tmp/tmp.BUKlhPiLxF/conf/base_symbios.conf"),
                                 SERVER_COUNT(1),
                                 RANDOM_SEED(100),
                                 STORAGE_SOLUTIONS(),
                                 DATA_DISTRIBUTION_POLICY(DataDistributionPolicy::RANDOM_POLICY){
            STORAGE_SOLUTIONS.insert({0, std::make_shared<FileStorageSolution>("./") });
            STORAGE_SOLUTIONS.insert({1, std::make_shared<RedisSS>("127.0.0.1", 6379) });
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
            config(doc, "POSIX_MOUNT_POINT", POSIX_MOUNT_POINT);
            config(doc, "RANDOM_SEED", RANDOM_SEED);
            config(doc, "STORAGE_SOLUTIONS", STORAGE_SOLUTIONS);
            config(doc, "DATA_DISTRIBUTION_POLICY", DATA_DISTRIBUTION_POLICY);
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