
/*
  TODO:
  - use ASIO instead of libcurl:
	  - parse url to get domain and port
	- refactor the while string/basic string thing
  - support more types of torrents (different structures)
  - parser error handling: longer strings than should be
*/

#define ASIO_STANDALONE
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include "include/openssl/sha.h"
#include "parser.hpp"
#include "networking.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Expected an argument.\n";
    return -1;
  }
  std::string contents = ReadFile(argv[1]);

  Parser parser(contents.c_str(), contents.size());
  if (!parser.Success()) return -1;
	
  std::string trackerUrl = parser.root->Dct()["announce"]->Str();
	std::string infoDict = parser.root->Dct()["info"]->Print();
  std::string infoHash = GetInfoHash(infoDict);
  std::string peerID = RandomString(20);
  int port = 6883; // TODO
  long long piecesLen = parser.root->Dct()["info"]->Dct()["length"]->Num();
	
  std::string args =
    "?info_hash=" + infoHash +
    "&peer_id=" + peerID +
    "&port=" + std::to_string(port) +
    "&downloaded=0&compact=1" +
    "&left=" + std::to_string(piecesLen);

	Networking networking;
	Networking::Endpoint endpoint = networking.GetEndpoint(trackerUrl);
	std::vector<char> response;
	networking.MakeGetReq(endpoint, args, response);

	for (int i = 0; i < response.size(); i++)
		std::cout << response[i];
	std::cout << "\n";
}
