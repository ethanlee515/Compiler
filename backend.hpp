#ifndef BACKEND_HPP_
#define BACKEND_HPP_

#include <string>
#include "visitors.hpp"
#include "AST.hpp"
#include "STable.hpp"
#include <map>
#include <sstream>

class AST;
class STable;
using std::string;

class backend : public a_visitor, public s_visitor{
public:
	backend(AST* ast, std::string fName);
	~backend(){};

	virtual void visit(Number* n) override;
	virtual void visit(Variable* n) override;
	virtual void visit(Index* n) override;
	virtual void visit(Field* n) override;
	virtual void visit(Binary* n) override;
	virtual void visit(Condition* n) override;
	virtual void visit(Assign* n) override;
	virtual void visit(If* n) override;
	virtual void visit(Repeat* n) override;
	virtual void visit(Read* n) override;
	virtual void visit(Write* n) override;
	virtual void visit(Var* n) override;
	virtual void visit(Integer* n) override;
	virtual void visit(Constant* n) override;
	virtual void visit(Array* n) override;
	virtual void visit(Record* n) override;
	virtual void visit(Invalid* n)override;

private:
	AST* ast;
	STable* symbols;
	std::string fName;
	int process(std::shared_ptr<Scope> s);
	void multiply(string reg, int i);
	std::map<Entry*, int> sizes; //type -> type size
	std::map<Entry*, int> arrLen; //Array type -> array length
	std::map<Entry*, int> offsets; //Variable -> variable offset
	Type* currentType = nullptr;
	std::stringstream ss;
	bool isNum = false;
	bool toR1 = false;
	std::string pop();
	std::string push();
	std::string borrow();
	std::string peek();
	void pool();
	int top = 4, warp = 0;
	int sum = 0;
	int lc = 1;
	int pc = 1;
	std::string cond;
	std::string condR;
};

#endif /* BACKEND_HPP_ */
