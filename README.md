# Compiler

This is a class project when I was an undergraduate.
The language's grammar was provided to us as EBNF.
We were asked to implement it as an interpreter/compiler combo.

## Usage

* First compile using GNU make. Then invoke the sc executable.
* If a file name is given, that file is processed. Otherwise, the standard input is processed.
* One of the following flags is allowed
	* -s: Runs the scanner, or more commonly called the lexer. That is, prints the list of tokens.
	* -c: Prints the concrete syntax tree. As in, the parse tree.
	* -t: Prints the symbol table. Note: May be buggy if complicated records/arrays structures are involved.
	* -a: Prints the abstract syntax tree.
	* -i: Runs the interpreter. Unfortunately this is NYI.
	* No flags: Compiles the source code into ARM assembly.

## Disclaimer

If you happen to be taking the same class, please do not cheat off of this. There's a good chance that this code doesn't work very well anyways.

To quote the man himself directly: "If you try to use this for homework, you can be certain that your instructor will find the matching code here and bust you. But your instructor may not be able to bust me because I might have graduated already. Ha-ha!"

## TODO
* Fix the bugs... does this thing even work most of the time?
* Switch to C++17, with smart pointers, enum classes, and variants
* Actually write the interpreter
* Write code generation for x86
