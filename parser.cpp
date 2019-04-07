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

#define SYNC1 0xB5
#define SYNC2 0x62
#define BAUDRATE B460800
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

#define BLEN 1024
#define MLEN 1024

class Parser{
    private:

        std::string portname;
        int test_fd, file_descr, nRead, msgcount;
        unsigned char rbuf[1024];
        unsigned char *rp = &rbuf[BLEN];
        int bufcnt = 0;
        unsigned char msg_class;
        unsigned char id;
        int length;
        unsigned char chka;
        unsigned char chlb;
        unsigned char msg[MLEN];
        std::vector<int> res;
        std::vector<float> hd_res;
        unsigned char sync;
        std::string fn;
        std::ifstream infile;

    public:
        Parser(std::string filename);
        void read_data();
        std::vector<float> parse_HD(bool if_Euler);
        std::vector<int> parse(bool if_Euler);
        void write_for_test(std::string filename);
        unsigned char getbyte();
};


unsigned char Parser::getbyte(){

    //std::cout << (rp - rbuf) << '\n';
    if ((rp - rbuf) >= bufcnt){

        bufcnt = read(test_fd, rbuf, BLEN);

        if (bufcnt <= 0){
            //std::cout << "Error in getbyte()\n";
            ;
        }
        else{

        }
        //std::cout << "buf: " << bufcnt << std::endl;
        rp = rbuf;
        //std::cout << (int)(*rp) << ' ';
    }

    //'std::cout << (int)(*rp) << ' ';
    std::cout << rbuf[0];
    return *rp++;
}

void Parser::read_data(){

    while(1){

        while (getbyte() != 0xB5)
          /* wait*/ ;
retry_sync:

        if ((sync = getbyte()) != SYNC2){
            if(sync == SYNC1)
                goto retry_sync;
            else
                continue;
        }

        msg_class = getbyte();
        id = getbyte();
        length = getbyte();
        length += getbyte() << 8;

        if (length > MLEN){
            continue;
        }
        for (auto i = 0; i < length; i++){
            msg[i] = getbyte();
        }
        std::cout <<"Msg class: " << (int)msg_class << " \n";
        std::cout <<"id class: " << (int)id << " \n";
        std::cout <<"length: " << (int)length << " \n";
        chka = getbyte();
        chlb = getbyte();
        //TODO: add checksum validation
        if (true){
            switch(id){
                case HPPOSECEF:
                    hd_res = Parser::parse_HD(true);
                    for(auto const& val1: hd_res){
                      std::cout << val1 << " ";
                    }
                case HPPOSLLH:
                    hd_res = Parser::parse_HD(false);
                    // for(auto const& val2: hd_res){
                    //   std::cout << val2 << " ";
                    // }
                case POSECEF:
                    std::cout << "ff";
                    res = Parser::parse(true);
                    // for(auto const& val3: res){
                    //   std::cout << val3 << " ";
                    // }
                case POSLLH:
                    std::cout << "aff";
                    res = Parser::parse(false);
                    // for(auto const& val4: res){
                    //   std::cout << val4 << " ";
                    // }
            }
            for(auto const& val1: hd_res){
              std::cout << val1 << " ";
            }
            for(auto const& val4: res){
              std::cout << val4 << " ";
            }
            std::cout<<'\n';
        }

        std::cout << "error in checksum validation\n" << std::endl;

    }
}


