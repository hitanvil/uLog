#include <unistd.h>
#include <iostream>
#include "../src/uLog.h"

#define LARGEST_PKT_SIZE (1024)
int main(int argc, char* argv[]) {
    uLog ulog(1, LARGEST_PKT_SIZE);
    std::cout << "usage:ulog_dump_parser file_name" << std::endl;
    std::cout << "      parse raw log meesage into human readiable text file." << std::endl;

    if (argc != 2) {
        std::cout << " Need just one input filename." << std::endl;
        exit(-1);
    }

    FILE* fp_bin;
    fp_bin = fopen(argv[1], "rb");
    if (fp_bin == NULL) {
        std::cout << " Open input file " << argv[1] << " fail." << std::endl;
        exit(-1);
    }

    FILE* fp_log;
    std::string bin_name(argv[1]);
    std::string txt_name;
    txt_name = bin_name.erase(bin_name.rfind('.'));
    txt_name = txt_name + ".log";
    fp_log = fopen(txt_name.data(), "w");

    int16_t msg_size;
    char buf[LARGEST_PKT_SIZE +
             1024];  // the buffer should be large enough to store one message's content

    bool exit_flag = false;
    fread(&msg_size, sizeof(msg_size), 1, fp_bin);
    while (!exit_flag) {
        // read one message size
        *(int16_t*)buf = msg_size;
        fread(buf + sizeof(msg_size), 1, msg_size - sizeof(msg_size), fp_bin);
        ulog.decode_one_msg(buf, fp_log);

        fread(&msg_size, sizeof(msg_size), 1, fp_bin);
        exit_flag = feof(fp_bin);
    }

    fclose(fp_bin);
    fclose(fp_log);
}