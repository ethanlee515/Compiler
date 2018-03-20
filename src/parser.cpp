/*
 * parser.cpp
 *
 *  Created on: Feb 19, 2017
 *      Author: root
 */
#include "parser.hpp"
#include <vector>
#include "p_obsv.hpp"
#include "STable.hpp"
#include <iterator>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <memory>
using std::vector;
using std::string;
using std::stringstream;
using std::pair;
using std::cout;
using std::endl;
using std::binary_search;
using std::shared_ptr;
using std::runtime_error;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::cerr;
using std::make_pair;

parser::parser(vector<token> v)
	: toks(v), lastError(-5), loc(0), begun(false) {
	ast = new AST();
	symbols = new STable(ast);
	ast->symbols = symbols;
}

const int errorGap = 4;

void parser::parse()
{
	try {
		program();
	} catch (parser_error& s)
	{
		cerr << "error: malformed PROGRAM. " << s.what() << endl;

		stringstream ss;
		ss << "Compiliation halted @" << toks[loc];
		throw parser_error(ss.str());
	}

	for(p_obsv* o : observers)
		o->done(lastError == -5);
}

void parser::attach(p_obsv* o)
{
	observers.push_back(o);
}

void parser::program()
{
	notify_start("Program");
	matchKW(PROGRAM);
	string pName = matchID();
	matchKW(SEMICOLON);
	declarations();

	if(toks[loc].isKey(BEGIN))
	{
		matchKW(BEGIN);
		ast->root = instructions();
	}

	matchKW(END);
	if(matchID() != pName)
		throwErr(pName);
	matchKW(DOT);
	matchKW(EOF_TOKEN);
	notify_end();
}

void parser::declarations()
{
	notify_start("Declarations");
	while(toks[loc].isKey(CONST)
			|| toks[loc].isKey(TYPE)
			|| toks[loc].isKey(VAR))
	{
		try
		{
			switch(toks[loc].v)
			{
			case CONST:
				constDecl();
				break;
			case TYPE:
				typeDecl();
				break;
			case VAR:
				varDecl();
				break;
			}
		} catch (parser_error& e)
		{
			if(loc - lastError >= errorGap)
			{
				cerr << e.what() << endl;
				lastError = loc;
			}
			notify_start("nextDecl");
			const keyword strongKeys[] =
				{BEGIN, CONST, END, TYPE, VAR, EOF_TOKEN};
			while(toks[loc].k != token::KEYWORD ||
					!binary_search(strongKeys, strongKeys+6, toks[loc].v))
				++loc;
		}
	}

	notify_end();
}

void parser::constDecl()
{
	notify_start("ConstDecl");
	int start = loc;
	matchKW(CONST);
	while(toks[loc].k == token::IDENTIFIER)
	{
		string id = matchID();
		try
		{
			matchKW(EQUALS);
			pair<uintptr_t, uintptr_t> expr = expression();
			if(expr.second != 0)
			{
				symbols->insert(id, make_shared<Invalid>());
				throwErr("Constant");
			}
			symbols->insert(id, make_shared<Constant>(start, loc, expr.first));

		} catch (parser_error& e)
		{
			if(loc - lastError >= errorGap)
			{
				cerr << e.what() << endl;
				lastError = loc;
			}
			symbols->insert(id,
				make_shared<Invalid>(start, loc));
			throw e;
		}
		matchKW(SEMICOLON);
	}
	notify_end();
}

void parser::typeDecl()
{
	notify_start("TypeDecl");
	matchKW(TYPE);
	while(toks[loc].k == token::IDENTIFIER)
	{
		string id = matchID();
		try{
			matchKW(EQUALS);
			shared_ptr<Type> t = type();
			symbols->insert(id, t);
		} catch(parser_error& e)
		{
			if(loc - lastError >= errorGap)
			{
				cerr << e.what() << endl;
				lastError = loc;
			}
			symbols->insert(id, make_shared<Invalid>());
			throw e;
		}
		matchKW(SEMICOLON);
	}
	notify_end();
}

void parser::populateScope(shared_ptr<Scope> sc)
{
	while(toks[loc].k == token::IDENTIFIER)
	{
		int start = loc;
		vector<string> ids = identifierList();
		try {
			matchKW(COLON);
			shared_ptr<Type> t = type();
			for(string id : ids)
				sc->insert(id,
					make_shared<Var>(start, loc, t));
		} catch(parser_error& e) {
			if(loc - lastError >= errorGap)
			{
				cerr << e.what() << endl;
				lastError = loc;
			}
			for(string id : ids)
				sc->insert(id,
					make_shared<Invalid>());
		}
		matchKW(SEMICOLON);
	}
}

