#include <string>
#include <fstream>
#include <random>
#include "include/openssl/sha.h"

std::string ReadFile(const char* filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open())
    return "{ERROR_OPENING_FILE}";
  std::ostringstream sstr;
  sstr << file.rdbuf();
  return sstr.str();
}

std::string UrlEncode(std::string bytes) {
  static const char* base = "0123456789abcdef";
  std::string result;
  for (int i = 0; i < bytes.size(); i++) {
    unsigned char byte = bytes[i];
    result += '%';
    result += base[(byte/16)%16];
    result += base[byte%16];
  }
  return result;
}

std::string GetInfoHash(const std::string& infoDict) {
  std::string sha1HashBytes;
  sha1HashBytes.resize(SHA_DIGEST_LENGTH);
  SHA1((unsigned char*)infoDict.data(), infoDict.size(), (unsigned char*)sha1HashBytes.data());
  return sha1HashBytes;
}

struct PeerAddr {
  std::string ip;
  unsigned short port;
};
std::vector<PeerAddr> GetIpv4(std::string str) {
  std::vector<PeerAddr> result;
  for (int i = 0; i < str.size(); i += 6) {
    PeerAddr p;
    p.ip += std::to_string((unsigned char)str[i+0]) + '.';
    p.ip += std::to_string((unsigned char)str[i+1]) + '.';
    p.ip += std::to_string((unsigned char)str[i+2]) + '.';
    p.ip += std::to_string((unsigned char)str[i+3]);
    
    p.port = ((unsigned char)str[i+4] << 8) | (unsigned char)str[i+5];
    result.push_back(p);
  }
  return result;
}
std::vector<PeerAddr> GetIpv6(std::string str) {
  static const char* base = "0123456789abcdef";
  std::vector<PeerAddr> result;
  for (int i = 0; i < str.size(); i += 18) {
    PeerAddr p;
    for (int j = 0; j < 16; j++) {
      if (j && j % 2 == 0) p.ip += ":";
      unsigned char byte = str[i+j];
      p.ip += base[(byte/16)%16];
      p.ip += base[byte%16];
    }
    p.port = ((unsigned char)str[i+16] << 8) | (unsigned char)str[i+17];
    result.push_back(p);
  }
  return result;
}

std::string RandomString(int len) {
  static const char* alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static std::random_device dev;
  static std::mt19937 rng(dev());
  static std::uniform_int_distribution<std::mt19937::result_type> dist(0, 62);
  std::string result;
  for (int i = 0; i < len; i++)
    result += alphabet[dist(rng)];
  return result;
}

