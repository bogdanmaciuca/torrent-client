
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
#include "include/asio.hpp"
#include "include/asio/ts/buffer.hpp"
#include "include/asio/ts/internet.hpp"
#include "parser.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Expected an argument.\n";
    return -1;
  }
  std::string contents = ReadFile(argv[1]);

  Parser parser(contents.c_str(), contents.size());
  if (!parser.Success()) return -1;
	
  std::string baseUrl = ToStr(parser.root->Dct()["announce"]->Str());
	std::cout << baseUrl << "\n";
	exit(0);
  String infoDict = parser.root->Dct()["info"]->Print();
  std::string infoHash = GetInfoHash(infoDict);
  std::string peerID = RandomString(20);
  int port = 6883; // TODO
  long long piecesLen = parser.root->Dct()["info"]->Dct()["length"]->Num();

  std::string url = baseUrl +
    "?info_hash=" + infoHash +
    "&peer_id=" + peerID +
    "&port=" + std::to_string(port) +
    "&downloaded=0&compact=1" +
    "&left=" + std::to_string(piecesLen);

  asio::error_code asioErr;
  asio::io_context asioContext;
  asio::io_service ioService;
  asio::ip::tcp::resolver resolver(ioService);
  asio::ip::tcp::resolver::query query("", "80");
  asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);

  asio::ip::tcp::endpoint endpoint = iter->endpoint();
  asio::ip::tcp::socket socket(asioContext);

  socket.connect(endpoint, asioErr);

  if (asioErr)
    std::cout << "Connection failed: " << asioErr.message();
  
  if (socket.is_open()) {
    std::string request =
      "GET /index.html HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";
    socket.write_some(asio::buffer(request.data(), request.size()), asioErr);
    socket.wait(socket.wait_read);
    int bytes = socket.available();
    if (bytes > 0) {
      List<char> buffer(bytes);
      socket.read_some(asio::buffer(buffer.data(), buffer.size()), asioErr);
      std::cout << buffer.data() << "\n";
    }
  }
}