void parser::varDecl()
{
	notify_start("VarDecl");
	matchKW(VAR);
	populateScope(symbols->current);
	notify_end();
}

shared_ptr<Type> parser::type()
{
	notify_start("Type");
	int start = loc;

	shared_ptr<Type> ans;

	if(toks[loc].k == token::IDENTIFIER)
	{
		string s = matchID();
		shared_ptr<Type> t =
			dynamic_pointer_cast<Type>(symbols->find(s));
		if(!t)
			throwErr("Type identifier");
		ans = t;
	}
	else if(toks[loc].k == token::KEYWORD)
	{
		if(toks[loc].v == ARRAY)
		{
			matchKW(ARRAY);
			pair<uintptr_t, uintptr_t> expr = expression();
			if(expr.second != 0)
				throwErr("Constant");
			if((int)expr.first < 0)
				throwErr("Positive array size");

			matchKW(OF);
			shared_ptr<Type> t = type();

			ans = make_shared<Array>(start, loc, t, expr.first);
		}
		else if (toks[loc].v == RECORD)
		{
			matchKW(RECORD);
			shared_ptr<Scope> sc = make_shared<Scope>(symbols->current);
			populateScope(sc);
			ans = make_shared<Record>(start, loc, sc);
			matchKW(END);
		}
		else
			throwErr("Array, Record, or Identifier");
	}
	else
		throwErr("Array, Record, or Identifier");

	notify_end();
	return ans;
}

pair<uintptr_t, uintptr_t> parser::eval(
			pair<uintptr_t, uintptr_t> lft,
			pair<uintptr_t, uintptr_t> rgt,
			keyword op)
{
	if(!lft.second && !rgt.second)
	{
		int v1 = lft.first;
		int v2 = rgt.first;

		switch(op)
		{
		case PLUS:
			return make_pair(v1+v2, 0);
		case MINUS:
			return make_pair(v1-v2, 0);
		case STAR:
			return make_pair(v1*v2, 0);
		case DIV:
			if(v2 == 0)
				throw AST_error("Division by 0");
			return make_pair(v1/v2, 0);
		case MOD:
			if(v2 == 0)
				throw AST_error("Mod by 0");
			return make_pair(v1%v2, 0);
		default:
			throw AST_error("collapsing expression with operation " + token::KWSTR[op]);
		}
	}
	else
	{
		Binary* node = new Binary();

		if((lft.second && lft.first != (uintptr_t) Integer::instance().get())
				||
			(rgt.second && rgt.first != (uintptr_t) Integer::instance().get()))
			throwErr("Integer for arithmetic expression");

		node->left = lft.second ?
				unique_ptr<Expression>((Expression*)lft.second)
				: unique_ptr<Expression>(new Number(lft.first));

		node->right = rgt.second ?
				unique_ptr<Expression>((Expression*)rgt.second)
				: unique_ptr<Expression>(new Number(rgt.first));
		node->op = op;

		return make_pair((uintptr_t)Integer::instance().get(), (uintptr_t) node);
	}
}

//First slot: the integer value, if no node is created.
//Second slot: pointer to the node, if one is created.
pair<uintptr_t, uintptr_t> parser::expression()
{
	pair<uintptr_t, uintptr_t> ans;
	notify_start("Expression");
	bool negated = false;
	if(toks[loc].k == token::KEYWORD)
	{
		if(toks[loc].v == PLUS)
			matchKW(PLUS);
		else if(toks[loc].v == MINUS)
		{
			matchKW(MINUS);
			negated = true;
		}
	}

	if(!negated)
		ans = term();
	else
		ans = eval(make_pair(0,0), term(), MINUS);

	while(toks[loc].isKey(PLUS) || toks[loc].isKey(MINUS))
	{
		keyword k = (keyword) toks[loc].v;
		matchKW(k);
		ans = eval(ans, term(), k);
	}

	notify_end();
	return ans;
}

pair<uintptr_t, uintptr_t> parser::term()
{
	notify_start("Term");
	pair<uintptr_t, uintptr_t> ans = factor();
	while(toks[loc].isKey(STAR) || toks[loc].isKey(DIV)
			|| toks[loc].isKey(MOD))
	{
		keyword k = (keyword) toks[loc].v;
		matchKW(k);
		ans = eval(ans, factor(), k);
	}
	notify_end();
	return ans;
}

