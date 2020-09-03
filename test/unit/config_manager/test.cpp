#include <symbios/common/configuration_manager.h>

//
// Created by mani on 9/1/2020.
//
int main(int argc, char* argv[]){
    SYMBIOS_CONF->CONFIGURATION_FILE = argv[1];
    SYMBIOS_CONF->LoadConfiguration();
    auto conf = SYMBIOS_CONF;
    return 0;
}