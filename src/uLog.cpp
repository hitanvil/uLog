#include "uLog.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>

/*
void uLog::init_case_val() {
    case_val[0] = typeid((int8_t(0))).hash_code();
    case_val[1] = typeid((uint8_t(0))).hash_code();
    case_val[2] = typeid((int16_t(0))).hash_code();
    case_val[3] = typeid((uint16_t(0))).hash_code();
    case_val[4] = typeid((int32_t(0))).hash_code();
    case_val[5] = typeid((uint32_t(0))).hash_code();
    case_val[6] = typeid((int64_t(0))).hash_code();
    case_val[7] = typeid((uint64_t(0))).hash_code();
    case_val[8] = typeid((float(0.))).hash_code();
    case_val[9] = typeid((double(0.))).hash_code();
    case_val[10] = typeid(std::string("")).hash_code();
} */

void uLog::set_client(const uLog_t ulog_level, const std::string& ip, const int port) {
    // create UDP tunnel
    if ((udpTunnel.sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("uLog socket creation failed");
        exit(-1);
    }

    std::memset(&udpTunnel.addr, 0, sizeof(udpTunnel.addr));
    udpTunnel.addr.sin_family = AF_INET;
    udpTunnel.addr.sin_addr.s_addr = inet_addr(ip.data());
    udpTunnel.addr.sin_port = htons(port);

    udpTunnel.buf = (char*)malloc(largest_udp_pkt_size +
                                  num_additional_entries);  // add additional space for safe

    least_log_level = ulog_level;
    udpTunnel.num_used_entries = 0;
}

void uLog::set_server(bool* exit_flag_in, const int port) {
    // create UDP tunnel
    if ((udpTunnel.sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("uLog socket creation failed");
        exit(-1);
    }

    std::memset(&udpTunnel.addr, 0, sizeof(udpTunnel.addr));
    udpTunnel.addr.sin_family = AF_INET;
    udpTunnel.addr.sin_addr.s_addr = INADDR_ANY;
    udpTunnel.addr.sin_port = htons(port);

    if (bind(udpTunnel.sockFd, (const struct sockaddr*)&udpTunnel.addr, sizeof(udpTunnel.addr)) <
        0) {
        perror("uLog socket bind failed");
        exit(-1);
    }

    // largest_udp_pkt_size = largest_pkt_size;
    udpTunnel.buf = (char*)malloc(largest_udp_pkt_size +
                                  num_additional_entries);  // add additional space for safe

    exit_flag = exit_flag_in;
    udpTunnel.num_used_entries = 0;
}

int uLog::log(uLog_t level, std::string module, std::string fname, uint32_t line, uint16_t count,
              ...) {
    va_list args;
    char* buf = udpTunnel.buf + udpTunnel.num_used_entries;
    int num_free_entries =
        largest_udp_pkt_size + num_additional_entries - udpTunnel.num_used_entries;
    char* p_msg_size;
    uint16_t msg_size = 0;
    std::string str;

    if (level < least_log_level) {
        return 0;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int16_t fn_len = fname.size();  // the size does not include '\0' as last char
    int16_t fn_len_1 = fn_len + 1;
    uint16_t name_size = module.size();
    uint16_t name_size_1 = name_size + 1;
    if (num_free_entries >= sizeof(level) + 2 + sizeof(uLog_Msg_t) + sizeof(ts) + sizeof(fn_len) +
                                fn_len + sizeof(line) + sizeof(uLog_Msg_t) +
                                32) {  // assume that module name size is not more than 30 chars
        p_msg_size = buf;              // reserve 2 bytes  for message size
        p_msg_size[0] = p_msg_size[1] = 0;
        buf += 2;
        memcpy(buf, &level, sizeof(level));
        udpTunnel.num_used_entries += 2 + sizeof(level);
        msg_size += 2 + sizeof(level);

        buf = udpTunnel.buf + udpTunnel.num_used_entries;
        memcpy(buf, &ver, sizeof(ver));
        udpTunnel.num_used_entries += sizeof(ver);
        msg_size += sizeof(ver);

        buf = udpTunnel.buf + udpTunnel.num_used_entries;
        memcpy(buf, &ts, sizeof(ts));
        udpTunnel.num_used_entries += sizeof(ts);
        msg_size += sizeof(ts);

        buf = udpTunnel.buf + udpTunnel.num_used_entries;

        memcpy(buf, &name_size_1, sizeof(name_size));
        buf += sizeof(name_size);
        memcpy(buf, &(module.c_str()[0]), name_size);
        buf[name_size] = '\0';
        buf += name_size_1;
        udpTunnel.num_used_entries += name_size_1 + sizeof(name_size);
        msg_size += name_size_1 + sizeof(name_size);

        buf = udpTunnel.buf + udpTunnel.num_used_entries;
        memcpy(buf, &fn_len_1, sizeof(fn_len));
        buf += sizeof(fn_len);
        memcpy(buf, fname.data(), fn_len);
        buf[fn_len] = '\0';
        buf += fn_len_1;
        memcpy(buf, &line, sizeof(line));
        udpTunnel.num_used_entries += sizeof(fn_len) + fn_len_1 + sizeof(line);
        msg_size += sizeof(fn_len) + fn_len_1 + sizeof(line);
        num_free_entries =
            largest_udp_pkt_size + num_additional_entries - udpTunnel.num_used_entries;
    } else {
        fprintf(stderr,
                "uLog::log() has only %d free entries and not enough to store new message.\n",
                num_free_entries);
        exit(-1);
    }

    va_start(args, count);
    for (int idx = 0; idx < count; idx++) {
        std::string msg_name = (std::string)va_arg(args, std::string);
        uint64_t msg_hash = (uint64_t)va_arg(args, uint64_t);
        switch (msg_name[0]) {
            case 'a': {  // typeinfo(int8_t).name()
                //int8_t val8 = va_arg(args, signed char);
                int8_t val8 = (int8_t)va_arg(args, int);
                msg_size += log_num<int8_t>(val8, uLogInt8);
                break;
            }
            case 'h': {  // typeinfo(uint8_t).name()
                //uint8_t uval8 = va_arg(args, unsigned char);
                uint8_t uval8 = (uint8_t)va_arg(args, int);
                msg_size += log_num<uint8_t>(uval8, uLogUInt8);
                break;
            }
            case 't': {  // typeinfo(int16_t).name()
                //int16_t val16 = va_arg(args, int16_t);
                int16_t val16 = (int16_t)va_arg(args, int);
                msg_size += log_num<int16_t>(val16, uLogInt16);
                break;
            }
            case 's': {  // typeinfo(uint16_t).name()
                //uint16_t uval16 = va_arg(args, uint16_t);
                uint16_t uval16 = (uint16_t)va_arg(args, int);
                msg_size += log_num<uint16_t>(uval16, uLogUInt16);
                break;
            }
            case 'i': {  // typeinfo(int32_t).name()
                int32_t val32 = va_arg(args, int32_t);
                msg_size += log_num<int32_t>(val32, uLogInt32);
                break;
            }
            case 'j': {  // typeinfo(uint32_t).name()
                uint32_t uval32 = va_arg(args, uint32_t);
                msg_size += log_num<uint32_t>(uval32, uLogUInt32);
                break;
            }
            case 'l': {  // typeinfo(int64_t/uint64_t).name()
                int64_t val64 = va_arg(args, int64_t);
                msg_size += log_num<int64_t>(val64, uLogInt64);
                break;
            }
            case 'm': {  // typeinfo(int64_t/uint64_t).name()
                uint64_t uval64 = va_arg(args, uint64_t);
                msg_size += log_num<uint64_t>(uval64, uLogUInt64);
                break;
            }
            case 'f': {  // typeinfo(float).name()
                //float valf = va_arg(args, float);
                float valf = (float)va_arg(args, double);
                msg_size += log_num<float>(valf, uLogFloat);
                break;
            }
            case 'd': {  // typeinfo(double).name()
                double vald = va_arg(args, double);
                msg_size += log_num<double>(vald, uLogDouble);
                break;
            }
            case 'N': {                                // typeinfo(std::string).name()
                if (msg_hash == 0x502408f29ce5607d) {  // typeinfo(std::string).hash_code()
                    str = va_arg(args, std::string);
                    msg_size += log_string(str);
                } else {
                    fprintf(stderr, "uLog::log() gets unknown string type.\n");
                    exit(-1);
                }
                break;
            }
            case 'A': {  // typeinfo("  ").name()
                int name_len = msg_name.size();
                if ((msg_name[name_len - 2] == '_') && (msg_name[name_len - 1] == 'c')) {
                    char* pc = va_arg(args, char*);
                    msg_size += log_pchar(pc, msg_name);
                } else {
                    fprintf(stderr, "uLog::log() gets unknown const string type.\n");
                    exit(-1);
                }
                break;
            }
            default:
                fprintf(stderr, "uLog::log() gets unknown message type.\n");
                exit(-1);
        }
    }

    va_end(args);
    memcpy(p_msg_size, &msg_size, sizeof(uint16_t));

    if ((udpTunnel.num_used_entries >= largest_udp_pkt_size) || (level >= uLogWarning)) {
        sendto(udpTunnel.sockFd, (const char*)udpTunnel.buf, udpTunnel.num_used_entries, 0,
               (const struct sockaddr*)&udpTunnel.addr, sizeof(udpTunnel.addr));
        udpTunnel.num_used_entries = 0;
    }

    return msg_size;
}

template <typename T>
int16_t uLog::log_num(T& val, uLog_Msg_t msg_t) {
    char* buf = udpTunnel.buf + udpTunnel.num_used_entries;
    int num_free_entries =
        largest_udp_pkt_size + num_additional_entries - udpTunnel.num_used_entries;
    if (num_free_entries >= (sizeof(msg_t) + sizeof(T))) {
        std::memcpy(buf, &msg_t, sizeof(msg_t));
        buf += sizeof(msg_t);
        std::memcpy(buf, &val, sizeof(T));
        udpTunnel.num_used_entries += sizeof(msg_t) + sizeof(T);
        return (sizeof(msg_t) + sizeof(T));
    } else {
        fprintf(stderr,
                "uLog::log() has only %d free entries and not enough to store new message.\n",
                num_free_entries);
        exit(-1);
    }
}

int16_t uLog::log_string(std::string& str) {
    char* buf = udpTunnel.buf + udpTunnel.num_used_entries;
    int num_free_entries =
        largest_udp_pkt_size + num_additional_entries - udpTunnel.num_used_entries;
    uLog_Msg_t msg_t = uLogString;
    if (str.size() >= max_string_len) {
        fprintf(stderr, "uLog::log() string is too long.\n");
        exit(-1);
    }

    uint8_t str_len = str.size();
    uint8_t str_len_1 = str_len + 1;

    if (num_free_entries >= (sizeof(uint16_t) + sizeof(uint8_t) + str_len)) {
        uLog_Msg_t msg_t = uLogString;
        std::memcpy(buf, &msg_t, sizeof(msg_t));
        buf += sizeof(msg_t);
        std::memcpy(buf, &str_len_1, sizeof(uint8_t));
        buf += sizeof(uint8_t);
        std::memcpy(buf, str.data(), str_len);
        buf[str_len] = '\0';
        udpTunnel.num_used_entries += (sizeof(msg_t) + sizeof(uint8_t) + str_len_1);
        return (sizeof(msg_t) + sizeof(uint8_t) + str_len_1);
    } else {
        fprintf(stderr,
                "uLog::log() has only %d free entries and not enough to store new message.\n",
                num_free_entries);
        exit(-1);
    }
}

int16_t uLog::log_pchar(char* pc, std::string& msg_name) {
    char* buf = udpTunnel.buf + udpTunnel.num_used_entries;
    uLog_Msg_t msg_t = uLogCharPtr;
    int num_free_entries =
        largest_udp_pkt_size + num_additional_entries - udpTunnel.num_used_entries;
    std::string ss = msg_name.substr(1);
    uint8_t msg_len = std::stoi(ss);
    if (num_free_entries >= (sizeof(msg_t) + sizeof(msg_len) + msg_len)) {
        std::memcpy(buf, &msg_t, sizeof(msg_t));
        buf += sizeof(msg_t);
        std::memcpy(buf, &msg_len, sizeof(msg_len));
        buf += sizeof(msg_len);
        std::memcpy(buf, pc, msg_len);
        udpTunnel.num_used_entries += sizeof(msg_t) + sizeof(msg_len) + msg_len;
        return (sizeof(msg_t) + sizeof(msg_len) + msg_len);
    } else {
        fprintf(stderr,
                "uLog::log() has only %d free entries and not enough to store new message.\n",
                num_free_entries);
        exit(-1);
    }
}

int uLog::sync() {
    int ret = udpTunnel.num_used_entries;
    if (udpTunnel.num_used_entries > 0) {
        sendto(udpTunnel.sockFd, (const char*)udpTunnel.buf, udpTunnel.num_used_entries, 0,
               (const struct sockaddr*)&udpTunnel.addr, sizeof(udpTunnel.addr));
        udpTunnel.num_used_entries = 0;
    }
    return ret;
}

void gen_filename(std::string& fname) {
    int idx;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    char* utc = asctime(gmtime(&ts.tv_sec));

    // get month
    static char month_list[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static char month_digital_list[12][3] = {"01", "02", "03", "04", "05", "06",
                                             "07", "08", "09", "10", "11", "12"};
    std::string month(month_list[0]);
    month[0] = utc[4];
    month[1] = utc[5];
    month[2] = utc[6];
    for (idx = 0; idx < 12; idx++) {
        if (month == std::string(month_list[idx])) {
            month = month_digital_list[idx];
            break;
        }
    }

    fname = fname + utc[20] + utc[21] + utc[22] + utc[23];  // add year
    utc[8] = (utc[8] == 32) ? 48 : utc[8];
    fname = fname + '_' + month + '_' + utc[8] + utc[9];  //  add month and day
    utc[11] = (utc[11] == 32) ? 48 : utc[11];
    utc[14] = (utc[14] == 32) ? 48 : utc[14];
    utc[17] = (utc[17] == 32) ? 48 : utc[17];
    fname = fname + '_' + utc[11] + utc[12] + '_' + utc[14] + utc[15] + '_' + utc[17] + utc[18] +
            ".bin";
}

void uLog::run_dump_server(char* filename) {
    FILE* fp;
    if (filename == NULL) {
        std::string fname("ulog_dump_");
        gen_filename(fname);

        printf("Raw log message is dumped into file %s\n", fname.data());

        fp = fopen(fname.data(), "wb");
    } else {
        fp = fopen(filename, "wb");
    }

    struct sockaddr_in sockaddr;
    int len = sizeof(sockaddr);

    while (!(*exit_flag)) {
        udpTunnel.num_used_entries =
            recvfrom(udpTunnel.sockFd, udpTunnel.buf, largest_udp_pkt_size + num_additional_entries,
                     0, (struct sockaddr*)&udpTunnel.addr, (socklen_t*)(&len));

        fwrite(udpTunnel.buf, 1, udpTunnel.num_used_entries, fp);
    }

    if (fp != NULL) {
        fclose(fp);
    }
}

void uLog::run_server(char* fname) {
    FILE* fp;
    FILE* fp_dump;
    std::string filename;
    if (fname == NULL) {
        std::string filename("ulog_dump_");
        gen_filename(filename);
        fp_dump = fopen(filename.data(), "wb");
        fp = stdout;
        fprintf(stderr,
                "log message prints on screen.\n"
                "Raw log message is dumped into file %s.\n",
                filename.data());
    } else {
        fp = fopen(fname, "w");
        if (fp == NULL) {
            filename = "ulog_dump_";
            gen_filename(filename);
            fp_dump = fopen(filename.data(), "wb");
            fp = stdout;
            fprintf(stderr,
                    "uLog server open file %s fails.\nlog message prints on screen.\n"
                    "Raw log message is dumped into file %s.\n",
                    fname, filename.data());
        } else {
            filename = fname;
            filename = filename.erase(filename.rfind('.'));
            filename = filename + ".bin";
            fp_dump = fopen(filename.data(), "wb");

            fprintf(stdout,
                    "log message prints to file %s.\n"
                    "Raw log message is dumped into file %s.\n",
                    fname, filename.data());
        }
    }

    struct sockaddr_in sockaddr;
    int len = sizeof(sockaddr);

    while (!(*exit_flag)) {
        udpTunnel.num_used_entries =
            recvfrom(udpTunnel.sockFd, udpTunnel.buf, largest_udp_pkt_size + num_additional_entries,
                     0, (struct sockaddr*)&udpTunnel.addr, (socklen_t*)(&len));

        fwrite(udpTunnel.buf, 1, udpTunnel.num_used_entries, fp_dump);

        run_server_kernel(fp);

        // output to file immediately after a UDP packet is parsed.
        if (fname != NULL) {
            fclose(fp);
            fp = fopen(fname, "a");
            if (fp == NULL) {
                fp = stdout;
                fprintf(stderr, "uLog server open file %s fails.\n", fname);
            }
        }
    }

    if (fp != NULL) {
        fclose(fp);
    }

    fclose(fp_dump);
}

void uLog::run_server_kernel(FILE* fp) {
    int16_t one_msg_size;
    int udp_pkt_size = udpTunnel.num_used_entries;
    char* buf = udpTunnel.buf;
    while (udp_pkt_size > 0) {
        one_msg_size = decode_one_msg(buf, fp);
        buf += one_msg_size;
        udp_pkt_size -= one_msg_size;
    }
}

int uLog::decode_one_msg(char* buf, FILE* fp) {
    int16_t one_msg_size, idx;
    char* buf_start = buf;
    memcpy(&one_msg_size, buf, sizeof(int16_t));
    buf += sizeof(int16_t);

    if (one_msg_size == 0) {
        return 0;
    }

    uLog_t level;
    memcpy(&level, buf, sizeof(level));
    buf += sizeof(level);

    uint8_t version;
    memcpy(&version, buf, sizeof(version));
    buf += sizeof(version);

    if (ver != version) {
        fprintf(stderr, "uLog server receives an unknown version log. ver=%d version=%d\n", ver,
                version);
        exit(-1);
    }

    struct timespec ts;
    memcpy(&ts, buf, sizeof(timespec));
    buf += sizeof(timespec);
    char* utc = asctime(gmtime(&ts.tv_sec));
    utc[19] = '\0';
    fprintf(fp, "%s:", utc);

    int ms = floor(ts.tv_nsec / 1e6);
    fprintf(fp, "%03d:", ms);
    ts.tv_nsec -= ms * 1e6;
    int us = floor(ts.tv_nsec / 1e3);
    fprintf(fp, "%03d:", us);
    ts.tv_nsec -= us * 1e3;
    int ns = ts.tv_nsec;
    fprintf(fp, "%03d ", ns);
    utc[24] = '\0';
    fprintf(fp, "%s ", &utc[20]);  // print year

    switch (level) {
        case uLogDebug:
            fprintf(fp, "uLogDebug:");
            break;
        case uLogInfo:
            fprintf(fp, "uLogInfo:");
            break;
        case uLogWarning:
            fprintf(fp, "uLogWarning:");
            break;
        case uLogError:
            fprintf(fp, "uLogError:");
            break;
        default:
            fprintf(stderr, "uLog server receives an unknown log level.");
            exit(-1);
    }

    uint16_t name_size;
    memcpy(&name_size, buf, sizeof(name_size));
    buf += sizeof(name_size);
    fprintf(fp, "%s:", buf);  // print module name
    buf += name_size;

    uint32_t line;
    memcpy(&name_size, buf, sizeof(name_size));
    buf += sizeof(name_size);
    fprintf(fp, "%s:", buf);  // print file name
    buf += name_size;
    memcpy(&line, buf, sizeof(line));
    fprintf(fp, "%4d:", line);
    buf += sizeof(line);

    uLog_Msg_t msg_t;
    while ((buf - buf_start) < one_msg_size) {
        memcpy(&msg_t, buf, sizeof(msg_t));
        buf += sizeof(msg_t);
        switch (msg_t) {
            case uLogInt8:
                int8_t i8;
                num_cpy<int8_t>(&i8, &buf);
                fprintf(fp, " %d", i8);
                break;
            case uLogUInt8:
                uint8_t u8;
                num_cpy<uint8_t>(&u8, &buf);
                fprintf(fp, " %d", u8);
                break;
            case uLogInt16:
                int16_t i16;
                num_cpy<int16_t>(&i16, &buf);
                fprintf(fp, " %d", i16);
                break;
            case uLogUInt16:
                uint16_t u16;
                num_cpy<uint16_t>(&u16, &buf);
                fprintf(fp, " %d", u16);
                break;
            case uLogInt32:
                int32_t i32;
                num_cpy<int32_t>(&i32, &buf);
                fprintf(fp, " %d", i32);
                break;
            case uLogUInt32:
                int32_t u32;
                num_cpy<int32_t>(&u32, &buf);
                fprintf(fp, " %d", u32);
                break;
            case uLogInt64:
                int64_t i64;
                num_cpy<int64_t>(&i64, &buf);
                fprintf(fp, " %ld", i64);
                break;
            case uLogUInt64:
                uint64_t u64;
                num_cpy<uint64_t>(&u64, &buf);
                fprintf(fp, " %ld", u64);
                break;
            case uLogFloat:
                float f;
                num_cpy<float>(&f, &buf);
                fprintf(fp, " %.16g", f);
                break;
            case uLogDouble:
                double d;
                num_cpy<double>(&d, &buf);
                fprintf(fp, " %.16g", d);
                break;
            case uLogString:
                delog_string(&buf, fp);
                break;
            case uLogCharPtr:
                delog_string(&buf, fp);
                break;
            default:
                fprintf(stderr, "uLog serever receives unknown message type.\n");
                exit(-1);
        }
    }
    fprintf(fp, "\n");
    return one_msg_size;
}

void uLog::delog_string(char** buf, FILE* fp) {
    uint8_t str_len, idx;
    memcpy(&str_len, *buf, sizeof(uint8_t));
    *buf += sizeof(uint8_t);
    char tmp[128];
    memcpy(tmp, *buf, str_len);
    if (str_len > max_string_len) {
        fprintf(stderr, "uLog serever receives a string which lenght is %d.\n", str_len);
        exit(-1);
    }
    tmp[str_len] = '\0';
    fprintf(fp, " %s", tmp);
    *buf += str_len;
}

template <typename T>
void uLog::num_cpy(T* t, char** buf) {
    memcpy(t, *buf, sizeof(T));
    *buf += sizeof(T);
}