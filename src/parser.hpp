#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <tuple>
#include "token.hpp"
#include "p_obsv.hpp"
#include "AST.hpp"
#include "STable.hpp"

class parser {
public:
	parser(std::vector<token>);
	~parser() {
		delete symbols;
		delete ast;
	};
	void parse();
	void attach(p_obsv* o);
	bool good(){return lastError<0;}
	STable* symbols;
	AST* ast;

private:
	std::vector<token> toks;
	std::vector<p_obsv*> observers;
	std::string programName;
	int lastError;
	int loc;
	bool begun;
	void throwErr(std::string exp);
	void program();
	void declarations();
	void constDecl();
	void typeDecl();
	void varDecl();
	std::shared_ptr<Type> type();

	//First slot: the integer value, if no node is created.
		//Otherwise, the RAW pointer to the type, if a node is created.
	//Second slot: RAW pointer to the node, if one is created.
	std::pair<uintptr_t, uintptr_t> expression();
	std::pair<uintptr_t, uintptr_t> term();
	std::pair<uintptr_t, uintptr_t> factor();

	std::unique_ptr<Instruction> instructions();
	std::unique_ptr<Instruction> instruction();
	std::unique_ptr<Assign> assign();
	std::unique_ptr<If> ifAST();
	std::unique_ptr<Repeat> repeat();
	std::unique_ptr<If> whileAST();
	std::unique_ptr<Condition> condition();
	std::unique_ptr<Write> write();
	std::unique_ptr<Read> read();

	//First slot: the integer value, if no node is created.
		//Otherwise, the RAW pointer to the type, if a node is created.
	//Second slot: RAW pointer to the node, if one is created.
	std::pair<uintptr_t, uintptr_t> designator();

	//First slot: the integer value, if no node is created.
		//Otherwise, the RAW pointer to the type, if a node is created.
	//Second slot: RAW pointer to the node, if one is created.
	std::pair<uintptr_t, uintptr_t> eval(
			std::pair<uintptr_t, uintptr_t> lft,
			std::pair<uintptr_t, uintptr_t> rgt,
			keyword op);

	//First slot: the integer value, if no node is created.
		//Otherwise, the RAW pointer to the type, if a node is created.
	//Second slot: RAW pointer to the node, if one is created.
	std::pair<uintptr_t, uintptr_t> selector(std::string id);

	std::vector<std::string> identifierList();
	std::vector<
		std::pair<uintptr_t, uintptr_t>
				> expressionList();

	void populateScope(shared_ptr<Scope> sc);

	std::string matchID();
	void matchKW(int key);
	int matchInt();

	void notify_start(std::string prod);
	void notify_end();
	void notify_match(token t);

	keyword negate(keyword r);
};

int intCmpr(const void*, const void*);

#endif /* PARSER_HPP_ */