pair<uintptr_t, uintptr_t> parser::factor()
{
	notify_start("Factor");
	pair<uintptr_t, uintptr_t> ans;
	if(toks[loc].k == token::INTEGER)
		ans = make_pair(matchInt(), 0);
	else if(toks[loc].k == token::KEYWORD
			&&toks[loc].v == OPEN_PAREN)
	{
		matchKW(OPEN_PAREN);
		ans = expression();
		matchKW(CLOSE_PAREN);
	}
	else
	{
		ans = designator();
	}
	notify_end();
	return ans;
}

bool startsInst(token t)
{
	return t.k == token::IDENTIFIER || (t.k == token::KEYWORD
			&& (t.v == COLON_EQ || t.v == IF
				|| t.v == REPEAT || t.v == WHILE
				|| t.v == READ || t.v == WRITE));
}

unique_ptr<Instruction> parser::instructions()
{
	notify_start("Instructions");
	unique_ptr<Instruction> root = instruction();
	Instruction* current = root.get();
	while((toks[loc].k == token::KEYWORD
			&& toks[loc].v == SEMICOLON)
			|| startsInst(toks[loc]))
	{
		try{
			matchKW(SEMICOLON);
			current->next = instruction();
			current = current->next.get();
		}
		catch(parser_error& e)
		{
			if(loc - lastError >= errorGap)
			{
				cerr << e.what() << endl;
				lastError = loc;
			}
			notify_start("nextInst");
			const keyword strongKeys[] =
				{END, IF, READ, REPEAT,
						WHILE, WRITE, EOF_TOKEN};
			while(toks[loc].k != token::KEYWORD ||
				!binary_search(strongKeys, strongKeys+7, toks[loc].v))
				++loc;
		}
	}
	notify_end();
	return root;
}

unique_ptr<Instruction> parser::instruction()
{
	notify_start("Instruction");

	unique_ptr<Instruction> ans;

	token t = toks[loc];
	if(t.k == token::INTEGER)
		throwErr("Instruction");
	if(t.k == token::KEYWORD)
		switch(t.v)
		{
			case COLON_EQ:
				ans = assign();
				break;
			case IF:
				ans = ifAST();
				break;
			case REPEAT:
				ans = repeat();
				break;
			case WHILE:
				ans = whileAST();
				break;
			case READ:
				ans = read();
				break;
			case WRITE:
				ans = write();
				break;
			default:
				throwErr("Instruction");
		}
	else
		ans = assign();

	notify_end();
	return ans;
}

unique_ptr<Assign> parser::assign()
{
	notify_start("Assign");
	unique_ptr<Assign> ans = unique_ptr<Assign>(new Assign());
	pair<uintptr_t, uintptr_t> d = designator();
	if(!d.second)
		throwErr("Non-constant in Assign");
	ans->loc = unique_ptr<Location>((Location*) d.second);
	matchKW(COLON_EQ);
	pair<uintptr_t, uintptr_t> e = expression();

	/*
	cout << "Integer=" << (uintptr_t) Integer::instance().get() << endl;
	cout << "D=(" << d.first << ", " << d.second << ")" << endl;
	cout << "E=(" << e.first << ", " << e.second << ")" << endl;
	*/

	if(d.first != e.first
			&& (
			(d.second != 0 && d.first != (uintptr_t) Integer::instance().get())
			|| (e.second != 0 && e.first != (uintptr_t) Integer::instance().get())
			)
		)
		throwErr("Same Type");

	ans->expr = unique_ptr<Expression>(
			e.second ?
			(Expression*) e.second :
			new Number(e.first)
			);
	notify_end();
	return ans;
}

unique_ptr<If> parser::ifAST()
{
	notify_start("If");
	unique_ptr<If> ans = unique_ptr<If>(new If());
	matchKW(IF);
	ans->cond = condition();
	matchKW(THEN);
	ans->inst_t = instructions();

	if(toks[loc].isKey(ELSE))
	{
		matchKW(ELSE);
		ans->inst_f = instructions();
	}
	matchKW(END);
	notify_end();
	return ans;
}

unique_ptr<Repeat> parser::repeat()
{
	unique_ptr<Repeat> ans;
	notify_start("Repeat");
	matchKW(REPEAT);
	ans->inst = instructions();
	matchKW(UNTIL);
	ans->cond = condition();
	matchKW(END);
	notify_end();
	return ans;
}

