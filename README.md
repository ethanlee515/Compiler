# Compiler

This is a class project I wrote as an undergraduate.
The language's grammar was provided to us in EBNF.
We were asked to implement it as an interpreter/compiler combo.

I apologize for lack of meaningful version history.
This was written when I didn't know how to use git.

***WARNING: The parser currently segfaults due to a double-free after giving output. If only I knew about smart pointers and ownership rules when I wrote this... One of these days I might come back and fix everything up here***

## Usage

* Build using GNU make, then invoke the sc executable.
* If a file name is given, that file is processed. Otherwise, the standard input is processed.
* One of the following options is allowed
	* No flags: Compiles the source code into ARM assembly.
	* -s flag: Runs the scanner, or more commonly called the lexer. That is, prints the list of tokens.
	* -c flag: Prints the concrete syntax tree. As in, the parse tree.
	* -t flag: Prints the symbol table. ***Symbol table may be buggy if complicated records/arrays structures are involved.***
	* -a flag: Prints the abstract syntax tree.
	* -i flag: Runs the interpreter. Unfortunately this is NYI.

Code examples can be found in the examples folder.

## Disclaimer

If you happen to be taking the same class, please do not cheat off of this. There's a good chance that this code doesn't work very well anyways.

To quote the man himself directly: "If you try to use this for homework, you can be certain that your instructor will find the matching code here and bust you. But your instructor may not be able to bust me because I might have graduated already. Ha-ha!"

## TODO
* Fix the bugs... does this thing even work most of the time?
* Switch to C++17, with smart pointers, enum classes, and variants
* Actually write the interpreter
* Write code generation for x86
