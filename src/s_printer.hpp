/*
 * s_printer.hpp
 *
 *  Created on: 27 de mar. de 2017
 *      Author: Ethan
 */

#ifndef S_PRINTER_HPP_
#define S_PRINTER_HPP_

#include "STable.hpp"
#include "visitors.hpp"
#include <iostream>

void printI(std::string, int, std::ostream& ss);

class s_printer : public s_visitor
{
public:
	s_printer(){};
	virtual void print(STable* st);
	virtual void print(Scope* s);
	virtual void visit(Var* n) override;
	virtual void visit(Integer* n) override;
	virtual void visit(Constant* n) override;
	virtual void visit(Array* n) override;
	virtual void visit(Record* n) override;
	virtual void visit(Invalid* n)override;
	~s_printer(){}
protected:
	void printScope(Scope* s);
	void print(std::string s) {printI(s, ind, std::cout);};
	int ind = 0;
};

class a_printer : public s_printer, public a_visitor
{
public:
	void visit(If* n);
	void visit(Binary* n);
	void visit(Repeat* n);
	void visit(Assign* n);
	void visit(Read* n);
	void visit(Write* n);
	void visit(Condition* n);
	void visit(Variable* n);
	void visit(Number* n);
	void visit(Index* n);
	void visit(Field* n);
};

#endif /* S_PRINTER_HPP_ */
