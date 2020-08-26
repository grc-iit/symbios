//
// Created by jaime on 8/26/2020.
//

#ifndef SYMBIOS_DAEMON_H
#define SYMBIOS_DAEMON_H

#include <fstream>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
#include <future>

#include <mpi.h>
#include <thread>

namespace symbios {
    template<class T>
    class Daemon {
    public:
        std::string main_log_file;
        std::promise<void> exitSignal;
        std::thread worker;
        T jobManager;

        [[noreturn]] explicit Daemon(std::string main_log_file = "symbios_server.log");

        virtual ~Daemon();

    private:
        void signalHandler(int sig);

        void logMessage(const std::string &filename, const std::string &message);

        void catchSignals();

    };
}

#endif //SYMBIOS_DAEMON_H
