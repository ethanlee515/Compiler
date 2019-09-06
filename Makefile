CXX=g++
CPPFLAGS=-std=c++11 -Wall -Wextra -Wpedantic -O2

all: sc

sc: token.o scanner.o parser.o p_printers.o s_printer.o sc.o backend.o
	g++ -o sc token.o scanner.o parser.o p_printers.o s_printer.o backend.o sc.o

token.o: token.hpp token.cpp

scanner.o: token.hpp scanner.hpp scanner.cpp

parser.o: token.hpp AST.hpp STable.hpp parser.hpp parser.cpp

p_printers.o: p_obsv.hpp p_printers.hpp p_printers.cpp

s_printer.o: s_printer.hpp s_printer.cpp

backend.o: backend.hpp backend.cpp AST.hpp STable.hpp

sc.o: sc.cpp scanner.hpp parser.hpp p_printers.hpp AST.hpp s_printer.hpp

clean:
	rm -f *.o \
	rm -f sc
