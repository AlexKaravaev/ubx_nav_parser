#include "Parser.hpp"

unsigned char Parser::getbyte(){
    if ((rp - rbuf) >= bufcnt){

        bufcnt = read(test_fd, rbuf, BLEN);

        if (bufcnt <= 0){
            //std::cout << "Error in getbyte()\n";
            ;
        }
        else{
        }
        rp = rbuf;
    }


    std::cout << rbuf[0];
    return *rp++;
}

void Parser::read_data(bool verbose = false){

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
        checka = 0;
        checkb = 0;

        msg_class = getbyte();
        id = getbyte();
        length = getbyte();
        length += getbyte() << 8;

        // Fill checksum
        checka += msg_class;
        checkb += checka;

        checka += id;
        checkb += checka;

        checka += length;
        checkb += checka;

        if (length > MLEN){
            continue;
        }
        for (auto i = 0; i < length; i++){
            msg[i] = getbyte();
            checka += msg[i];
            checkb += checka;
        }

        checka = checka & 0xFF;
        checkb = checkb & 0xFF;

        chka = getbyte();
        chlb = getbyte();
        //TODO: When real checksum used , wipe off true below
        if (true || (chka == checka & chlb == checkb)){
            switch(id){
                case HPPOSECEF:
                    res = Parser::parse_HD(true);
                    break;
                case HPPOSLLH:
                    res = Parser::parse_HD(false);
                    break;
                case POSECEF:
                    res = Parser::parse(true);
                    break;
                case POSLLH:
                    res = Parser::parse(false);
                    break;
            }
            if(verbose){
              std::cout << "-----------------------------------\n";
              std::cout <<"Msg class: " << (int)msg_class << " \n";
              std::cout <<"id class: " << (int)id << " \n";
              std::cout <<"length: " << (int)length << " \nPayload: ";
              for(auto const& val: res){
                std::cout << val << " ";
              }
              std::cout << "\n-----------------------------------\n";
              std::cout << "\n";
            }
        }
        else
            std::cout << "\nerror in checksum validation\n" << std::endl;

    }
}


// Generic func to parse hd message
std::vector<long double> Parser::parse_HD(bool if_Euler){
    std::vector<int> pre_result;
    std::vector<long double> result;

    auto msg1_offset = (if_Euler) ? HD_EULER_MSG1_OFFSET : HD_POS_MSG1_OFFSET;

    auto msg1_len = (if_Euler) ? HD_EULER_MSG1_LEN : HD_POS_MSG1_LEN;

    auto msg2_offset = (if_Euler) ? HD_EULER_MSG2_OFFSET : HD_POS_MSG2_OFFSET;

    auto msg2_len = (if_Euler) ? HD_EULER_MSG2_LEN : HD_POS_MSG2_LEN;

    msg1_offset += MSG_LEN;
    msg2_offset += MSG_LEN;

    auto offset = (if_Euler) ? 3 : 2;

    auto payload = (if_Euler) ? HD_EULER_MSG_PAYLOAD_LEN : HD_POS_MSG_PAYLOAD_LEN;

    auto scale = (if_Euler) ? 1 : 1e-7;

    for (auto i = msg1_offset; i <  msg1_offset + msg1_len; i+=4){
       pre_result.push_back((long double)((signed char)(msg[i]) << 24 |
                                (signed char)(msg[i+1]) << 16 |
                                (signed char)(msg[i+2]) << 8 |
                                (signed char)(msg[i+3])));
     }

    for (auto i = msg2_offset; i < msg2_offset + msg2_len; i++){
        pre_result.push_back(int((signed char)(msg[i])));
     }

    for (auto i = 0; i < offset; i++){
        result.push_back(pre_result[i]*scale + (pre_result[i+offset] * 0.01));
    }


    return result;
}

std::vector<long double> Parser::parse(bool if_Euler){
    std::vector<long double> result;

    auto msg_offset = (if_Euler) ? EULER_MSG_OFFSET : POS_MSG_OFFSET;

    auto msg_len = (if_Euler) ? EULER_MSG_LEN : POS_MSG_LEN;

    auto payload = (if_Euler) ? EULER_MSG_PAYLOAD_LEN : POS_MSG_PAYLOAD_LEN;

    msg_offset += MSG_LEN;

    auto scale = (if_Euler) ? 1 : 1e-7;

    for (auto i = msg_offset; i < msg_offset + msg_len; i+=4){
        result.push_back(((signed char)(msg[i]) << 24 |
                             (signed char)(msg[i+1]) << 16 |
                             (signed char)(msg[i+2]) << 8 |
                             (signed char)(msg[i+3])) * scale);
    }

    return result;
}



Parser::Parser(std::string filename = "testing.txt"){

    test_fd = open(filename.c_str(), O_RDWR);

}

void fill_data(char msg_class, char msg_id, char msg_len, std::vector<int> &data){

  data.push_back(SYNC1);
  data.push_back(SYNC2);
  data.push_back(msg_class);
  data.push_back(msg_id);
  data.push_back(msg_len & 0x00FF);
  data.push_back((msg_len & 0xFF00) >> 8);
  if(msg_id == POSLLH || msg_id == POSECEF){
    for (auto i=0; i < msg_len + CHECKSUM_LEN; i++){
        data.push_back(0x01);
    }
  }
  else{
    for (auto i=0; i < msg_len + CHECKSUM_LEN; i++){
      data.push_back(0x01);

    }
}
}

void Parser::write_for_test(std::string filename = "testing.txt"){
    Parser::test_fd = open(filename.c_str(), O_RDWR);

    std::vector<int> data = {0x00};

    fill_data(NAV_CLASS, POSLLH, POS_MSG_PAYLOAD_LEN, data);

    // JUNK
    data.push_back(0x00);
    data.push_back(0x23);

    fill_data(NAV_CLASS, POSLLH, POS_MSG_PAYLOAD_LEN, data);

    fill_data(NAV_CLASS, HPPOSLLH, HD_POS_MSG_PAYLOAD_LEN, data);

    std::ofstream of(filename);

    std::ostream_iterator<char> out_iterator(of, "");

    std::copy(data.begin(), data.end(), out_iterator);
}
