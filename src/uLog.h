#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <cstdarg>
#include <string>
#include <typeinfo>

#pragma once

/**
     * @brief uLog level
     */
typedef enum _uLog_t : uint8_t { uLogDebug, uLogInfo, uLogWarning, uLogError } uLog_t;

#define __FILENAME__ \
    (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define TYPE(arg) std::string(typeid(arg).name()), typeid(arg).hash_code()

/**
 * ref: https://blog.csdn.net/u011787119/article/details/53815950
 */
#define ATTR(args) args
#define COUNT_PARMS_IMP(_1, _2, _3, _4, _5, _6, _7, _8, _9, NUM, ...) NUM
#define COUNT_PARMS(...) ATTR(COUNT_PARMS_IMP(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define ARG_1(arg) TYPE(arg), arg
//    { std::cout << arg << " "; }
#define ARG_1_MORE(arg) TYPE(arg), arg,
#define ARG_2(arg, ...) ARG_1_MORE(arg) ATTR(ARG_1(__VA_ARGS__))
#define ARG_3(arg, ...) ARG_1_MORE(arg) ATTR(ARG_2(__VA_ARGS__))
#define ARG_4(arg, ...) ARG_1_MORE(arg) ATTR(ARG_3(__VA_ARGS__))
#define ARG_5(arg, ...) ARG_1_MORE(arg) ATTR(ARG_4(__VA_ARGS__))
#define ARG_6(arg, ...) ARG_1_MORE(arg) ATTR(ARG_5(__VA_ARGS__))
#define ARG_7(arg, ...) ARG_1_MORE(arg) ATTR(ARG_6(__VA_ARGS__))
#define ARG_8(arg, ...) ARG_1_MORE(arg) ATTR(ARG_7(__VA_ARGS__))
#define ARG_9(arg, ...) ARG_1_MORE(arg) ATTR(ARG_8(__VA_ARGS__))
#define ARG_10(arg, ...) ARG_1_MORE(arg) ATTR(ARG_9(__VA_ARGS__))
#define ARG_11(arg, ...) ARG_1_MORE(arg) ATTR(ARG_10(__VA_ARGS__))
#define ARG_12(arg, ...) ARG_1_MORE(arg) ATTR(ARG_11(__VA_ARGS__))
#define ARG_13(arg, ...) ARG_1_MORE(arg) ATTR(ARG_12(__VA_ARGS__))
#define ARG_14(arg, ...) ARG_1_MORE(arg) ATTR(ARG_13(__VA_ARGS__))
#define ARG_15(arg, ...) ARG_1_MORE(arg) ATTR(ARG_14(__VA_ARGS__))
#define ARG_16(arg, ...) ARG_1_MORE(arg) ATTR(ARG_15(__VA_ARGS__))

//#define CHARIZE(arg) #@ arg

#define SYMBOL_CATENATE(arg1, arg2) arg1##arg2

//#define CHARIZE_WITH_MACRO(arg) CHARIZE(arg)
#define SYMBOL_CATENATE_WITH_MACRO(arg1, arg2) SYMBOL_CATENATE(arg1, arg2)

#define ARG_N(...) \
    ATTR(SYMBOL_CATENATE_WITH_MACRO(ARG_, ATTR(COUNT_PARMS(__VA_ARGS__)))(__VA_ARGS__))

#define uLOG(level, moudle, ...)                                                  \
    do {                                                                          \
        ulog.log(level, moudle, __FILENAME__, __LINE__, COUNT_PARMS(__VA_ARGS__), \
                 ARG_N(__VA_ARGS__));                                             \
    } while (0)

class uLog {
   public:
    uLog(const int largest_pkt_size = 1024) { largest_udp_pkt_size = largest_pkt_size; };
    void set_client(const uLog_t ulog_level = uLogWarning, const std::string& ip = "127.0.0.1",
                    const int port = 5400, const int largest_pkt_size = 1024);

    void set_server(bool* exit_flag, const int port = 5400, const int largest_pkt_size = 1024);

    ~uLog(){};

    /**
     * @brief generate log stream
     * @param  level            this log's level
     * @param  ...              msg_type, msg, msg_type, msg, ... the last msg should be uLogMsgEnd
     *
     */

    /**
     * @brief generate log stream
     * @param  level           this log's level
     * @param  module          the name of a module that this log belongs to
     * @param  fname           file name
     * @param  line            line number
     * @count  count           number of input arg in ...
     */
    int log(uLog_t level, std::string module, std::string fname, uint32_t line, uint16_t count,
            ...);

    /**
     * @brief force to send all buffer message
     * @return int
     */
    int sync();

    /**
     * @brief wait UDP packet and decompress it into readable log information
     * @param filename path and file name to store log information. log just output to sreen when it is NULL.
     */
    void run_server(char* filename = NULL);

   private:
    /**
     * @brief uLog message data type
     *  uLogMsgEnd should be first definition as in stream, 0 means packet size is 0.
     */
    typedef enum uLog_Msg_t : uint8_t {
        uLogInt8,
        uLogUInt8,
        uLogInt16,
        uLogUInt16,
        uLogInt32,
        uLogUInt32,
        uLogInt64,
        uLogUInt64,
        uLogFloat,
        uLogDouble,
        uLogString,
        uLogCharPtr
    } uLog_Msg_t;

    const uint8_t max_string_len = 64;

    int largest_udp_pkt_size = 1024;
    int num_additional_entries = largest_udp_pkt_size;
    struct {
        struct sockaddr_in addr;
        int sockFd = -1;
        char* buf;
        int num_used_entries = 0;
    } udpTunnel;

    uLog_t least_log_level;

    /**
     * client side log function
    */
    template <typename T>
    int16_t log_num(T& val, uLog_Msg_t);
    int16_t log_string(std::string&);
    int16_t log_pchar(char* pc, std::string& msg_name);

    /**
     * server side log member
    */
    bool* exit_flag;
    void run_server_kernel(FILE* fp);
    int decode_one_msg(char* buf, FILE* fp);
    template <typename T>
    void num_cpy(T* t, char** buf);
    void delog_string(char** buf, FILE* fp);

    void init_case_val(){};
};