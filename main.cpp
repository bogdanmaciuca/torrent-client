/*
  TODO:
  - socket hangs when the it can't connect to the peer
  - parse tracker response: v6 IPs?
  - support more types of torrents (different structures)
  - parser error handling: longer strings than should be
*/

#define ASIO_STANDALONE
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <iostream>
#include <string>
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
    "?info_hash=" + UrlEncode(infoHash) +
    "&peer_id=" + peerID +
    "&port=" + std::to_string(port) +
    "&downloaded=0&compact=1" +
    "&left=" + std::to_string(piecesLen);

  Networking networking;
  Networking::Endpoint trackerEndpoint = networking.GetEndpoint(trackerUrl);
  std::string response;
  networking.MakeGetReq(trackerEndpoint, args, response);
  response = networking.GetBody(response);

  parser = Parser(response.data(), response.size());
  std::string ipBlob = parser.root->Dct()["peers"]->Str();
  auto peersIPs = GetIPs(ipBlob);
  for (int i = 0; i < peersIPs.size(); i++)
    std::cout << peersIPs[i].ip << ":" << peersIPs[i].port << "\n";

  std::string handshake;
  handshake += 19; // Protocol identifier length
  handshake += "BitTorrent protocol"; // Protocol identifier
  handshake += std::string(8, 0); // Extension reserved bytes
  handshake += infoHash; // Info hash
  handshake += peerID;

  for (int i = 0; i < peersIPs.size(); i++) {
    Networking::Peer peer(networking, peersIPs[0].ip, peersIPs[0].port);
    response = peer.Send(handshake);
    //std::cout << UrlEncode(handshake) << "\n";
    //std::cout << UrlEncode(response) << "\n";
    std::string peerInfoHash(response.begin() + 28, response.begin() + 48);

    std::cout << "Peer #" << i << ": ";
    if (infoHash == peerInfoHash)
      std::cout << "Info hash matches.\n";
    else
      std::cout << "Info hash does not match.\n";
    }
  

  //for (int i = 0; i < peersIPs.size(); i++) {
  //  Networking::Endpoint endpoint = networking.GetEndpoint(peersIPs[i].ip, peersIPs[i].port);
  //}
}
