/*
 * AST.hpp
 *
 *  Created on: Mar 6, 2017
 *      Author: root
 */

#ifndef AST_HPP_
#define AST_HPP_

#include "p_obsv.hpp"
#include "STable.hpp"
#include <memory>
#include <queue>
#include <tuple>

using std::unique_ptr;
using std::shared_ptr;

class a_visitor;

struct ASTNode
{
	virtual ~ASTNode() {}
	virtual void accept(a_visitor* p)=0;
};

struct Instruction : public ASTNode
{
	unique_ptr<Instruction> next;
};

struct Expression : public ASTNode {};

struct Number : public Expression{
	shared_ptr<Constant> num;
	Number(int i)
	{
		num = std::make_shared<Constant>(-1, -1, i);
	}
	~Number(){}
	int v(){return num->value;}
	virtual void accept(a_visitor*) override;
};

struct Location : public Expression {};

struct Variable : public Location{
	shared_ptr<Var> var;
	virtual void accept(a_visitor*) override;
};

struct Index : public Location{
	int start, end;
	unique_ptr<Location> loc;
	unique_ptr<Expression> expr;

	virtual void accept(a_visitor*) override;
};

struct Field : public Location{

	unique_ptr<Location> loc;
	unique_ptr<Variable> var;

	virtual void accept(a_visitor*) override;
};

struct Binary : public Expression {
	keyword op;
	unique_ptr<Expression> left, right;

	virtual void accept(a_visitor*) override;
};

struct Condition : public ASTNode
{
	keyword r;
	shared_ptr<Expression> left, right;

	virtual void accept(a_visitor*) override;
};

struct Assign : public Instruction{
	unique_ptr<Location> loc;
	unique_ptr<Expression> expr;

	virtual void accept(a_visitor*) override;
};

struct If : public Instruction{
	unique_ptr<Condition> cond;
	unique_ptr<Instruction> inst_t, inst_f;

	virtual void accept(a_visitor*) override;
};

struct Repeat : public Instruction{
	unique_ptr<Condition> cond;
	unique_ptr<Instruction> inst;

	virtual void accept(a_visitor*) override;
};

struct Read : public Instruction{
	unique_ptr<Location> loc;

	virtual void accept(a_visitor*) override;
};

struct Write : public Instruction{
	unique_ptr<Expression> expr;

	virtual void accept(a_visitor*) override;
};

struct AST
{
	unique_ptr<Instruction> root;
	STable* symbols;
};

#endif
