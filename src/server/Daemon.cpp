//
// Created by jaime on 4/14/20.
//

#include <basket/common/singleton.h>
#include "Daemon.h"

template<class T>
[[noreturn]] symbios::Daemon<T>::Daemon(std::string main_log_file): main_log_file(main_log_file), jobManager() {
    std::future<void> futureObj = exitSignal.get_future();

    worker = std::thread(&T(), this, std::move(futureObj));

//    basket::Singleton<T>
    main_log_file = "Rhea_JobManager.log";
    catchSignals();
    while (true) sleep(1);
}

template<class T>
symbios::Daemon<T>::~Daemon() {
    exitSignal.set_value();
    worker.join();
}

template<class T>
void symbios::Daemon<T>::signalHandler(int sig) {
    switch (sig) {
        case SIGHUP: {
            logMessage(main_log_file, "hangup signal caught");
            break;
        }
        case SIGTERM: {
            logMessage(main_log_file, "terminate signal caught");
            //finalize(); Handle by the destructor
            exit(0);
        }
        default: {
            void *array[20];
            size_t size;
            // get void*'s for all entries on the stack
            size = backtrace(array, 20);
            // print out all the frames to stderr
            fprintf(stderr, "Error: signal %d:\n", sig);
            backtrace_symbols_fd(array, size, STDERR_FILENO);
            ::raise(SIGTERM);
        }
    }
}

template<class T>
void symbios::Daemon<T>::logMessage(const std::string &filename, const std::string &message) {
    std::ofstream outfile;
    outfile.open(filename, std::ios_base::app); // append instead of overwrite
    outfile << message;
}

template<class T>
void symbios::Daemon<T>::catchSignals() {
    signal(SIGTERM, this->signalHandler); /* catch kill signal */
    signal(SIGHUP, this->signalHandler); /* catch hangup signal */
    signal(SIGABRT, this->signalHandler); /* catch hangup signal */
    signal(SIGSEGV, this->signalHandler); /* catch hangup signal */
    signal(SIGBUS, this->signalHandler); /* catch hangup signal */
    signal(SIGCHLD, SIG_IGN);              /* ignore child */
    signal(SIGTSTP, SIG_IGN);              /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}