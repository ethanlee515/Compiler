#include <iostream>
#include <cstdlib>
#include <exception>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "scanner.hpp"
#include "parser.hpp"
#include "p_printers.hpp"
#include "AST.hpp"
#include "s_printer.hpp"
#include "backend.hpp"

using std::vector;
using std::cout;
using std::endl;
using std::cerr;
using std::exception;
using std::string;
using std::istream;
using std::ifstream;
using std::runtime_error;

void fillFile(string& str, string& fName)
{
	istream* file = (fName != "") ? new ifstream(fName) : (&std::cin);

	if(!file->good())
	{
		delete file;
		throw runtime_error("error opening file.");
	}
	int c = file->get();
	while(c != EOF)
	{
		str += c;
		c = file->get();
	}

	if(fName != "")
		delete file;
}

int main(int argc, char* argv[])
{
	try
	{
		int pos = 1;

		char flag = '\0';

		//flag

		if (pos < argc && argv[1][0] == '-')
		{
			if(argv[1][1] == '\0' || argv[1][2] != '\0')
				throw runtime_error("strange flag encountered");
			flag = argv[1][1];
			pos++;
		}

		char validFlags[] =
			{'\0', 'a', 'c', 'i', 's', 't', 'x'};

		if(!std::binary_search(validFlags, validFlags+7, flag))
			throw runtime_error("invalid flag.");

		if (pos + 1 < argc)
			throw runtime_error("too many inputs");

		string fName = pos < argc ? argv[pos] : "";
		string file;
		fillFile(file, fName);

		//actual compiling

		scanner* s = new scanner(file);
		if(flag == 's')
		{
			s->printAllTokens();
			return 0;
		}

		vector<token> toks;
		try {
			toks = s->all();
		} catch (runtime_error& e) {
			delete s;
			throw e;
		}
		delete s;

		parser* p = new parser(toks);

		if(flag == 'c')
		{
			p_printer* pr = new p_printer();
			p->attach(pr);
			p->parse();
			delete pr;
			delete p;
			return 0;
		}

		p->parse();

		if(!p->good())
			return 0;

		AST* a = p->ast;
		STable* st = p->symbols;

		if(flag == 't')
		{
			s_printer* pr = new s_printer();
			pr->print(st);
			delete pr;
			return 0;
		}
		if(flag == 'a')
		{
			a_printer* pr = new a_printer();
			a->root->accept(pr);
			delete pr;
			return 0;
		}
		if(flag == 'i')
		{
			cout << "Sorry, I gave up on the interpreter." << endl
				<< "At least I used that time wisely;" << endl
				<< "my AST is (hopefully) solid now." << endl;
			return 0;
		}

		backend(a, fName);

		return 0;
	}
	catch(runtime_error& e)
	{
		cerr << "error: " << e.what() << endl;
		return 1;
	}
}
