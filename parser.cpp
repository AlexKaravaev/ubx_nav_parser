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
        int cursor;


    public:
        Parser(std::string filename);
        void read_data();
        std::vector<int> parse_HD(bool if_Euler);
        std::vector<int> parse(bool if_Euler);
        void write_for_test(std::string filename);
};


static unsigned char Parser::readbyte(){
    if
}
void Parser::read_data(){
    int cursor;

    while(1){

        while (this-<getbyte != SYNC1)
            // Wait till 1st sync byte;
retry_sync:
        if ((sync = this->getbyte()) != SYNC2){
            if(sync == SYNC1)
                goto retry_sync;
            else
                continue;
        }

        this->msg_class = this->getbyte();
        this->id = this->getbyte();

        this->length = this->getbyte();
        this->length += this->getbyte() << 8;

        if (this->length > MLEN)
            continue;

        for (auto i = 0; i < this->length; i++){
            this->msg[i] = this->getbyte();
        }

        this->chka = this->getbyte();
        this->chlb = this->getbyte();

        //TODO: add checksum validation
        if (true){
            switch(this->msg_class){
                case HPPOSECEF:
                    res = Parser::parse_HD(true);
                case HPPOSLLH:
                    res = Parser::parse_HD(false);
                case POSECEF:
                    res = Parser::parse(true);
                case POSLLH:
                    res = Parser::parse(false);
                default:
                    continue;
            }
        }

        std::cout << "error in checksum validation" << std::endl;

    }
}


// Generic func to parse hd message
std::vector<int> Parser::parse_HD(bool if_Euler){
    std::vector<int> pre_result;
    std::vector<int> result;

    auto msg1_offset = (if_Euler) ? HD_EULER_MSG1_OFFSET : HD_POS_MSG1_OFFSET;

    auto msg1_len = (if_Euler) ? HD_EULER_MSG1_LEN : HD_POS_MSG1_LEN;

    auto msg2_offset = (if_Euler) ? HD_EULER_MSG2_OFFSET : HD_POS_MSG2_OFFSET;

    auto msg2_len = (if_Euler) ? HD_EULER_MSG2_LEN : HD_POS_MSG2_LEN;

    msg1_offset += MSG_LEN;
    msg2_offset += MSG_LEN;

    auto offset = (if_Euler) ? 3 : 2;

    auto payload = (if_Euler) ? HD_EULER_MSG_PAYLOAD_LEN : HD_POS_MSG_PAYLOAD_LEN;

    for (auto i = Parser::cursor + msg1_offset; i < Parser::cursor +                          msg1_offset + msg1_len; i+=4){
       pre_result.push_back(int((signed char)(Parser::buf[i]) << 24 |
                                (signed char)(Parser::buf[i+1]) << 16 |
                                (signed char)(Parser::buf[i+2]) << 8 |
                                (signed char)(Parser::buf[i+3])));
     }

    for (auto i = Parser::cursor + msg2_offset; i < Parser::cursor + msg2_offset + msg2_len; i++){
        pre_result.push_back(int((signed char)(Parser::buf[i])));
     }

    for (auto i = 0; i < 2; i++){
        result.push_back(pre_result[i] + (pre_result[i+offset] * 1e-2));
    }

    Parser::cursor += payload;

    return result;
}

std::vector<int> Parser::parse(bool if_Euler){
    std::vector<int> result;

    auto msg_offset = (if_Euler) ? EULER_MSG_OFFSET : POS_MSG_OFFSET;

    auto msg_len = (if_Euler) ? EULER_MSG_LEN : POS_MSG_LEN;

    auto payload = (if_Euler) ? EULER_MSG_PAYLOAD_LEN : POS_MSG_PAYLOAD_LEN;

    msg_offset += MSG_LEN;

    for (auto i = Parser::cursor + msg_offset; i < Parser::cursor + msg_offset + msg_len; i+=4){
        result.push_back(int((signed char)(Parser::buf[i]) << 24 |
                             (signed char)(Parser::buf[i+1]) << 16 |
                             (signed char)(Parser::buf[i+2]) << 8 |
                             (signed char)(Parser::buf[i+3])));
    }

    Parser::cursor += payload;

    return result;
}



Parser::Parser(std::string filename = "testing.txt"){
    std::ifstream infile(filename);

    infile.seekg(0, infile.end);
    this->nRead = infile.tellg();
    infile.seekg(0, infile.beg);

    if (this->nRead > sizeof(this->buf))
        this->nRead = sizeof(this->buf);

    infile.read(this->buf, this->nRead);


}

void Parser::write_for_test(std::string filename = "testing.txt"){
    Parser::test_fd = open(filename.c_str(), O_RDWR);

    std::vector<char> data = {(char)SYNC1, (char)SYNC2, (char)NAV_CLASS, (char)POSECEF};
    signed char foo = 0x01;

    for (auto i=0; i < POS_MSG_PAYLOAD_LEN + CHECKSUM_LEN; i++){
        data.push_back(foo);
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
