/*
  TODO:
  - make it async
  - download data from peer
  - support more types of torrents (different structures)
  - parser error handling: longer strings than should be
*/

#define ASIO_STANDALONE
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <iostream>
#include <string>
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
  std::string ipv4Blob = parser.root->Dct()["peers"]->Str();
  std::string ipv6Blob = parser.root->Dct()["peers6"]->Str();
  auto peersIpv4 = GetIpv4(ipv4Blob);
  auto peersIpv6 = GetIpv6(ipv6Blob);
  std::vector<Networking::Peer> peers;

  std::cout << "=== ipv4 peers: ===\n";
  for (int i = 0; i < peersIpv4.size(); i++) {
    std::cout << peersIpv4[i].ip << ":" << peersIpv4[i].port << "\n";
    peers.push_back(Networking::Peer(networking, peersIpv4[i].ip, peersIpv4[i].port));
  }
  std::cout << "=== ipv6 peers: ===\n";
  for (int i = 0; i < peersIpv6.size(); i++) {
    std::cout << peersIpv6[i].ip << "  " << peersIpv6[i].port << "\n";
    peers.push_back(Networking::Peer(networking, peersIpv6[i].ip, peersIpv6[i].port));
  }

  std::string handshake;
  handshake += 19; // Protocol identifier length
  handshake += "BitTorrent protocol"; // Protocol identifier
  handshake += std::string(8, 0); // Extension reserved bytes
  handshake += infoHash; // Info hash
  handshake += peerID;

  for (int i = 0; i < peers.size(); i++) {
  //for (int i = 0; i < 2; i++) {
    if (peers[i].Connect()) {
      response = peers[i].Send(handshake);
      std::string peerInfoHash(response.begin() + 28, response.begin() + 48);
  
      std::cout << "Peer #" << i << ": ";
      if (infoHash == peerInfoHash)
        std::cout << "Info hash matches.\n";
      else
        std::cout << "Info hash does not match.\n";
    }
  }  
  
  //for (int i = 0; i < peersIPs.size(); i++) {
  //  Networking::Endpoint endpoint = networking.GetEndpoint(peersIPs[i].ip, peersIPs[i].port);
  //}
}
