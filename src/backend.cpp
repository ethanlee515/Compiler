#include "backend.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

using std::pair;
using std::cout;
using std::endl;
using std::cerr;
using std::ofstream;
using std::dynamic_pointer_cast;
using std::stringstream;
using std::vector;
using std::to_string;

backend::backend(AST* ast, std::string fName)
: ast(ast), symbols(ast->symbols), fName(fName)
{
	int totalSize = process(ast->symbols->current);
	ss << "\t.comm\tvars, " << totalSize << endl;
	ss << "wi:\t.asciz\t\"%d\\n\"" << endl;
	ss << "ri:\t.asciz\t\"%d\"" << endl;
	ss << "err:\t.word\tstderr" << endl;
	ss << "oob:\t.asciz\t\"error: Index %d out of bound @(%d, %d)\\n\"" << endl;
	ss << "\t.align\t2" << endl;
	ss << "\t.global\tmain" << endl;
	ss << "main:" << endl;
	ss << "\tstmfd\tsp!, {fp,lr}" << endl;
	ss << "\tadd\tfp, sp, #4" << endl;

	if(ast->root)
		ast->root->accept(this);

	ss << endl;
	ss << "\t@epilogue" << endl;
	ss << "\tmov\tr0, #0" << endl;
	ss << "\tldmfd\tsp!, {fp, pc}" << endl;

	if(fName == "")
	{
		cout << ss.str();
	}
	else
	{
		int loc = fName.find(".sim");
		if(loc == -1 || (int)fName.length() != loc + 4)
			throw runtime_error("strange file name");
		ofstream oFile(fName.substr(0, loc) + ".s");
		oFile << ss.str();
	}
}

void backend::visit(Number* n) {
	string dest = toR1 ? "r1" : push();
	toR1 = false;
	ss << "\tmov\t" << dest << ", #" << n->v() << endl;
	isNum = true;
	currentType = Integer::instance().get();
}

void backend::visit(Variable* n) {
	pool();
	string dest = toR1 ? "r1" : push();
	toR1 = false;
	currentType = n->var->type.get();
	ss << "\tldr\t" << dest << ", =vars+" << offsets[n->var.get()] << endl;
	isNum = false;
}

void backend::visit(Index* n) {

	Number* num = dynamic_cast<Number*>(n->expr.get());

	bool r = toR1;

	if(!num)
		toR1 = false;
	n->loc->accept(this);

	Array* arr = dynamic_cast<Array*>(currentType);

	if(!arr)
	{
		cout << "Integer = "
				<< (uintptr_t) Integer::instance().get() << endl;
		cout << "Unknown type = "
				<< (uintptr_t) currentType << endl;
		throw runtime_error("Array expected in backend Index");
	}

	int len = arr->len;
	currentType = currentType->apply();
	Type* t = currentType;

	if(num)
	{
		if(num->v() > len || num->v() < 0)
		{
			stringstream msg;
			msg << "Index " << num->v() << " out of bound @("
					<< n->start << ", " << n->end << ")";
			throw runtime_error(msg.str());
		}
		sum += num->v() * sizes[currentType];
	}
	else
	{
		toR1 = true;
		n->expr->accept(this);
		if(!isNum)
		{
			if(sum == 0)
				ss << "\tldr\tr1, [r1]" << endl;
			else
			{
				ss << "\tldr\tr1, [r1, #" << sum << "]" << endl;
				sum = 0;
			}
		}

		ss << "\t@boundcheck" << endl;
		ss << "\tcmp\tr1, #" << len << endl;
		int l = lc++;
		ss << "\tbge\toob" << l << endl;
		ss << "\tcmp\tr1, #0" << endl;
		ss << "\tblt\toob" << l << endl;
		ss << "\tb\tsafe" << l << endl;
		ss << "oob" << l << ":" << endl;
		ss << "\tmov\tr2, r1" << endl;
		ss << "\tldr\tr1, =oob" << endl;
		ss << "\tldr\tr0, err" << endl;
		ss << "\tldr\tr0, [r0]" << endl;
		ss << "\tmov\tr3, #" << n->end << endl;
		ss << "\tstmfd\tsp!, {r3}" << endl;
		ss << "\tmov\tr3, #" << n->start << endl;
		ss << "\tbl\tfprintf" << endl;
		ss << "\tmov\tr0, #1" << endl;
		ss << "\tbl\texit" << endl;
		ss << "safe" << l << ":" << endl;
		multiply("r1", sizes[currentType]);

		string reg = pop();
		string dest = r ? "r1" : push();
		ss << "\tadd\t" << dest << ", " << reg << ", r1" << endl;
	}

	currentType = t;
	isNum = false;
}

