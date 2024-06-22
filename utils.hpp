#include <string>
#include <fstream>
#include <random>

std::string ReadFile(const char* filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open())
    return "{ERROR_OPENING_FILE}";
  std::ostringstream sstr;
  sstr << file.rdbuf();
  return sstr.str();
}

std::string UrlEncode(unsigned char* bytes, int size) {
  static const char* base = "0123456789abcdef";
  std::string result;
  for (int i = 0; i < size; i++) {
    unsigned char byte = bytes[i];
    result += '%';
    result += base[(byte/16)%16];
    result += base[byte%16];
  }
  return result;
}

std::string GetInfoHash(const std::string& infoDict) {
	unsigned char sha1HashBytes[SHA_DIGEST_LENGTH];
	SHA1((unsigned char*)infoDict.data(), infoDict.size(), sha1HashBytes);
	return UrlEncode(sha1HashBytes, SHA_DIGEST_LENGTH);
}

struct Peer {
  std::string ip;
  unsigned short port;
};
std::vector<Peer> GetIPs(std::string str) {
  std::vector<Peer> result;
	for (int i = 0; i < str.size(); i += 6) {
    Peer p;
		p.ip += std::to_string((unsigned char)str[i+0]) + '.';
    p.ip += std::to_string((unsigned char)str[i+1]) + '.';
    p.ip += std::to_string((unsigned char)str[i+2]) + '.';
    p.ip += std::to_string((unsigned char)str[i+3]);
    p.port = ((unsigned int)(unsigned char)str[i+4] << 2) | (unsigned int)(unsigned char)str[i+5]; // TODO: how the fuck do we
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

