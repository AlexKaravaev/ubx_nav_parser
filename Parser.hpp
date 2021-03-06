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

#define HPPOSECEF 0x13 // Msg class with high-precision ECEF angles
#define HPPOSLLH 0x14 // Msg class with high-precision position
#define POSECEF 0x01// Msg class with usual ECEF angles
#define POSLLH 0x02 // Msg class with usual position
#define ATT 0x05 // Msg class with euler angles

#define HD_ECEF_MSG_PAYLOAD_LEN 28
#define HD_ECEF_MSG1_OFFSET 8
#define HD_ECEF_MSG1_LEN 12
#define HD_ECEF_MSG2_OFFSET 20
#define HD_ECEF_MSG2_LEN 3

#define HD_POS_MSG_PAYLOAD_LEN 36
#define HD_POS_MSG1_OFFSET 8
#define HD_POS_MSG1_LEN 8
#define HD_POS_MSG2_OFFSET 24
#define HD_POS_MSG2_LEN 2

#define POS_MSG_PAYLOAD_LEN 28
#define POS_MSG_OFFSET 4
#define POS_MSG_LEN 8

#define ECEF_MSG_PAYLOAD_LEN 20
#define ECEF_MSG_OFFSET 4
#define ECEF_MSG_LEN 12

#define ATT_MSG_PAYLOAD_LEN 32
#define ATT_MSG_OFFSET 8
#define ATT_MSG_LEN 12

#define CHECKSUM_LEN 2

#define MSG_LEN 2

class Parser{
    private:
        int test_fd, length, bufcnt = 0;
        unsigned char rbuf[1024];
        unsigned char *rp = &rbuf[BLEN];
        unsigned char msg_class, id, chka, chlb, sync, checka, checkb;
        unsigned char msg[MLEN];
        std::vector<long double> ECEF={};
        std::vector<long double> POS={};
        std::vector<long double> HP_ECEF={};
        std::vector<long double> HP_POS={};
        std::vector<long double> EULER={};
    public:
        Parser(std::string filename);
        void read_data(bool verbose);
        long double parse_4_byte(int pos);
        long double parse_part(int pos, int bytes_long);
        void parse_msg(int id, bool if_verbose);
        void update(std::vector<long double> result, char id, bool if_verbose);
        void write_for_test(std::string filename);
        unsigned char getbyte();
};