void backend::visit(Field* n) {
	n->loc->accept(this);
	string reg = peek();
	currentType = n->var->var->type.get();
	sum += offsets[n->var->var.get()];
}

int intlog2(int);
void backend::visit(Binary* n) {
	bool r1 = toR1;
	toR1 = false;
	n->left->accept(this);
	if(!isNum)
	{
		string last = peek();
		if(sum == 0)
			ss << "\tldr\t" << last << ", [" << last << "]" << endl;
		else
		{
			ss << "\tldr\t" << last << ", ["
					<< last << ", #" << sum << "]" << endl;
			sum = 0;
		}
	}

	isNum = false;

	//Shortcut optimization for constants
	Number* num = dynamic_cast<Number*>(n->right.get());
	if(num && (n->op == STAR))
	{
		multiply(peek(), num->v());
		isNum = true;
		if(r1)
			ss << "\tmov\tr1, " << pop();
		return;
	}

	if(num && n->op == PLUS)
	{
		isNum = true;
		if(r1)
			ss << "\tadd\tr1, "
				<< pop() << ", #" << num->v() << endl;
		else
			ss << "\tadd\t" << peek() << ", "
				<< peek() << ", #" << num->v() << endl;
		return;
	}

	if(num && n->op == DIV &&
			(1 << intlog2(num->v())) == num->v())

	{
		isNum = true;
		if(r1)
			ss << "\tmov\tr1, "
				<< pop() << ", asl #" << intlog2(num->v()) << endl;
		else
			ss << "\tmov\t" << peek() << ", "
				<< peek() << ", asl #" << intlog2(num->v()) << endl;
		return;
	}
	toR1 = true;
	n->right->accept(this);
	if(!isNum)
		ss << "\tldr\tr1, [r1]" << endl;

	string reg = pop();

	string dest = (r1) ? "r1" : push();
	switch(n->op)
	{
	case PLUS:
		ss << "\tadd\t" << dest << ", " << reg << ", r1" << endl;
		break;
	case MINUS:
		ss << "\tsub\t" << dest << ", " << reg << ", r1" << endl;
		break;
	case STAR:
		ss << "\tmul\t" << dest << ", " << reg << ", r1" << endl;
		break;
	case DIV:
		ss << "\tmov\tr0, " << reg << endl;
		ss << "\tbl\t__aeabi_idiv" << endl;
		ss << "\tmov\t" << dest << ", r0" << endl;
		break;
	case MOD:
		ss << "\tmov\tr0, " << reg << endl;
		ss << "\tbl\t__aeabi_idivmod" << endl;
		if(!r1)
			ss << "\tmov\t" << dest << ", r1" << endl;
		break;
	default:
		throw runtime_error("Backend parsed unknown binary operation #"
					+ to_string(n->op) + " in AST");
	}
	isNum = true;
}
void backend::visit(Condition* n) {
	n->left->accept(this);
	if(!isNum)
	{
		string reg = peek();
		if(sum == 0)
			ss << "\tldr\t" <<
				reg << ", [" << reg << "]" << endl;
		else
		{
			ss << "\tldr\t" << reg
					<< ", [" << reg << ", #"
					<< sum << "]" << endl;
			sum = 0;
		}
	}

	toR1 = true;
	n->right->accept(this);
	if(!isNum)
	{
		if(sum == 0)
			ss << "\tldr\tr1, [r1]" << endl;
		else
		{
			ss << "\tldr\tr1, [r1, #" << sum << "]" << endl;
			sum = 0;
		}
	}


	ss << "\tcmp\t" << pop() << ", r1" << endl;

	switch(n->r)
	{
	case EQUALS:
		cond = "be";
		condR = "bne";
		break;
	case HASH:
		cond = "bne";
		condR = "beq";
		break;
	case GREATER:
		cond = "bgt";
		condR = "ble";
		break;
	case GREATER_EQ:
		cond = "bge";
		condR = "blt";
		break;
	case LESS:
		cond = "blt";
		condR = "bge";
		break;
	case LESS_EQ:
		cond = "ble";
		condR = "bgt";
		break;
	default:
		throw runtime_error("Unknown comparison #" + to_string(n->r));
	}
}

