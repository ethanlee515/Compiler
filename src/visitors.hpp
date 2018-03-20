/*
 * visitors.hpp
 *
 *  Created on: 27 de mar. de 2017
 *      Author: Ethan
 */

#ifndef VISITORS_HPP_
#define VISITORS_HPP_

#include "AST.hpp"
#include "STable.hpp"

class a_visitor {
public:
	virtual void visit(Number* n) =0;
	virtual void visit(Variable* n) =0;
	virtual void visit(Index* n) =0;
	virtual void visit(Field* n) =0;
	virtual void visit(Binary* n) =0;
	virtual void visit(Condition* n) =0;
	virtual void visit(Assign* n) =0;
	virtual void visit(If* n) =0;
	virtual void visit(Repeat* n) =0;
	virtual void visit(Read* n) =0;
	virtual void visit(Write* n) =0;
	virtual ~a_visitor(){}
};

class s_visitor {
public:
	virtual void visit(Var* n) =0;
	virtual void visit(Integer* n) =0;
	virtual void visit(Constant* n) =0;
	virtual void visit(Array* n) =0;
	virtual void visit(Record* n) =0;
	virtual void visit(Invalid* n)=0;
	virtual ~s_visitor(){}
};

#endif /* VISITORS_HPP_ */
