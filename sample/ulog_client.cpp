#include <unistd.h>
#include <iostream>
#include "../src/uLog.h"

int main() {
    int msg_len;
    uLog ulog(1, 64);
    ulog.set_client(uLogDebug);
    int8_t s8 = 1;
    uint8_t u8 = 2;
    int16_t s16 = 4;
    uint16_t u16 = 8;
    int32_t s32 = 16;
    uint32_t u32 = 32;
    int64_t s64 = 64;
    uint64_t u64 = 128;
    float f = 256.;
    double d = 512.;
    std::string str1("example1");
    std::string str2("example2");
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_REALTIME, &ts0);

    uLOG(uLogDebug, "Module", "string=", str1);
    uLOG(uLogDebug, "Module", "s8=", s8);
    uLOG(uLogDebug, "Module", "u8=", u8);
    uLOG(uLogDebug, "Module", "s16=", s16);
    uLOG(uLogDebug, "Module", "u16=", u16);
    uLOG(uLogDebug, "Module", "s32=", s32);
    uLOG(uLogDebug, "Module", "u32=", u32);
    uLOG(uLogDebug, "Module", "s64=", s64);
    uLOG(uLogDebug, "Module", "u64=", u64);
    uLOG(uLogDebug, "Module", "float=", f);
    uLOG(uLogDebug, "Module", "float=", 0.00000256);
    uLOG(uLogDebug, "Module", "float=", 25600000000000.);
    uLOG(uLogDebug, "Module", "double=", d);
    uLOG(uLogDebug, "Module", "double=", 0.00000512);
    uLOG(uLogDebug, "Module", "double=", 51200000000000.);
    uLOG(uLogDebug, "Module", "string=", str2);

    for (int idx = 0; idx < 1000; idx++) {
        uLOG(uLogDebug, "Module", "string=", str1);
        uLOG(uLogDebug, "Module", "double=", 51200000000000.);
    }

    ulog.sync();

    clock_gettime(CLOCK_REALTIME, &ts1);
    str1.clear();
    str1 = str1 + asctime(gmtime(&ts0.tv_sec));
    str1 = str1.erase(str1.rfind('\n')) + " " + std::to_string(ts0.tv_nsec) + "ns";

    str2.clear();
    str2 = str2 + asctime(gmtime(&ts1.tv_sec));
    str2 = str2.erase(str2.rfind('\n')) + " " + std::to_string(ts1.tv_nsec) + "ns";

    std::cout << "ulog client start at " << str1 << std::endl;
    std::cout << " ulog client stop at " << str2 << std::endl;
    return 0;
}