string combine(vector<string> v, unsigned int n = 8)
{
	stringstream str;
	int sz = n < v.size() ? n : v.size();
	str << "{";
	for(int i = 0; i < sz - 1; ++i)
	{
		str << v[i] << ",";
	}
	str << v[sz-1] << "}";
	return str.str();
}

void backend::visit(Assign* n) {
	pool();
	ss << endl;
	ss << "\t@assignment" << endl;

	n->loc->accept(this);
	string last = peek();
	if(sum != 0)
	{
		ss << "\tadd\t" << last << ", " << last << ", #" << sum << endl;
		sum = 0;
	}

	int numEle = sizes[currentType] / 4;

	isNum = false;
	toR1 = true;
	n->expr->accept(this);
	if(isNum)
	{
		ss << "\tstr\tr1, [" << pop() << "]" << endl;
		if(n->next)
			n->next->accept(this);
		return;
	}

	int numReg = numEle > 8 ? 8 : numEle;

	string dest = peek();
	if(numEle > 4)
	{
		ss << "\tmov\tr0, " << pop() << endl;
		dest = "r0";
	}

	vector<string> regs;
	for(int i = 0; i < numReg - 1; ++i)
		regs.push_back(push());
	regs.push_back(borrow());

	for(int i = 0; i < numEle / 8; ++i)
	{
		ss << "\tldmfd\tr1!, " << combine(regs) << endl;
		ss << "\tstmea\t" << dest << "!, " << combine(regs) << endl;
	}

	if(numEle%8 != 1)
	{
		ss << "\tldmfd\tr1!, " << combine(regs, numEle % 8) << endl;
		ss << "\tstmea\t" << dest << "!, "
				<< combine(regs, numEle % 8) << endl;
	}
	else
		ss << "\tstr\tr1, [" << dest << "]" << endl;

	for(int i = 0; i < numReg - 1; ++i)
		pop();

	if(dest != "r0")
		pop();

	if(n->next)
		n->next->accept(this);
}

void backend::visit(If* n) {
	pool();
	ss << endl;
	ss << "\t@If" << endl;

	int l = lc++;
	n->cond->accept(this);
	string t = cond, f = condR;
	ss << "\t" << f << "\tF" << l << endl;
	n->inst_t->accept(this);

	if(n->inst_f)
		ss << "\tb\tE" << l << endl;

	ss << "F" << l << ":" << endl;
	if(n->inst_f)
		n->inst_f->accept(this);

	ss << "E" << l << ":" << endl;

	if(n->next)
		n->next->accept(this);
}

void backend::visit(Repeat* n)
{
	pool();
	ss << endl;
	ss << "\t@Repeat" << endl;
	int l = lc++;
	string t = cond;
	ss << "R" << l << ":" << endl;
	n->inst->accept(this);
	n->cond->accept(this);
	ss << "\t" << t << "\tR" << l << endl;
	if(n->next)
		n->next->accept(this);
}

void backend::visit(Read* n) {
	pool();
	ss << endl;
	ss << "\t@Read" << endl;
	isNum = false;
	toR1 = true;
	n->loc->accept(this);

	if(sum != 0)
	{
		ss << "\tadd\tr1, r1, #" << sum << endl;
		sum = 0;
	}

	ss << "\tldr\tr0, =ri" << endl;
	ss << "\tbl\t__isoc99_scanf" << endl;
	if(n->next)
	n->next->accept(this);
}

