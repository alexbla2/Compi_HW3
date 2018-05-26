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
extern stack<SymbolTable> TableStack; 

extern int yylineno;


#define YYSTYPE Node*

class Type;

class Symbol{
    public:
    string name;
    Type type;											//now is Type class and not a string
    int offset;
    bool isFunc;
    std::vector<string> args;
    string ret;
    
	Symbol(): name(""), type(Type()), offset(0), isFunc(false){}
    
	Symbol(const Symbol& sym) : name(sym.name), type(sym.type), offset(sym.offset),
		isFunc(sym.isFunc), ret(sym.ret), args(sym.args) {}
	
    Symbol(string name, Type type ,int offset) : name(name), type(type),
		offset(offset), isFunc(false) {}
    
	Symbol(string name, Type type ,int offset,std::vector<string>& args,string ret ) : 
		name(name), type(type), offset(offset),args(args),ret(ret), isFunc(true) {}
};

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
	Type type;						//now is Type class and not a string
  
	Int() : type(Type("INT")) {}

};

class Byte : public Node {
	public:
	Type type;						//now is Type class and not a string

	Byte() : type(Type("BYTE")) {}
	
};

class B : public Node {};

class Bool : public Node {
	public:
	Type type;					//now is Type class and not a string

	Bool() : type(Type("BOOL")) {}
	
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
    Type type;								//now is Type class and not a string
    int value;
	
    Num(char* yytext) : type(Type("INT")), value(atoi(yytext)) {}
	
};

class String : public Node {
    public:
    Type type;								//now is Type class and not a string
    string value;
	
    String(char* yytext) :	 type(Type("STRING")), value(string(yytext)) {}
		
};

//the class is now more complicated - need to update its CONS accord.
class Type : public Node {
	public:
	string type;
	int size; 														//new val added to TYPE
	bool isArray;													//new val added to TYPE
	
	Type(){};
	Type(string type): type(type),size(0),isArray(false){}			//cons for name only (not an array)
	Type(const Type& Ctype) : type(Ctype.type), size(Ctype.size), isArray(Ctype.isArray){}		//Copy cons added
	Type(Int* type,int typeSize,bool typeIsArray);			//need to change type cons
	Type(Byte* type,int typeSize,bool typeIsArray);
	Type(Bool* type,int typeSize,bool typeIsArray);
};

class Exp : public Node {
	public:
	Type type;												//now has a TYPE *class* - and not string !! (need to change accord)

	Exp(){};
	Exp(String* s);
	Exp(Exp* exp);
	Exp(Exp* exp1, Binop* binop, Exp* exp2);
	Exp(Id* id,Exp* exp);
	Exp(Id* id,Exp* exp,bool isArray);						//new cons for Array exp !
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
	std::vector<Type> types;							//vector of type classes

	ExpList() : types( std::vector<Type>() ) {}
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
	Statement(Id* id, Exp* expression1,Exp* expression2);		//cons for Array assignment (example: a[7]=4)
	Statement(Exp* expression, Statement* statement);
	Statement(Type* type, Id* id, Exp* expression);
	Statement(Type* type, Id* id, Num* num);					//new cons for Array statement
	Statement(Type* type, Id* id, Num* num, B* byte); 			//new cons for Array statement
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
	Type type;											//now is Type class and not a string
	string id;

	FormalDecl(){}
	FormalDecl(Type* type, Id* id);

	//those two are only array option so we dont send a flag with them
	FormalDecl(Type* type, Id* id,Num* num);						//cons for array : bool a[5]; 5 is num 
	FormalDecl(Type* type, Id* id,Num* num, B* byte );				//cons for array : bool a[5]; 5 is numB
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
	Type type;							//now is Type class and not a string

	RetType(){}
	RetType(Void* voidNode);
	RetType(Type* type);
};

class Func : public Node {
	public:
	string id;
	Type ret;						//now is Type class and not a string
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


