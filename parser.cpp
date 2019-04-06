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


class Parser{
    private: 

        std::string portname;
        int test_fd, file_descr, nRead, msgcount;
        char buf[1024];
        int cursor;
        
    public:
        Parser(std::string PortName);
        void read_data();
        std::vector<int> parse_HD(bool if_Euler);
        std::vector<int> parse(bool if_Euler);
        void write_for_test(std::string filename);
};


void Parser::read_data(){
    int cursor;

    while(1){
    
        Parser::nRead = read(Parser::file_descr, Parser::buf, 255);
        Parser::cursor = 0;
        
        while(Parser::cursor < Parser::nRead){

            if (Parser::buf[cursor++] == SYNC1){
                Parser::cursor++;
                if(Parser::buf[cursor++] == SYNC2){
                    Parser::cursor++;
                    if(Parser::buf[cursor++] == NAV_CLASS){
                        Parser::cursor++;
                        Parser::cursor++;
                        switch(Parser::buf[cursor]){
                            case HPPOSECEF:
                                Parser::parse_HD(true);
                            case HPPOSLLH:
                                Parser::parse_HD(false);
                            case POSECEF:
                                Parser::parse(true);
                            case POSLLH:
                                Parser::parse(false);
                            default:
                                continue;
                        Parser::cursor += CHECKSUM_LEN; 
                        }
                    }
                }
            }
        
        }
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

    auto offset = (if_Euler) ? 3 : 2;
    
    auto payload = (if_Euler) ? HD_EULER_MSG_PAYLOAD_LEN : HD_POS_MSG_PAYLOAD_LEN;

    for (auto i = Parser::cursor + msg1_offset; i < Parser::cursor +                          msg1_offset + msg1_len; i+=4){
       pre_result.push_back(int((Parser::buf[i]) << 24 |
                                (Parser::buf[i+1]) << 16 |
                                (Parser::buf[i+2]) << 8 |
                                (Parser::buf[i+3])));
     }

    for (auto i = Parser::cursor + msg2_offset; i < Parser::cursor + msg2_offset + msg2_len; i++){
        pre_result.push_back(int((Parser::buf[i])));
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

    for (auto i = Parser::cursor + msg_offset; i < Parser::cursor + msg_offset + msg_len; i+=4){
        result.push_back(int((Parser::buf[i]) << 24 |
                             (Parser::buf[i+1]) << 16 |
                             (Parser::buf[i+2]) << 8 |
                             (Parser::buf[i+3])));
    }

    Parser::cursor += payload; 

    return result;
}



Parser::Parser(std::string PortName = "/dev/ttyACM0"){
    Parser::file_descr = open(PortName.c_str(), O_RDWR);
    
    if (Parser::file_descr < 0){
        std::cout << "Cannot open device at" << PortName << "\n Terminating...";
    }
    
}

void Parser::write_for_test(std::string filename = "testing.txt"){
    Parser::test_fd = open(filename.c_str(), O_RDWR);

    std::vector<char> data = {SYNC1, SYNC2, NAV_CLASS, POSECEF};
    signed char foo = 0x01;

    for (auto i=0; i < POS_MSG_PAYLOAD_LEN + CHECKSUM_LEN; i++){
        data.push_back(foo);
    } 

    std::ofstream of(filename);

    std::ostream_iterator<std::vector<char>> out_iterator(of, "\n");

    std::copy(data.begin(), data.end(), out_iterator);
}

int main(){

    Parser parser;
    parser.write_for_test();
    parser.read_data();
    return 0;
}
