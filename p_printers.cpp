#include "p_printers.hpp"
#include <string>
#include <iostream>
#include <vector>
using std::string;
using std::cout;
using std::ostream;
using std::endl;
using std::vector;
using std::to_string;

void printI(string s, int ind, ostream& ss = cout)
{
	for(int i = 0; i < ind; ++i)
		ss << "  ";
	ss << s << endl;
}

void p_printer::start(string s)
{
	printI(s, ind, ss);
	ind++;
}

void p_printer::done(bool good)
{
	if(good) cout << ss.str();
}

void p_printer::end()
{
	ind--;
}

void p_printer::match(token t)
{
	printI((string)t, ind, ss);
}

string p_printer_dot::nextNode()
{
	string s = "n";
	s += to_string(nodeCount);
	nodeCount += 1;
	return s;
}

void p_printer_dot::start(string prod)
{
	string node = nextNode();
	ss << node << " [label=\"" << prod << "\" shape=box];" << endl;

	if(!st.empty()) {
		ss << st.back() << "->" << node
	  << ";" << endl;
	}

	st.push_back(node);
}

void p_printer_dot::end()
{
	if(!st.empty())
		st.pop_back();
}

void p_printer_dot::matchStr(string s)
{
	string node = nextNode();
	ss << node << " [label=\"" << s << "\" shape=diamond];" << endl;
	ss << st.back()  << "->" << node
	  << ";" << endl;
}

void p_printer_dot::match(token t)
{
	matchStr(represent(t));
}

void p_printer_dot::done(bool good)
{
	if(!good)
		return;
	cout << "digraph G {" << endl;
	cout << ss.str();
	cout << "}" <<  endl;
}

string p_printer_dot::represent(token t)
{
	switch(t.k)
	{
	case token::KEYWORD:
		return token::KWSTR[t.v];
	case token::IDENTIFIER:
		return t.id;
	case token::INTEGER:
		return to_string(t.v);
	default:
		return "Invalid token";
	}
}

