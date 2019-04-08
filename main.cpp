#include "Parser.cpp"

int main(){

    Parser parser;
    parser.write_for_test("testing.txt");
    parser.read_data(true);
    return 0;
}
