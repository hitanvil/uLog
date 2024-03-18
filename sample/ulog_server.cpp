
#include <unistd.h>
#include <iostream>
#include "../src/uLog.h"

#include <csignal>  // for signal handling

bool exit_flag = false;

// Signal handler function
void signalHandler(int signal) {
    std::cout << "Ctrl+C received. Exiting..." << std::endl;

    exit_flag = true;

    exit(signal);  // Exit the program with the received signal
}

int main(int argc, char* argv[]) {
    int msg_len;
    uLog ulog;

    std::cout << "usage:ulog_server [file_name]" << std::endl;
    std::cout << "      log outputs to file with name file_name." << std::endl;
    std::cout << "      log outputs to screen if file_name is omited." << std::endl;

    std::cout << "Press Ctrl+C to exit." << std::endl;

    // Register the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);

    ulog.set_server(&exit_flag);

    if (argc == 1) {
        ulog.run_server();
    } else {
        std::cout << "log message is stored in file " << argv[1] << std::endl;
        ulog.run_server(argv[1]);
    }
}