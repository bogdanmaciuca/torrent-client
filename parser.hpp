#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <variant>

template<class T>
using Dict = std::unordered_map<std::string, std::unique_ptr<T>>;
template<class T>
using List = std::vector<std::unique_ptr<T>>;

enum NodeType {
	NUMBER, STRING, DICT, LIST
};
struct Node {
	std::variant<long int, std::string, Dict<Node>, List<Node>> value;
	long int& Num() { return std::get<NUMBER>(value); }
	std::string& Str() { return std::get<STRING>(value); }
	Dict<Node>& Dct() { return std::get<DICT>(value); }
	List<Node>& Lst() { return std::get<LIST>(value); }
	NodeType Type() { return (NodeType)value.index(); }
};

class Parser {
public:
	Parser(Node &tree, std::string data)
		: data((char*)data.c_str()), dataLen(data.size()) {
		tree = ParseDict();
	}
private:
	char* data;
	int dataLen;
	int i = 0;

	Node ParseNode() {
		Node node;
		switch(data[i]) {
		case 'd': node = ParseDict(); break;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': node = ParseString(); break;
		case 'i': node = ParseInt(); break;
		case 'l': node = ParseList(); break;
		}
		return node;
	}
	Node ParseString() {
		std::string lenStr;
		while (isdigit(data[i])) {
			lenStr += data[i++];
		}
		int len = std::stoi(lenStr);
		i++; // ':'
		std::string value;
		for (int j = 0; j < len; j++)
			value += data[i++];
		Node node;
		node.value = value;
		return node;
	}
	Node ParseInt() {
		i++; // 'i'
		std::string numStr;
		while(data[i] != 'e')
			numStr += data[i++]; // TODO: Check for non-digit characters
		i++; // 'e'
		Node node;
		node.value = std::stol(numStr);
		return node;
	}
	Node ParseList() {
		i++; // 'l'
		Node node;
		node.value = List<Node>();
		while (data[i] != 'e')
			node.Lst().push_back(std::make_unique<Node>(ParseNode()));
		i++; // 'e'
		return node;
	}
	Node ParseDict() {
		i++; // 'd'
		Node node;
		node.value = Dict<Node>();
		while (data[i] != 'e') {
			std::string s = ParseString().Str();
			node.Dct()[s] = std::make_unique<Node>(ParseNode());
		}
		i++; // 'e'
		return node;
	}
	bool End() { return i >= dataLen; }
};
