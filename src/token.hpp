/*
 * token.hpp
 *
 *  Created on: 10 de feb. de 2017
 *      Author: Ethan
 */

#ifndef TOKEN_HPP_
#define TOKEN_HPP_

#include <string>

bool isKeyword(std::string s);
int keyValue(std::string s);

enum keyword {
		HASH, OPEN_PAREN, CLOSE_PAREN, STAR, PLUS, COMMA, MINUS, DOT,
		COLON, COLON_EQ, SEMICOLON, LESS, LESS_EQ, EQUALS, GREATER, GREATER_EQ,

		ARRAY, BEGIN, CONST, DIV, DO, ELSE, END, IF, MOD,
		OF, PROGRAM, READ, RECORD, REPEAT, THEN, TYPE, UNTIL, VAR, WHILE,
		WRITE, OPEN_SQUARE, CLOSE_SQUARE, EOF_TOKEN
	};

struct token {
	enum kind {
		KEYWORD, IDENTIFIER, INTEGER
	};

	static const std::string KWSTR[];
	kind k;
	int v; //integer value or keyword value
	std::string id; //identifier
	int start, end;

	friend std::ostream& operator<<(std::ostream&, const token&);

	token(std::string id, int start, int end);
	token(kind k, int v, int start, int end);
	bool isKey(int key) const;
	operator std::string() const;
};
token INVALID_TOKEN();
const token PLACEHOLDER_TOKEN = token(token::KEYWORD, 0, 0, 0);

void debugPrint(std::string);

#endif /* TOKEN_HPP_ */
