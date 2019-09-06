#include "s_printer.hpp"
#include "STable.hpp"
#include <iostream>

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::to_string;

void s_printer::print(STable* st)
{
	print(st->current.get());
}

void s_printer::print(Scope* s)
{
	print("SCOPE BEGIN");
	ind++;

	for(pair<string, shared_ptr<Entry>> entry : s->vars)
	{
		print(entry.first + " =>");
		ind++;
		entry.second->accept(this);
		ind--;
	}
	ind--;
	print("END SCOPE");
}

void Var::accept(s_visitor* v){v->visit(this);}
void s_printer::visit(Var* n)
{
	print("VAR BEGIN");
	ind++;
	print("type:");
	ind++;
	n->type->accept(this);
	ind--;
	ind--;
	print("END VAR");
}

void Integer::accept(s_visitor* v){v->visit(this);}
void s_printer::visit(Integer*)
{
	print("INTEGER");
}

void Constant::accept(s_visitor* v){v->visit(this);}
void s_printer::visit(Constant* n)
{
	print("CONST BEGIN");
	print("  type:");
	print("    INTEGER");
	print("  value:");
	print("    " + to_string(n->value));
	print("END CONST");
}

void Array::accept(s_visitor* v){v->visit(this);}
void s_printer::visit(Array* n)
{
	print("ARRAY BEGIN");
	ind++;
	print("type:");
	ind++;
	n->type->accept(this);
	ind--;
	print("length:");
	print("  " + to_string(n->len));
	ind--;
	print("END ARRAY");
}

void Record::accept(s_visitor* v){v->visit(this);}
void s_printer::visit(Record* n)
{
	print("RECORD BEGIN");
	ind++;
	print(n->s.get());
	ind--;
	print("END RECORD");
}

void Invalid::accept(s_visitor* v){v->visit(this);}
void s_printer::visit(Invalid*)
{
	print("INVALID SYMBOL BEGIN");
	print("END INVALID SYMBOL");
}

void Number::accept(a_visitor* v) {v->visit(this);}
void a_printer::visit(Number* n)
{
	print("Number:");
	print("Value =>");
	ind++;
	n->num->accept(this);
	ind--;
}

void Variable::accept(a_visitor* v){v->visit(this);}
void a_printer::visit(Variable* n)
{
	print("Variable:");
	print("variable =>");
	ind++;
	n->var->accept(this);
	ind--;
}

void Index::accept(a_visitor* v){v->visit(this);}
void a_printer::visit(Index* n)
{
	print("Index:");
	print("location =>");
	ind++;
	n->loc->accept(this);
	ind--;
	print("expression =>");
	ind++;
	n->expr->accept(this);
	ind--;
}


void Field::accept(a_visitor* v) {v->visit(this);}
void a_printer::visit(Field* n)
{
	print("Field:");
	print("location =>");
	ind++;
	n->loc->accept(this);
	ind--;
	print("variable =>");
	ind++;
	n->var->accept(this);
	ind--;
}

void Binary::accept(a_visitor* v){v->visit(this);}
void a_printer::visit(Binary* n)
{
	print("Binary (" + token::KWSTR[n->op] +"):");
	print("left =>");
	ind++;
	n->left->accept(this);
	ind--;
	print("right =>");
	ind++;
	n->right->accept(this);
	ind--;
}

void Condition::accept(a_visitor* v){v->visit(this);}
void a_printer::visit(Condition* n)
{
	print("Condition (" + token::KWSTR[n->r] +"):");
	print("left =>");
	ind++;
	n->left->accept(this);
	ind--;
	print("right =>");
	ind++;
	n->right->accept(this);
	ind--;
}

void Assign::accept(a_visitor* v) {v->visit(this);}
void a_printer::visit(Assign* n)
{
	print("Assign:");
	print("location =>");
	ind++;
	n->loc->accept(this);
	ind--;
	print("expression =>");
	ind++;
	n->expr->accept(this);
	ind--;
	if(n->next)
		n->next->accept(this);
}

void If::accept(a_visitor* v) {v->visit(this);}
void a_printer::visit(If* n)
{
	print("If:");
	print("condition =>");
	ind++;
	n->cond->accept(this);
	ind--;
	print("true =>");
	ind++;
	n->inst_t->accept(this);
	ind--;
	print("false =>");
	ind++;
	if(n->inst_f)
		n->inst_f.get()->accept(this);
	ind--;
	if(n->next)
		n->next->accept(this);
}

void Repeat::accept(a_visitor* v) {v->visit(this);}
void a_printer::visit(Repeat* n)
{
	print("Repeat:");
	print("condition =>");
	ind++;
	n->cond->accept(this);
	ind--;
	print("instructions =>");
	ind++;
	n->inst->accept(this);
	ind--;
	if(n->next)
		n->next->accept(this);
}

void Read::accept(a_visitor* v) {v->visit(this);}
void a_printer::visit(Read* n)
{
	print("Read:");
	print("location =>");
	ind++;
	n->loc->accept(this);
	ind--;
	if(n->next)
		n->next->accept(this);
}

void Write::accept(a_visitor* v){v->visit(this);}
void a_printer::visit(Write* n)
{
	print("Write:");
	print("expression =>");
	ind++;
	n->expr->accept(this);
	ind--;
	if(n->next)
		n->next->accept(this);
}
