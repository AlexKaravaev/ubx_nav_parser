#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

#define SYNC1 0xB5
#define SYNC2 0x62 
#define BAUDRATE B460800
#define NAV_CLASS 0x01

#define HPPOSECEF 0x13 // Msg class with high-precision euler angles
#define HPPOSLLH 0x14 // Msg class with high-precision position
#define POSECEF 0x01// Msg class with usual euler angles
#define POSLLH 0x02 // Msg class with usual position

#define HD_EULER_PAYLOAD_LEN 28
#define HD_EULER_MSG1_OFFSET 8
#define HD_EULER_MSG1_LEN 12
#define HD_EULER_MSG2_OFFSET 20
#define HD_EULER_MSG2_LEN 3

#define HD_POS_MSG1_OFFSET 8
#define HD_POS_MSG1_LEN 8
#define HD_POS_MSG2_OFFSET 24
#define HD_POS_MSG2_LEN 2
#define HD_POS_PAYLOAD_LEN 36

#define POS_PAYLOAD_LEN 28
#define POS_MSG1_OFFSET 4
#define POS_MSG1_LEN 8

#define EULER_PAYLOAD_LEN 20
#define EULER_MSG1_OFFSET 4
#define EULER_MSG1_LEN 12

class Parser{
    private: 
        struct NAV{

        };

        std::string portname;
        int file_descr, nRead, msgcount;
        char buf[1024];
        int cursor;

    public:
        Parser(std::string PortName);
        void read_data(char* buf, int fd);
        std::vector<int> parse_HD_EULER();
        std::vector<int> parse_HD_POS();
        std::vector<int> parse_EULER();
        std::vector<int> parse_POS();
        std::vector<int> parse_HD(bool if_Euler);
};


void Parser::read_data(char* buf, int fd){
    int cursor;
    
    //flags for catching sync bytes
    bool FIRST_SYNC = false, SEC_SYNC = false;

    while(1){
    
        Parser::nRead = read(fd, buf, 255);
        Parser::cursor = 0;
        
        while(Parser::cursor < Parser::nRead){

            if (buf[cursor++] == SYNC1){
                cursor++;
                if(buf[cursor++] == SYNC2){
                    cursor++;
                    if(buf[cursor++] == NAV_CLASS){
                        switch(buf[cursor]){
                            case HPPOSECEF:
                                Parser::parse_HD_EULER();
                            case HPPOSLLH:
                                std::cout << "MOCK";
                            case POSECEF:
                                std::cout << "MOCK";
                            case POSLLH:
                                std::cout << "MOCK";
                        }
                    }
                }
            }
        
        }
    }
}

std::vector<int> Parser::parse_HD_EULER(){
    
    // Vector = {ecefX, ecefY, ecefZ, ecefX_HP, ecefY_HP, ecefZ_hp}
    // 8, 12, 16, 20, 21, 22 - bytes adresses
    std::vector<int> pre_ece;
    
    // Final HD euler angles
    std::vector<int> ece;
    
    
    //TODO: dafuck don't know how to read bytes
    
    for (auto i = Parser::cursor + HD_EULER_MSG1_OFFSET; i < Parser::cursor + HD_EULER_MSG1_OFFSET + HD_EULER_MSG1_LEN; i += 4){

       pre_ece.push_back(int((unsigned char)(Parser::buf[i]) << 24 |
                (unsigned char)(Parser::buf[i+1]) << 16 |
                (unsigned char)(Parser::buf[i+2]) << 8 |
                (unsigned char)(Parser::buf[i+3])));
    }

    //TODO
    for(auto i = Parser::cursor + HD_EULER_MSG2_OFFSET;
            i < Parser::cursor + HD_EULER_MSG2_OFFSET + HD_EULER_MSG2_LEN; i++){
        pre_ece.push_back(int((unsigned char)(Parser::buf[i])));

    }

    for (auto i = 0; i < 3; i++){
        ece.push_back(pre_ece[i] + (pre_ece[i+3] * 1e-2));
    }

    Parser::cursor += HD_EULER_PAYLOAD_LEN;
    return ece;
}

std::vector<int> Parser::parse_HD_POS(){
    std::vector<int> pre_pos;

    std::vector<int> pos;

    for (auto i = Parser::cursor + HD_POS_MSG1_OFFSET; i < Parser::cursor + HD_POS_MSG1_OFFSET + HD_POS_MSG1_LEN; i+=4){
        pre_pos.push_back(int((unsigned char)(Parser::buf[i]) << 24 |
                              (unsigned char)(Parser::buf[i+1]) << 16 |
                              (unsigned char)(Parser::buf[i+2]) << 8 |
                              (unsigned char)(Parser::buf[i+3])));
    }

    for (auto i = Parser::cursor+HD_POS_MSG2_OFFSET; i < Parser::cursor + HD_POS_MSG2_OFFSET + HD_POS_MSG2_LEN; i++){
        pre_pos.push_back(int((unsigned char)(Parser::buf[i])));
    }

    for (auto i = 0; i < 2; i++){
        pos.push_back(pre_pos[i] + (pre_pos[i+2] * 1e-2));
    }

    Parser::cursor += HD_POS_PAYLOAD_LEN;
    return pos;
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
    
    auto payload = (if_Euler) ? HD_EULER_PAYLOAD : HD_POS_PAYLOAD;

    for (auto i = Parser::cursor + msg1_offset; i < Parser::cursor +                          msg1_offset + msg1_len; i+=4){
       pre_result.push_back(int((unsigned char)(Parser::buf[i]) << 24 |
                             (unsigned char)(Parser::buf[i+1]) << 16 |
                             (unsigned char)(Parser::buf[i+2]) << 8 |
                             (unsigned char)(Parser::buf[i+3])));
     }

    for (auto i = Parser::cursor + msg2_offset; i < Parser::cursor + msg2_offset + msg2_len; i++){
        pre_result.push_back(int((unsigned char)(Parser::buf[i])));
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

    auto payload = (if_Euler) ? EULER_MSG_PAYLOAD : POS_MSG_PAYLOAD;

    for (auto i = Parser::cursor + msg_offset; i < Parser::cursor + msg_offset + msg_len;)
}


/*
Parser::Parser(std::string PortName = "/dev/ttyACM0"){
    Parser::file_descr = open(PortName, O_RDWR);
    
    if (Parser::file_descr < 0){
        std::cout << "Cannot open device at" << PortName << "\n Terminating...";
    }
    
}
*/

int main(){

    return 0;
}
