#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

template<class T>
using Dict = std::unordered_map<std::string, T*>;
template<class T>
using List = std::vector<T*>;

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
	Node *root;
	Parser(std::string data)
		: data((char*)data.c_str()), dataLen(data.size()) {
		try {
			root = ParseDict();
		} catch (std::exception e) {
			std::cout << "Parsing failed.\n";
			success = false;
		}
	}
	~Parser() {
		FreeNode(root);
	}
	bool Success() { return success; }
private:
	char* data;
	int dataLen;
	int i = 0;
	bool success = true;

	Node* ParseNode() {
		Node *node = new Node();
		switch(data[i]) {
		case 'd': node = ParseDict(); break;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': node = ParseString(); break;
		case 'i': node = ParseInt(); break;
		case 'l': node = ParseList(); break;
		default:
			std::cout << "Unexpected character: '" << data[i] << "'.\n";
			throw std::exception();
			break;
		}
		return node;
	}
	Node* ParseString() {
		std::string lenStr;
		while (isdigit(data[i])) {
			lenStr += data[i++];
		}
		int len = std::stoi(lenStr);
		Expect(':');
		std::string value;
		for (int j = 0; j < len; j++)
			value += data[i++];
		Node *node = new Node();
		node->value = value;
		return node;
	}
	Node* ParseInt() {
		i++; // 'i'
		std::string numStr;
		while(data[i] != 'e')
			numStr += data[i++]; // TODO: Check for non-digit characters
		Expect('e');
		Node *node = new Node();
		node->value = std::stol(numStr);
		return node;
	}
	Node* ParseList() {
		i++; // 'l'
		Node *node = new Node();
		node->value = List<Node>();
		while (data[i] != 'e')
			node->Lst().push_back(ParseNode());
		Expect('e');
		return node;
	}
	Node* ParseDict() {
		i++; // 'd'
		Node *node = new Node();
		node->value = Dict<Node>();
		while (data[i] != 'e') {
			std::string s = ParseString()->Str();
			node->Dct()[s] = ParseNode();
		}
		Expect('e');
		return node;
	}
	bool End() { return i >= dataLen; }
	void Expect(char c) {
		if (End() || (data[i++] != c)) {
			std::cout << "Parser error: Expected '" << c << " " << data[i-1] << "'.\n";
			throw std::exception();
		}
	}

	void FreeNode(Node* node) {
		switch(node->Type()) {
		case DICT:
			for (auto& it: node->Dct())
				FreeNode(it.second);
			break;
		case LIST:
			for (auto& e: node->Lst())
				FreeNode(e);
		}
	}
};
