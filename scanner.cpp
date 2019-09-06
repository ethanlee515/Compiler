#include "scanner.hpp"
#include "token.hpp"
#include <exception>
#include <string>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

using std::string;
using std::vector;
using std::endl;
using std::cout;
using std::stringstream;
using std::runtime_error;
using std::stoi;
using std::to_string;

scanner::scanner(string s) : file(s), loc(0){}

vector<token> scanner::all()
{
	if(loc != 0)
		throw runtime_error("\"next\" called in scanner before \"all\" is called.");

	vector<token> v;
	token t = PLACEHOLDER_TOKEN;

	while(true)
	{
		t = next();
		v.push_back(t);

		if(t.isKey(EOF_TOKEN))
			break;
	}

	return v;
}

void scanner::removeWhitespace()
{
	while(loc < file.length() && isSpace(file[loc]))
		++loc;
}

token scanner::next()
{
	removeWhitespace();

	if(loc >= file.length())
		return token(token::KEYWORD, EOF_TOKEN, loc, loc);

	if(isalpha(file[loc]))
		return nextID();

	if(isdigit(file[loc]))
		return nextNum();

	return nextSymbol();
}

token scanner::nextID()
{
	int begin = loc;
	string s;
	s.push_back(file[loc]);

	loc++;
	while(loc < file.length() && isalnum(file[loc]))
	{
		s+=file[loc];
		++loc;
	}

	int v = keyValue(s);
	if(v == -1 || v == EOF_TOKEN)
		return token(s, begin, loc-1);
	else
		return token(token::KEYWORD, v, begin, loc-1);
}

token scanner::nextNum()
{
	int begin = loc;
	string s;
	s.push_back(file[loc]);

	loc++;
	while(loc < file.length() && isdigit(file[loc]))
	{
		s+=file[loc];
		++loc;
	}
	int i;
	try
	{
		i = stoi(s);
	}
	catch (std::exception& e)
	{
		throw runtime_error("Integer out of range");
	}	

	return token(token::INTEGER, i, begin, loc-1);
}

token scanner::nextSymbol()
{
	if(loc >= file.length())
		return token(token::KEYWORD, EOF_TOKEN, loc, loc);
	char c = file[loc];
	if(c=='<')
	{
		if(loc+1 < file.length() && file[loc+1] == '=')
		{
			token t = token(token::KEYWORD, LESS_EQ, loc, loc+1);
			loc+=2;
			return t;
		}
		else
		{
			token t = token(token::KEYWORD, LESS, loc, loc);
			loc++;
			return t;
		}
	}
	if(c=='>')
	{
		if(loc+1 < file.length() && file[loc+1] == '=')
		{
			token t = token(token::KEYWORD, GREATER_EQ, loc, loc+1);
			loc+=2;
			return t;
		}
		else
		{
			token t = token(token::KEYWORD, GREATER, loc, loc);
			loc++;
			return t;
		}
	}
	if(c==':')
	{
		if(loc+1 < file.length() && file[loc+1] == '=')
		{
			token t = token(token::KEYWORD, COLON_EQ, loc, loc+1);
			loc+=2;
			return t;
		}
		else
		{
			token t = token(token::KEYWORD, COLON, loc, loc);
			loc++;
			return t;
		}
	}
	if(c=='(')
	{
		if(file[loc+1] == '*')
		{
			loc += 2;
			removeComment();
			return next();
		}
		else
		{
			token t = token(token::KEYWORD, OPEN_PAREN, loc, loc);
			loc++;
			return t;
		}
	}

	loc++;
	int key = keyValue(string(1, c));
	if(key != -1)
		return token(token::KEYWORD, key, loc-1, loc-1);

	return INVALID_TOKEN();
}

bool scanner::isSpace(char c)
{
	return c==' ' || c=='\t' || c=='\n' || c=='\f' || c=='\r';
}

void scanner::printAllTokens()
{
	token t = PLACEHOLDER_TOKEN;
	while(!t.isKey(EOF_TOKEN))
	{
		t = next();
		std::cout << t << std::endl;
	}
}

void scanner::removeComment()
{
	int depth = 1;

	while(depth > 0 && loc+1 < file.length())
	{
		if(file[loc] == '*' && file[loc+1]==')')
		{
			loc++;
			depth--;
		}
		else if(file[loc] == '(' && file[loc+1]=='*')
		{
			loc++;
			depth++;
		}
		loc++;
	}
}
