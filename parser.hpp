#ifndef H_PARSER
#define H_PARSER

#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <stack>
#include "output.hpp"

using namespace std;
class Symbol;
typedef vector<Symbol> SymbolTable;
extern stack<SymbolTable> TableStack; //TABLESTACK !!!TODO

extern int yylineno;


#define YYSTYPE Node*

class Symbol{
    public:
    string name;
    string type;
    int offset;
    bool isFunc;
    std::vector<string> args;
    string ret;
    
	Symbol(): name(""), type(""), offset(0), isFunc(false){}
    
	Symbol(const Symbol& sym) : name(sym.name), type(sym.type), offset(sym.offset),
		isFunc(sym.isFunc), ret(sym.ret), args(sym.args) {}
	
    Symbol(string name, string type ,int offset) : name(name), type(type),
		offset(offset), isFunc(false) {}
    
	Symbol(string name, string type ,int offset,std::vector<string>& args,string ret ) : 
		name(name), type(type), offset(offset),args(args),ret(ret), isFunc(true) {}
}

class Node {
	public:
	int linenum;
	std::vector<Node*> sons;
	
	Node(){
		linenum=yylineno;
	}

	virtual ~Node() {
	for (std::vector<Node*>::const_iterator it = sons.begin(); it != sons.end(); it++)
	  delete *it;
	}
};

class Void : public Node {
	public:
	string text;

	Void();
	
};

class Int : public Node {
	public:
	string type;
  
	Int() : type("INT") {}

};

class Byte : public Node {
	public:
	string type;

	Byte() : type("BYTE") {}
	
};

class B : public Node {};

class Bool : public Node {
	public:
	string type;

	Bool() : type("BOOL") {}
	
};

class And : public Node {};
class Or : public Node {};
class Not : public Node {};
class True : public Node {};
class False : public Node {};
class Return : public Node {};
class If : public Node {};
class Else : public Node {};
class While : public Node {};
class Break : public Node {};
class Sc : public Node {};
class Comma : public Node {};
class Lparen : public Node {};
class Rparen : public Node {};
class Lbrace : public Node {};
class Rbrace : public Node {};
class Lbrack : public Node {};
class Rbrack : public Node {};
class Assign : public Node {};
class Relop : public Node {};
class Binop : public Node {};

class Id : public Node {
    public:
    string text;
	
    Id(char* yytext) : text(string(yytext)) {}
    Id(string yytext) : text(yytext) {}
	
    virtual ~Id() {}
	
};

class Num : public Node {
    public:
    string type;
    int value;
	
    Num(char* yytext) : value(atoi(yytext)), type("INT") {}
	
};

class String : public Node {
    public:
    string type;
    string value;
	
    String(char* yytext) :	value(string(yytext)), type("STRING") {}
		
};


class Type : public Node {
	public:
	string type;
	
	Type(){};
	Type(Int* type);
	Type(Byte* type);
	Type(Bool* type);
};

class Exp : public Node {
	public:
	string type;

	Exp(){};
	Exp(String* s);
	Exp(Exp* exp);
	Exp(Exp* exp1, Binop* binop, Exp* exp2);
	Exp(Id* id,Exp* exp);
	Exp(Id* id);
	Exp(Call* call);
	Exp(Num* num);
	Exp(Num* num, B* byte);
	Exp(False* f);
	Exp(True* t);
	Exp(Not* notOp, Exp* exp2);
	Exp(Exp* expression1, And* andOp, Exp* expression2);
	Exp(Exp* exp1, Or* orOp, Exp* exp2);
	Exp(Exp* exp1, Relop* relop, Exp* exp2);
};

class ExpList : public Node {
	public:
	std::vector<string> types;

	ExpList() : types( std::vector<string>() ) {}
	ExpList(Exp* expression);
	ExpList(Exp* expression, ExpList* expList);
};

class Statements;

class Statement : public Node {
	public:
	
	Statement();
	Statement(Statements* statements);
	Statement(Call* call);
	Statement(Exp* expression);
	Statement(Type* type, Id* id);
	Statement(Id* id, Exp* expression);
	Statement(Id* id, Exp* expression1,Exp* expression2);
	Statement(Exp* expression, Statement* statement);
	Statement(Type* type, Id* id, Exp* expression);
	Statement(Type* type, Id* id, Num* num);
	Statement(Type* type, Id* id, Num* num, B* byte);
	Statement(Exp* expression, Statement* ifStatment, Statement* elseStatement);
};

class Statements : public Node {
	public:

	Statements();
	Statements(Statement* statement);
	Statements(Statements* statements, Statement* statement); //recursion

};

class Call : public Node {
	public:
	string id;

	Call(){}
	Call(Id* id);
	Call(Id* id, ExpList* expList);
};

class FormalDecl : public Node {
	public:
	string type;
	string id;

	FormalDecl(){}
	FormalDecl(Type* type, Id* id);
	FormalDecl(Type* type, Id* id,Num* num);
	FormalDecl(Type* type, Id* id,Num* num, B* byte );
};

class FormalsList : public Node {
	public:
	std::vector<FormalDecl*> list;
	
	FormalsList(){}
	FormalsList(FormalDecl* formalDecl);
	FormalsList(FormalsList* formalsList, FormalDecl* formalsDecl); //recursion

};

class Formals : public Node {
	public:
	std::vector<FormalDecl*> formals;

	Formals();
	Formals(FormalsList* formalsList);
};

class RetType : public Node {
	public:
	string type;

	RetType(){}
	RetType(Void* voidNode);
	RetType(Type* type);
};

class Func : public Node {
	public:
	string id;
	string ret;
	Formals* formals;
	
	Func(){}
	Func(RetType* ret, Id* id, Formals* formals, Statements* statements);
};

class FuncList : public Node {
	public:
	vector<Func*> funcs;
	
	FuncList();
	FuncList(FuncList* list, Func* func);  //recursion

	virtual ~FuncList() {}
};

class Program : public Node {
	public:
	vector<Func*> funcs;

	Program(FuncList* funcs);

	virtual ~Program() {}
};


