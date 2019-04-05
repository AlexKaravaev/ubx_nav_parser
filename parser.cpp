#include <cmath>
#include <iostream>
#include <vector>

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
#define POS_MSG2_LEN 8

#define EULER_PAYLOAD_LEN 20
#define EULER_MSG1_OFFSET 4
#define EULER_MSG1_LEN 12

class Parser{
    private: 
        struct NAV{

        }

        std::string portname;
        int file_descr, nRead, msgcount;
        char buf[1024];
        int cursor;

    public:
        Parser(std::string PortName);
        void read_data(char* buf, int fd);
        void parse_HD_EULER(char* buf, int cursor);
        void parse_HD_POS(char* buf, int cursor);
        void parse_EULER(char* buf, int cursor);
        void parse_POS(char* buf, int cursor);
}


void Parser::read_data(char* buf, int fd){
    int cursor;
    
    //flags for catching sync bytes
    auto FIRST_SYNC = false, SEC_SYNC = false;

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
    
    auto i = Parser::cursor + HD_EULER_OFFSET;

    //TODO: dafuck don't know how to read bytes
    
    for(auto i = Parser::cursor + HD_EULER_OFFSET; i < Parser::cursor + HD_EULER_MSG1_LEN; i+=4){
    
       pre_ece.push_back(int((unsigned char)(Parser::buf[i]) << 24 |
                (unsigned char)(Parser::buf[i+1]) << 16 |
                (unsigned char)(Parser::buf[i+2]) << 8 |
                (unsigned char)(Parser::buf[i+3])));
    }

    //TODO
    for(auto i = Parser::cursor + HD_EULER_OFFSET + HD_EULER_MSG1_LEN;
            i < Parser::cursor + HD_EULER_MSG2_LEN; i++){
                pre_ece.push_back(int((unsigned char)(Parser::buf[i]) << 24 |
                                      (unsigned char)(Parser::buf[i+1]) << 16 |
                                      (unsigned char)(Parser::buf[i+2]) << 8 |
                                      (unsigned char)(Parser::buf[i+3])));

    }

    for (auto i = 0; i < 3; i+=1){
        ece.push_back(pre_ece[i] + (pre_ece[i+3] * 1e-2))
    }

    Parser::cursor += HD_EULER_PAYLOAD_LEN;
    return ece
}

std::vector<int> Parser::parse_HD_POS(){
    std::vector<int> pre_pos;

    std::vactor<int> pos;

    for (auto i = Parser::cursor + HD_POS_OFFSET; i < Parser::cursor + HD_POS_MSG_LEN; i+=;)
}

/*
Parser::Parser(std::string PortName = "/dev/ttyACM0"){
    Parser::file_descr = open(PortName, O_RDWR);
    
    if (Parser::file_descr < 0){
        std::cout << "Cannot open device at" << PortName << "\n Terminating...";
    }
    
}
*/
