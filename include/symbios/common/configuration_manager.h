//
// Created by Jie on 8/27/20.
//

#ifndef SYMBIOS_CONFIGURATION_MANAGER_H
#define SYMBIOS_CONFIGURATION_MANAGER_H

class ConfigurationManager {
public:
    /*
     * Constructor
     */
    ConfigurationManager(){

    }

    /*
     * Methods
     */
    void LoadConfiguration(){

    }

public:
    /* Variables*/
    std::string redis_cluster_host = "127.0.0.1";
    int redis_cluster_port = 6379;
    std::string mongodb_cluster_url = "mongodb://localhost:27017";
    std::string mongodb_cluster_database = "mydb";
    std::string mongodb_cluster_collection = "test";
};

#endif //SYMBIOS_CONFIGURATION_MANAGER_H

