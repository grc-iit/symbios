#ifndef SYMBIOS_DAEMON_H
#define SYMBIOS_DAEMON_H

#include <fstream>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
#include <future>

#include <basket/common/singleton.h>
#include <mpi.h>
#include <thread>
#include <basket/common/data_structures.h>

namespace symbios {
    template<class T>
    class Daemon {
    public:
        static CharStruct main_log_file;
        std::promise<void> exitSignal;
        std::thread worker;
        std::shared_ptr<T> jobManager;

        Daemon(CharStruct main_log_file = "/tmp/tmp.BUKlhPiLxF/build/symbios_server.log"): jobManager() {
            main_log_file = Daemon<T>::main_log_file;
            std::future<void> futureObj = exitSignal.get_future();
            jobManager=std::make_shared<T>();
            worker = std::thread(&T::Run, jobManager.get(), std::move(futureObj));
            printf("Running\n");
            catchSignals();
        }

        ~Daemon(){
            exitSignal.set_value();
            worker.join();
        }

    private:
        static void signalHandler(int sig){
            auto instance = basket::Singleton<symbios::Daemon<T>>::GetInstance();
            switch (sig) {
                case SIGHUP: {
                    logMessage(instance->main_log_file.c_str(), "hangup signal caught");
                    break;
                }
                case SIGTERM: {
                    logMessage(instance->main_log_file.c_str(), "terminate signal caught");
                    //finalize(); Handle by the destructor
                    MPI_Finalize();
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

        static void logMessage(const std::string &filename, const std::string &message){
            std::ofstream outfile;
            outfile.open(filename, std::ios_base::app); // append instead of overwrite
            outfile << message;
        }

        void catchSignals(){
            signal(SIGTERM, symbios::Daemon<T>::signalHandler); /* catch kill signal */
            signal(SIGHUP, symbios::Daemon<T>::signalHandler); /* catch hangup signal */
            signal(SIGABRT, symbios::Daemon<T>::signalHandler); /* catch hangup signal */
            signal(SIGSEGV, symbios::Daemon<T>::signalHandler); /* catch hangup signal */
            signal(SIGBUS, symbios::Daemon<T>::signalHandler); /* catch hangup signal */
            signal(SIGCHLD, SIG_IGN);              /* ignore child */
            signal(SIGTSTP, SIG_IGN);              /* ignore tty signals */
            signal(SIGTTOU, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
        }

    };

    template<typename T>
    CharStruct symbios::Daemon<T>::main_log_file = "";
}

#endif //SYMBIOS_DAEMON_H