void backend::visit(Write* n) {
	pool();
	ss << endl;
	ss << "\t@write" << endl;
	isNum = false;
	toR1 = true;
	n->expr->accept(this);
	if(!isNum)
	{
		if(sum == 0)
			ss << "\tldr\tr1, [r1]" << endl;
		else
		{
			ss << "\tldr\tr1, [r1, #" << sum << "]" << endl;
			sum = 0;
		}
	}
	ss << "\tldr\tr0, =wi" << endl;
	ss << "\tbl\tprintf" << endl;
	if(n->next)
		n->next->accept(this);
}

//ST
//-------------------------------
int backend::process(shared_ptr<Scope> sc)
{
	int offset = 0;

	for(pair<string, shared_ptr<Entry>> v : sc->vars)
	{
		v.second->accept(this);
		if(offsets.find(v.second.get()) != offsets.end())
		{
			int temp = offsets[v.second.get()];
			offsets[v.second.get()] = offset;
			offset += temp;
		}
	}
	return offset;
}

void backend::visit(Var* n) {
	n->type->accept(this);
	offsets[n] = sizes[n->type.get()];
}

void backend::visit(Integer* n) {
	sizes[n] = 4;
}

void backend::visit(Constant*) {} // actually does nothing

void backend::visit(Array* n) {
	if(sizes[n] != 0)
		return;

	arrLen[n] = n->len;
	n->type->accept(this);
	sizes[n] = (n->len) * sizes[n->type.get()];
}
void backend::visit(Record* n) {
	if(sizes[n] != 0)
		return;

	sizes[n] = process(n->s);
}

//-------------------------------

string backend::push()
{
	int n = top;
	if(top == 7 && warp != 0)
		ss << "\tstmfd\tsp!, {r8,r9,r10,r11}" << endl;
	if(top == 11)
	{
		ss << "\tstmfd\tsp!, {r4,r5,r6,r7}" << endl;
		warp++;
		top = 3;
	}
	top++;

	return string("r")
			+ to_string(n);
}

string backend::pop()
{
	if(top == 4 && warp != 0)
	{
		ss << "\tldmfd\tsp!, {r4,r5,r6,r7}" << endl;
		warp--;
		top = 12;
	}
	else if(top == 8 && warp != 0)
		ss << "\tldmfd\tsp!, {r8,r9,r10,r11}" << endl;
	top--;
	return string("r") + to_string(top);
}

string backend::borrow()
{
	return string("r") + to_string(top);
}

string backend::peek()
{
	return string("r") + to_string((top == 4) ? 11 : top-1);
}

void backend::pool()
{
	if(!((pc++)%30))
	{
		int l = lc++;
		ss << "\t@pool" << endl;
		ss << "\tbl\tP" << l << endl;
		ss << "\t.ltorg" << endl;
		ss << "P" << l << ":" << endl;
	}
}

void backend::visit(Invalid*){} // actually does nothing

int intlog2(int a)
{
	int count = -1;
	while(a)
	{
		a = a >> 1;
		count++;
	}
	return count;
}

void backend::multiply(string reg, int num)
{
	ss << "\t@constMult" << endl;

	if(num < 0)
	{
		string r = borrow();
		ss << "\tmov\t" << r << ", #" << num << endl;
		ss << "\tmul\t" << reg << ", " << reg << ", " << r;
		return;
	}
	if(num == 0)
	{
		ss << "\tmov\t" << reg << ", #0" << endl;
		return;
	}
	if(num == 1)
	{
		return;
	}

	string r = borrow();

	if(1 << intlog2(num) != num)
		ss << "\tmov\t" << r << ", " << reg << endl;

	int counter = 1;
	for(int i = intlog2(num) - 1; i >= 0; --i)
	{
		if((num >> i) & 1)
		{
			ss << "\tmov\t" << reg << ", "
				<< reg << ", asl #" << counter << endl;
			counter = 1;

			ss << "\tadd\t" << reg << ", " << reg
					<< ", " << r << endl;
		}
		else
			counter++;
	}
	if(counter != 1)
		ss << "\tmov\t" << reg << ", "
			<< reg << ", asl #" << counter-1 << endl;
}
