/*
 * STable.hpp
 *
 *  Created on: 27 de mar. de 2017
 *      Author: Ethan
 */

#ifndef STABLE_HPP_
#define STABLE_HPP_

#include <stdexcept>
#include <string>
#include <iostream>
#include "token.hpp"

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <memory>

using std::runtime_error;
using std::shared_ptr;
using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;

struct parser_error : public std::runtime_error {
	parser_error(std::string s) : std::runtime_error(s) {}
};

struct AST_error : public parser_error
{
	AST_error(std::string s) : parser_error(s) {}
};

struct ST_error : public AST_error
{
	ST_error(std::string s) : AST_error(s) {}
};

class s_visitor;

struct Entry {
	int start, end;
	Entry(int start, int end)
		: start(start), end(end) {}
	virtual void accept(s_visitor*) = 0;
	virtual ~Entry(){};
};

struct Type : public Entry {
	Type(int start, int end)
		: Entry(start, end){}
	virtual Type* apply() =0;
	virtual Type* apply(std::string) =0;
};

struct Var : public Entry {
	Var(int start, int end, shared_ptr<Type> type)
		: Entry(start, end), type(type){}
	shared_ptr<Type> type;
	void accept(s_visitor*) override;
};

struct Integer : public Type {
public:
	Type* apply()
		{throw ST_error("Applying index to Int");}

	Type* apply(std::string)
		{throw ST_error("Applying field to Int");}

	static shared_ptr<Integer> instance()
	{
		static Integer inst;
		static shared_ptr<Integer> ptr
			= shared_ptr<Integer>(&inst);
		return ptr;
	}
	Integer(Integer const&) = delete;
	void operator=(Integer const&) = delete;
	void accept(s_visitor*) override;
	~Integer(){}

private:
	Integer() : Type(-1, -1){}

};

struct Constant : virtual Entry{
	Constant(int start, int end, int value)
		: Entry(start, end), type(Integer::instance()), value(value){}
	shared_ptr<Type> type;
	int value;
	void accept(s_visitor*) override;
};

struct Array : Type{
	Array(int start, int end, shared_ptr<Type> type, int len) :
		Type(start, end), type(type), len(len) {}
	shared_ptr<Type> type;
	int len;
	void accept(s_visitor*) override;
	Type* apply(){return type.get();}
	Type* apply(std::string)
		{throw ST_error("Applying field to array");}
};

struct Invalid : virtual Type
{
	Invalid(int start = -1, int end = -1)
			: Type(start, end){}
	void accept(s_visitor*) override;

	Type* apply(){return this;}
	Type* apply(std::string){return this;}
};

struct Scope {
	Scope(shared_ptr<Scope> outer = shared_ptr<Scope>()) : outer(outer){};
	shared_ptr<Scope> outer;
	std::map<std::string, shared_ptr<Entry>> vars;

	bool local(std::string s){
		return vars.find(s)	!= vars.end();
	};

	shared_ptr<Entry> find(std::string s) {
		if(outer == nullptr && !local(s))
			throw ST_error("Symbol " + s + " used before declared.");
		return local(s) ? vars.find(s)->second : outer->find(s);
	}

	void insert(std::string s, shared_ptr<Entry> e)
	{
//		std::cout << "Inserting " << s << std::endl;

		if(local(s))
		{
//			std::cout << "Throwing insertion error in STable.hpp" << std::endl;

			std::stringstream ss;
			shared_ptr<Entry> e = find(s);
			ss << "Duplicate variable " <<
					s << ", last seen at loc: ("
					<< e->start << ", " << e->end << ")";
			throw ST_error(ss.str());
		}
		vars[s] = e;
	}

	void accept(s_visitor*);
};

struct Record : Type{
	Record(int start, int end, shared_ptr<Scope> sc)
		: Type(start, end), s(sc) {}
	shared_ptr<Scope> s;
	void accept(s_visitor*) override;

	Type* apply(){throw ST_error("Applying index to Record");}
	Type* apply(std::string id)
	{
		shared_ptr<Entry> e = s->find(id);
		shared_ptr<Var> v = dynamic_pointer_cast<Var>(e);
		if(!v)
			throw ST_error("Error converting record entry to Var");
		return v->type.get();
	}
};

class AST;

class STable
{
friend class AST;
public:
	STable(AST* t) : ast(t)
	{
		shared_ptr<Scope> univ = make_shared<Scope>();
		univ->insert("INTEGER", Integer::instance());
		current = make_shared<Scope>(univ);
	}

	~STable(){}

	shared_ptr<Scope> current;
	shared_ptr<Entry> find(std::string s)
	{
		return current->find(s);
	}
	void insert(std::string s, shared_ptr<Entry> e)
		{current->insert(s, e);}

private:
	AST* ast;
};

#endif /* STABLE_HPP_ */
