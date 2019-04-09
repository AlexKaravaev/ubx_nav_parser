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


    //std::cout << rbuf[0];
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
          if(verbose){
            std::cout << "-----------------------------------\n";
            std::cout <<"Msg class: " << (int)msg_class << " \n";
            std::cout <<"id class: " << (int)id << " \n";
            std::cout <<"length: " << (int)length << " \nPayload: ";
          }
          Parser::parse_msg(id, verbose);
          if(verbose){
              std::cout << "\n-----------------------------------\n";
              std::cout << "\n";
            }
        }
        else
            std::cout << "\nerror in checksum validation\n" << std::endl;

    }
}

long double Parser::parse_4_byte(int pos){
  return (long double)((signed char)(msg[pos]) << 24 |
                           (signed char)(msg[pos+1]) << 16 |
                           (signed char)(msg[pos+2]) << 8 |
                           (signed char)(msg[pos+3]));
}

long double Parser::parse_part(int pos, int bytes_long){
  return (bytes_long==4) ? parse_4_byte(pos) : int((signed char)(msg[pos]));
}

// Generic func to parse hd message
void Parser::parse_msg(int id, bool if_verbose){
    std::vector<long double> result;

    // push val in vector according to what part of the msg you need
    // EX: part1 - from 0 byte to 4 byte 4 byte long -> msg_len_offset_bytes.push_back({0, 4, 4})
    std::vector< std::vector <int> > msg_len_offset_bytes;
    bool if_HP;
    int offset;
    long double scale;

    switch(id){
      case HPPOSECEF:

        {
          msg_len_offset_bytes.push_back(std::vector<int> {HD_ECEF_MSG1_OFFSET,HD_ECEF_MSG1_LEN, 4});
          msg_len_offset_bytes.push_back(std::vector<int> {HD_ECEF_MSG2_OFFSET,HD_ECEF_MSG2_LEN, 1});
          offset = 3;

          scale = 1;
          if_HP = true;
          break;
        }
      case HPPOSLLH:
      {

        msg_len_offset_bytes.push_back(std::vector<int> {HD_POS_MSG1_OFFSET,HD_POS_MSG1_LEN, 4});
        msg_len_offset_bytes.push_back(std::vector<int> {HD_POS_MSG2_OFFSET,HD_POS_MSG2_LEN, 1});
        offset = 2;

        scale = 1e-7;
        if_HP = true;
        break;
      }
      case POSLLH:
      {
        msg_len_offset_bytes.push_back(std::vector<int> {POS_MSG_OFFSET,POS_MSG_LEN, 4});

        scale = 1e-7;
        if_HP = false;
        break;
      }
      case POSECEF:
      {
        msg_len_offset_bytes.push_back(std::vector<int> {ECEF_MSG_OFFSET,ECEF_MSG_LEN, 4});

        scale = 1;
        if_HP = false;
        break;
      }
    }

    for (auto const& msg_part: msg_len_offset_bytes){
      for (auto i = msg_part[0] + MSG_LEN; i <  msg_part[0] + MSG_LEN + msg_part[1]; i+=msg_part[2]){
         result.push_back(parse_part(i, msg_part[2]));
       }

    }

    if(if_HP){
      std::vector<long double> temp;
      for (auto i = 0; i < offset; i++){
          temp.push_back(result[i]*scale + (result[i+offset] * 0.01));

      }
      update(temp, id, if_verbose);
    }
    else{
      update(result, id, if_verbose);
    }
}

void Parser::update(std::vector<long double> result, char id, bool if_verbose){
  if(if_verbose){
    for(auto const& val: result){
      std::cout << val << " ";
    }
  }
  switch (id){
    case POSLLH:
      POS = result;
      break;
    case POSECEF:
      ECEF = result;
      break;
    case HPPOSLLH:
      HP_POS = result;
      break;
    case HPPOSECEF:
      HP_ECEF = result;
      break;
  }
}


Parser::Parser(std::string filename = "testing.txt"){


    test_fd = open(filename.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(test_fd < 0){
      std::cout << "Error while opening file\n";
    }
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