unique_ptr<If> parser::whileAST()
{
	unique_ptr<If> ans = unique_ptr<If>(new If());
	unique_ptr<Repeat> rep = unique_ptr<Repeat>(new Repeat());
	notify_start("While");
	matchKW(WHILE);
	ans->cond = condition();
	rep->cond = unique_ptr<Condition>
		(new Condition(*(ans->cond)));
	rep->cond->r = negate(rep->cond->r);
	matchKW(DO);
	rep->inst = instructions();
	ans->inst_t = move(rep);
	matchKW(END);
	notify_end();
	return ans;
}

unique_ptr<Condition> parser::condition()
{
	unique_ptr<Condition> ans = unique_ptr<Condition>(new Condition());
	notify_start("Condition");
	pair<uintptr_t, uintptr_t> lft = expression();

	ans->left = lft.second ?
			unique_ptr<Expression>((Expression*)lft.second) :
			unique_ptr<Number>(new Number(lft.first));


	keyword k = (keyword) toks[loc].v;
	if(toks[loc].k != token::KEYWORD ||
		(k != EQUALS && k != HASH && k != LESS
				&& k != GREATER && k != LESS_EQ && k != GREATER_EQ))
		throwErr("Condition");

	matchKW(k);
	ans->r = k;

	auto rgt = expression();
	ans->right = rgt.second ?
				unique_ptr<Expression>((Expression*)rgt.second) :
				unique_ptr<Number>(new Number(rgt.first));
	notify_end();
	return ans;
}

unique_ptr<Write> parser::write()
{
	auto ans = unique_ptr<Write>(new Write());
	notify_start("Write");
	matchKW(WRITE);
	auto expr = expression();
	notify_end();
	ans->expr = expr.second ?
			unique_ptr<Expression>((Expression*)expr.second) :
			unique_ptr<Number>(new Number(expr.first));
	return ans;
}

unique_ptr<Read> parser::read()
{
	auto ans = unique_ptr<Read>(new Read());
	notify_start("Read");
	matchKW(READ);
	auto d = designator();
	if(d.second == 0)
		throwErr("Constant");
	ans->loc = unique_ptr<Location>((Location*)d.second);
	notify_end();
	return ans;
}

std::pair<uintptr_t, uintptr_t> parser::designator()
{
	notify_start("Designator");
	string id = matchID();
	auto ans = selector(id);
	notify_end();
	return ans;
}

pair<uintptr_t, uintptr_t> parser::selector(string id)
{
	pair<uintptr_t, uintptr_t> ans;
	notify_start("Selector");

//	cout << "Selectors for " << id << "..." << endl;


	shared_ptr<Entry> e = symbols->find(id);
	if(shared_ptr<Constant> c
			= dynamic_pointer_cast<Constant>(e))
	{
		ans = make_pair(c->value, 0);
	}
	else if(shared_ptr<Var> c
			= dynamic_pointer_cast<Var>(e))
	{
		Variable* v = new Variable();
		v->var = c;
		ans = make_pair(
				(uintptr_t) c->type.get(),
				(uintptr_t) v);
	}
	else
		throwErr("Constant or Variable");

//	cout << "Type of ans:" << ans.first << endl;

	while(toks[loc].k == token::KEYWORD
			&& (toks[loc].v == OPEN_SQUARE
					|| toks[loc].v == DOT))
	{
		if(!ans.second)
			throwErr("No selectors (for Constant)");

		if(toks[loc].v == OPEN_SQUARE)
		{
			int indStart = loc;
			matchKW(OPEN_SQUARE);
			vector<pair<uintptr_t, uintptr_t>> v
				= expressionList();

			for(pair<uintptr_t, uintptr_t> p : v)
			{
				ans.first = (uintptr_t)((Type*)ans.first)->apply();

				Index* ind = new Index();
				ind->start = indStart;
				ind->end = loc;
				ind->loc = unique_ptr<Location>((Location*)ans.second);
				ind->expr =
						unique_ptr<Expression>(
						p.second ? (Expression*) p.second :
								new Number(p.first));
				ans.second = (uintptr_t) ind;
			}

			matchKW(CLOSE_SQUARE);
		}
		else
		{
			matchKW(DOT);
			string id = matchID();
			Field* f = new Field();
//			cout << "Checkpoint A" << endl;

			f->loc = unique_ptr<Location>((Location*) ans.second);
			Variable* v = new Variable();

//			cout << "Debug message:" << endl;
//			for(pair<string, shared_ptr<Entry>> p
//					: ((Record*) ans.first)->s->vars)
//				cout << p.first << endl;

//			cout << "Checkpoint D" << endl;

			v->var = dynamic_pointer_cast<Var>(
					((Record*) ans.first)->s->find(id));

//			cout << "Checkpoint C" << endl;
			f->var = unique_ptr<Variable>(v);
			ans.first = (uintptr_t)(((Type*) ans.first)->apply(id));
			ans.second = (uintptr_t) f;
		}
	}
	notify_end();
	return ans;
}

