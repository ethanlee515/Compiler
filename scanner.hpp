/*
 * scanner.hpp
 *
 *  Created on: 10 de feb. de 2017
 *      Author: Ethan
 */

#ifndef SCANNER_HPP_
#define SCANNER_HPP_
#include <vector>
#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include "token.hpp"

class scanner
{
public:
	scanner(std::string);
	token next();
	std::vector<token> all();
	void printAllTokens();
private:
	token nextID();
	token nextNum();
	token nextSymbol();
	void removeComment();
	void removeWhitespace();
	std::string file;
	bool isSpace(char c);
	unsigned int loc;
};

bool isValid(token t);


#endif /* SCANNER_HPP_ */
