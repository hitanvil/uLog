
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
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
    int msg_len, idx;
    uLog ulog;

    std::cout << "usage:ulog_dump_server" << std::endl;
    std::cout
        << "      it generates a binary file which stores all received log message in raw format."
        << std::endl;

    std::cout << "Press Ctrl+C to exit." << std::endl;

    // Register the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);

    ulog.set_server(&exit_flag);

    ulog.run_dump_server();
}