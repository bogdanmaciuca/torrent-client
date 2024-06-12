/*
TODO:
- parser error handling: longer strings than should be
*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include "include/openssl/sha.h"
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

std::string UrlEncode(uchar* bytes, int size) {
	static const char* base = "0123456789abcdef";
	std::string result;
	for (int i = 0; i < size; i++) {
		uchar byte = bytes[i];
		result += '%';
		result += base[(byte/16)%16];
		result += base[byte%16];
	}
	return result;
}

std::string GetInfoHash(const String& infoDict) {
	uchar sha1HashBytes[SHA_DIGEST_LENGTH];
	SHA1(infoDict.data(), infoDict.size(), sha1HashBytes);
	return UrlEncode(sha1HashBytes, SHA_DIGEST_LENGTH);
}

List<std::string> GetIPs(String str) {
	List<std::string> result;
	for (int i = 0; i < str.size(); i += 6) {
		std::string ip;
		ip += std::to_string(str[i+0]) + '.';
		ip += std::to_string(str[i+1]) + '.';
		ip += std::to_string(str[i+2]) + '.';
		ip += std::to_string(str[i+3]) + ':';
		int port = 0;
		port = ((unsigned int)str[i+4] << 2) | str[i+5]; // TODO: how the fuck do we
		ip += std::to_string(port);
		result.push_back(ip);
		std::cout << ip << "\n";
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

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Expected an argument.\n";
		return -1;
	}
	std::string contents = ReadFile(argv[1]);

	Parser parser(contents.c_str(), contents.size());
	if (!parser.Success()) return -1;

	std::string baseUrl = ToStr(parser.root->Dct()["announce"]->Str());
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

	CurlWrapper curl;
	curl.MakeRequest(url.c_str());

	parser = Parser(curl.GetData().memory, curl.GetData().size);
	auto ipList = GetIPs(parser.root->Dct()["peers"]->Str());
}