// Generic func to parse hd message
std::vector<float> Parser::parse_HD(bool if_Euler){
    std::vector<int> pre_result;
    std::vector<float> result;

    auto msg1_offset = (if_Euler) ? HD_EULER_MSG1_OFFSET : HD_POS_MSG1_OFFSET;

    auto msg1_len = (if_Euler) ? HD_EULER_MSG1_LEN : HD_POS_MSG1_LEN;

    auto msg2_offset = (if_Euler) ? HD_EULER_MSG2_OFFSET : HD_POS_MSG2_OFFSET;

    auto msg2_len = (if_Euler) ? HD_EULER_MSG2_LEN : HD_POS_MSG2_LEN;

    msg1_offset += MSG_LEN;
    msg2_offset += MSG_LEN;

    auto offset = (if_Euler) ? 3 : 2;

    auto payload = (if_Euler) ? HD_EULER_MSG_PAYLOAD_LEN : HD_POS_MSG_PAYLOAD_LEN;

    for (auto i = msg1_offset; i <  msg1_offset + msg1_len; i+=4){
       pre_result.push_back(int((signed char)(this->msg[i]) << 24 |
                                (signed char)(this->msg[i+1]) << 16 |
                                (signed char)(this->msg[i+2]) << 8 |
                                (signed char)(this->msg[i+3])));
     }

    for (auto i = msg2_offset; i < msg2_offset + msg2_len; i++){
        pre_result.push_back(int((signed char)(this->msg[i])));
     }

    for (auto i = 0; i < 2; i++){
        result.push_back(pre_result[i] + (pre_result[i+offset] * 1e-2));
    }
    std::cout << "shd: " << result.size() << std::endl;

    return result;
}

std::vector<int> Parser::parse(bool if_Euler){
    std::vector<int> result;

    auto msg_offset = (if_Euler) ? EULER_MSG_OFFSET : POS_MSG_OFFSET;

    auto msg_len = (if_Euler) ? EULER_MSG_LEN : POS_MSG_LEN;

    auto payload = (if_Euler) ? EULER_MSG_PAYLOAD_LEN : POS_MSG_PAYLOAD_LEN;

    msg_offset += MSG_LEN;

    for (auto i = msg_offset; i < msg_offset + msg_len; i+=4){
        result.push_back(int((signed char)(this->msg[i]) << 24 |
                             (signed char)(this->msg[i+1]) << 16 |
                             (signed char)(this->msg[i+2]) << 8 |
                             (signed char)(this->msg[i+3])));
    }

    std::cout << "s: " << result.size() << std::endl;
    return result;
}



Parser::Parser(std::string filename = "testing.txt"){
    //std::ifstream Parser::infile(filename);
    test_fd = open(filename.c_str(), O_RDWR);

}

void Parser::write_for_test(std::string filename = "testing.txt"){
    Parser::test_fd = open(filename.c_str(), O_RDWR);

    std::vector<int> data = {0x00, SYNC1, SYNC2, NAV_CLASS, POSECEF,
                POS_MSG_PAYLOAD_LEN & 0x00FF, (POS_MSG_PAYLOAD_LEN & 0xFF00) >> 8};
    signed char foo = 0x01;

    for (auto i=0; i < POS_MSG_PAYLOAD_LEN + CHECKSUM_LEN; i++){
        data.push_back(foo);
    }

    // JUNK
    data.push_back(0x00);
    data.push_back(0x23);

    data.push_back(SYNC1);
    data.push_back(SYNC2);
    data.push_back(NAV_CLASS);
    data.push_back(POSECEF);
    data.push_back(POS_MSG_PAYLOAD_LEN & 0x00FF);
    data.push_back((POS_MSG_PAYLOAD_LEN & 0xFF00) >> 8);
    for (auto i=0; i < POS_MSG_PAYLOAD_LEN + CHECKSUM_LEN; i++){
        data.push_back(foo);
    }

    data.push_back(SYNC1);
    data.push_back(SYNC2);
    data.push_back(NAV_CLASS);
    data.push_back(HPPOSLLH);
    data.push_back(HD_POS_MSG_PAYLOAD_LEN & 0x00FF);
    data.push_back((HD_POS_MSG_PAYLOAD_LEN & 0xFF00) >> 8);

    for (auto i=0; i < HD_POS_MSG_PAYLOAD_LEN + CHECKSUM_LEN; i++){
        if(i > 7 && i <17)
          data.push_back(foo);
        else
          data.push_back(0x02);
    }

    std::ofstream of(filename);

    std::ostream_iterator<char> out_iterator(of, "");

    std::copy(data.begin(), data.end(), out_iterator);
}

int main(){

    Parser parser;
    parser.write_for_test();
    parser.read_data();
    return 0;
}
