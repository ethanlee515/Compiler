#include "token.hpp"
#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;
using std::string;
using std::runtime_error;
using std::to_string;

const string token::KWSTR[]=
{"#", "(", ")", "*", "+", ",", "-", ".",
	":", ":=", ";", "<", "<=", "=", ">", ">=",
	"ARRAY", "BEGIN", "CONST", "DIV", "DO", "ELSE", "END",
	"IF", "MOD", "OF", "PROGRAM", "READ", "RECORD", "REPEAT",
	"THEN", "TYPE", "UNTIL", "VAR", "WHILE", "WRITE", "[", "]", "eof"};

token::operator string() const
{
	string r;
	switch (k)
	{
	case KEYWORD:
		r = KWSTR[v];
		break;
	case IDENTIFIER:
		r = ("identifier<" + id + ">");
		break;
	case INTEGER:
		r = "integer<" + to_string(v) + ">";
		break;
	default:
		throw runtime_error("Strange type of token detected.");
	}
	r+="@(" + to_string(start) + ", " + to_string(end) + ")";

	return r;
}

bool token::isKey(int key) const
{
	return k==KEYWORD && v == key;
}

token::token(kind k, int value, int start, int end) : k(k), v(value), start(start), end(end){}

token::token(string id, int start, int end) : k(IDENTIFIER), v(0), id(id), start(start), end(end){}

std::ostream& operator<<(std::ostream& ss, const token& t)
{
	ss << (string) t;
	return ss;
}

int strcpr(const void* s1, const void* s2)
{
	return ((string*)s1)->compare(*((string*)s2));
}

bool isKeyword(string s)
{
	return NULL != bsearch(&s, token::KWSTR, sizeof(token::KWSTR)/sizeof(string), sizeof(string), &strcpr);
}

int keyValue(string s)
{
	string* ptr = (string*) bsearch(&s, token::KWSTR,
			sizeof(token::KWSTR)/sizeof(string), sizeof(string), &strcpr);
	if(ptr == NULL)
		return -1;
	return ptr - token::KWSTR;
}

token INVALID_TOKEN()
{
	throw runtime_error("invalid character encountered in the source file");
	return PLACEHOLDER_TOKEN;
}

void debugPrint(string s)
{
	cout << s << endl;
}