vector<string> parser::identifierList()
{
	vector<string> ans;
	notify_start("IdentifierList");
	ans.push_back(matchID());
	while((toks[loc].k == token::KEYWORD
			&& toks[loc].v == COMMA)
			|| toks[loc].k == token::IDENTIFIER)
	{
		matchKW(COMMA);
		ans.push_back(matchID());
	}
	notify_end();
	return ans;
}

bool startsExp(token t)
{
	return t.k == token::INTEGER
			|| (t.k == token::KEYWORD && (
					t.v == PLUS || t.v == MINUS
					|| t.v == OPEN_PAREN
			)) || t.k == token::IDENTIFIER;
}

vector<pair<uintptr_t, uintptr_t>> parser::expressionList()
{
	vector<pair<uintptr_t, uintptr_t>> v;
	try{
	notify_start("ExpressionList");

	pair<uintptr_t, uintptr_t> temp = expression();
	if(temp.second != 0
			&& temp.first != (uintptr_t) Integer::instance().get())
		throwErr("Integer expression");
	v.push_back(temp);

	while (toks[loc].isKey(COMMA) || startsExp(toks[loc]))
	{
		matchKW(COMMA);
		pair<uintptr_t, uintptr_t> temp = expression();
		if(temp.second != 0
				&& temp.first != (uintptr_t) Integer::instance().get())
			throwErr("Integer expression");
		v.push_back(temp);
	}
	} catch(parser_error& e)
	{
		if(loc - lastError >= errorGap)
		{
			cerr << e.what() << endl;
			lastError = loc;
		}
		for(pair<uintptr_t, uintptr_t> p : v)
			delete static_cast<Expression*>((Expression*)p.second);
		throw e;
	}
	notify_end();
	return v;
}

void parser::throwErr(string prod)
{
	stringstream ss;

	ss << "error: " << "@token " << loc << " "
		<< prod << " expected, \""
		<< toks[loc] << "\" encountered.";

	string msg = ss.str();
	throw parser_error(msg);
}

void parser::matchKW(int key)
{
	token t = toks[loc];
	if(t.k == token::KEYWORD && t.v == key) {
		notify_match(t);
		loc++;
	}
	else {
		const keyword weakKeys[] =
			{CLOSE_PAREN, COMMA, SEMICOLON,
					END, CLOSE_SQUARE};

		if(binary_search(weakKeys, weakKeys + 5, key))
		{
			try{throwErr(token::KWSTR[key]);}catch(parser_error& e){
				if(loc - lastError >= errorGap)
				{
					cerr << e.what() << endl;
					lastError = loc;
				}
			}
			notify_match(token(token::KEYWORD, key, -1, -1));
		}

		else
			throwErr(token::KWSTR[key]);
	}
}

int parser::matchInt()
{
	token t = toks[loc];
	if(t.k == token::INTEGER) {
		notify_match(t);
		loc++;
	}
	else
		throwErr("Integer");
	return toks[loc-1].v;
}

string parser::matchID()
{
	token t = toks[loc];
	if(t.k == token::IDENTIFIER) {
		notify_match(t);
		loc++;
	}
	else
		throwErr("Identifier");
	return toks[loc-1].id;
}

void parser::notify_start(string s)
{
	for(p_obsv* o : observers)
		o->start(s);
}

void parser::notify_end()
{
	for(p_obsv* o : observers)
		o->end();
}

void parser::notify_match(token t)
{
	for(p_obsv* o : observers)
		o->match(t);
}

keyword parser::negate(keyword r)
{
	switch(r)
	{
	case EQUALS:
		return HASH;
	case HASH:
		return EQUALS;
	case LESS:
		return GREATER_EQ;
	case GREATER_EQ:
		return LESS;
	case GREATER:
		return LESS_EQ;
	case LESS_EQ:
		return GREATER;
	default:
		throw AST_error("Negating non-negatable token.");
		return EQUALS;
	}
}
