#include <cmath>
#include <iostream>
#define SYNC1 0xB5
#define SYNC2 0x62 
#define BAUDRATE B460800
#define NAV_CLASS 0x01

#define HPPOSECEF 0x13 // Msg class with high-precision euler angles
#define HPPOSLLH 0x14 // Msg class with high-precision position
#define POSECEF 0x01// Msg class with usual euler angles
#define POSLLH 0x02 // Msg class with usual position


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
                                std::cout << "MOCK";
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

void Parser::parse_HD_EULER(){
    int* ecefX, ecefY, ecefZ;

    for (auto i = Parser::cursor + HD_EULER_OFFSET; i < Parser::nRead; i++){
        
    }
    
}
/*
Parser::Parser(std::string PortName = "/dev/ttyACM0"){
    Parser::file_descr = open(PortName, O_RDWR);
    
    if (Parser::file_descr < 0){
        std::cout << "Cannot open device at" << PortName << "\n Terminating...";
    }
    
}
*/
