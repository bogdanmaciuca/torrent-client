/*
TODO:
- conversion operators
- [] operator
- error handling
*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "parser.hpp"
#include "request.hpp"

std::string ReadFile(const char* filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
		return "{ERROR_OPENING_FILE}";
	std::ostringstream sstr;
    sstr << file.rdbuf();
    return sstr.str();
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Expected an argument.\n";
		return -1;
	}
	std::string contents = ReadFile(argv[1]);
	
	Node tree;
	Parser parser(tree, contents);
	std::cout << tree.Dct()["url-list"]->Lst()[0]->Str();
	CurlWrapper curl;

}
