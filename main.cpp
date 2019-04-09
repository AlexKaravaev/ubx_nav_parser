#include "Parser.cpp"

int main(){

    Parser parser;
    parser.write_for_test("/dev/ttyACM0");
    parser.read_data(true);
    return 0;
}
