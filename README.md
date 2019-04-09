# Parser for UBX messages
Parser for NAV msgs in UBX protocol

![UBX message structure](img/ubx_msg.jpg)

### Usage
1. Change filename in main.cpp, from where information will come
2. Compile: `g++ -std=c++11 main-cpp -o main`
3. Run

### Adding new message type
For example, new message type will have id = MSG_ID

1. Define message id and message structure in Parser.hpp
2. Define new class attribute in Parser. `std::vector<long double> MSG_ID={};`
3. Add `case MSG_ID: ` in `Parser::parser_part(int id, bool if_verbose)` with the structure as in other messages
4. Add `case MSG_ID: ` in `Parser::update(std::vector<long double> result, char id, bool if_verbose)` with the structure as in other messages
5. Recompile

### Supported messages types

- [x] HPPOSECEF - High-Precision coordinates in ECEF system.
- [x] HPPOSLLH - High-Precision Langtitude and Longtitude.
- [x] POSLLH - Langtitude and Longtitude.
- [x] POSECEF - Coordinates in ECEF system.
- [x] ATT - Euler-angles.

| MSG_ID        | Class Attribute and MSG_STRUCTURE  |
| ------------- |:-------------:|
| HPPOSECEF     | Parser::HD_ECEF = {HP_ECEFX, HP_ECEFY, HP_ECEFZ} |
| HPPOSLLH      | Parser::HD_POS = {HP_LON, HP_LAT}                |
| POSECEF       | Parser::ECEF = {ECEFX, ECEFY, ECEFZ}             |
| POSLLH        | Parser::POS = {LON, LAT}                         |
| ATT           | Parser::EULER = {ROLL, PITCH, HEADING}           |
