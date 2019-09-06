#ifndef P_PRINTER_H
#define P_PRINTER_H

#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "token.hpp"
#include "p_obsv.hpp"

class p_printer : public p_obsv
{
public:
	p_printer() : ind(0) {}
	virtual void done(bool good) override;
	virtual void start(std::string prod) override;
	virtual void end() override;
	virtual void match(token t) override;
	void forceOutput(){std::cout << ss.str();}
	~p_printer(){}
private:
	int ind;
	std::stringstream ss;
};

class p_printer_dot : public p_printer {
public:
	p_printer_dot() : nodeCount(0){}
	virtual void start(std::string prod) override;
	virtual void end() override;
	virtual void match(token t) override;
	virtual void done(bool good) override;
	~p_printer_dot(){}

private:
	int nodeCount; //labelCount
	void matchStr(std::string s);
	std::string represent(token t);
	std::string nextNode();
	std::vector<std::string> st;
	std::stringstream ss;
};

#endif
