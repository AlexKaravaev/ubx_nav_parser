#pragma once

#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLEN 1024
#define MLEN 1024

#define SYNC1 0xB5
#define SYNC2 0x62
#define NAV_CLASS 0x01

#define HPPOSECEF 0x13 // Msg class with high-precision euler angles
#define HPPOSLLH 0x14 // Msg class with high-precision position
#define POSECEF 0x01// Msg class with usual euler angles
#define POSLLH 0x02 // Msg class with usual position

#define HD_EULER_MSG_PAYLOAD_LEN 28
#define HD_EULER_MSG1_OFFSET 8
#define HD_EULER_MSG1_LEN 12
#define HD_EULER_MSG2_OFFSET 20
#define HD_EULER_MSG2_LEN 3

#define HD_POS_MSG_PAYLOAD_LEN 36
#define HD_POS_MSG1_OFFSET 8
#define HD_POS_MSG1_LEN 8
#define HD_POS_MSG2_OFFSET 24
#define HD_POS_MSG2_LEN 2

#define POS_MSG_PAYLOAD_LEN 28
#define POS_MSG_OFFSET 4
#define POS_MSG_LEN 8

#define EULER_MSG_PAYLOAD_LEN 20
#define EULER_MSG_OFFSET 4
#define EULER_MSG_LEN 12

#define CHECKSUM_LEN 2

#define MSG_LEN 2

class Parser{
    private:
        int test_fd, length, bufcnt = 0;
        unsigned char rbuf[1024];
        unsigned char *rp = &rbuf[BLEN];
        unsigned char msg_class, id, chka, chlb, sync, checka, checkb;
        unsigned char msg[MLEN];
        std::vector<long double> res;

    public:
        Parser(std::string filename);
        void read_data(bool verbose);
        std::vector<long double> parse_HD(bool if_Euler);
        std::vector<long double> parse(bool if_Euler);
        void write_for_test(std::string filename);
        unsigned char getbyte();
};
