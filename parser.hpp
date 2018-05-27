#ifndef H_PARSER
#define H_PARSER

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>
#include <stack>
#include "output.hpp"

using namespace std;
class Symbol;
typedef vector<Symbol> SymbolTable;
extern stack<SymbolTable> TableStack; 

extern int yylineno;		//extern var from lexer - keeps the current line number

#define YYSTYPE Node*

class Node {
	public:
	int linenum;
	vector<Node*> sons;
	
	Node(): linenum(yylineno){}

	virtual ~Node() {
	for (vector<Node*>::const_iterator it = sons.begin(); it != sons.end(); it++)
	  delete *it;
	}
};


class Void : public Node {
	public:
	string type;

	Void() : type("Void"){}
	
};

class Int : public Node {
	public:
	string type;						//now is Type class and not a string
  
	Int() : type("INT") {}

};

class Byte : public Node {
	public:
	string type;						//now is Type class and not a string

	Byte() : type("BYTE") {}
	
};

class b : public Node {
};

class Bool : public Node {
	public:
	string type;					//now is Type class and not a string

	Bool() : type("BOOL") {}
	
};

//the class is now more complicated - need to update its CONS accord.
class Type : public Node {
	public:
	string type;
	// int size; 														//new val added to TYPE
	// bool isArray;													//new val added to TYPE
	
	Type(): type(""){}
	Type(string t): type(t){}			//cons for name only (*not an array*)TODO SIZE(1) - V done maybe we dont need it

	Type(const Type& Ctype) : type(Ctype.type){}		//Copy cons added
	// Type(Int* t) : type("INT"),size(1),isArray(false){}
	// Type(Byte* t) : type("BYTE"),size(1),isArray(false){}
	// Type(Bool* t) : type("BOOL"),size(1),isArray(false){}

	// Type(Int* t,int typeSize,bool typeIsArray);			//need to change type cons - array cons
	// Type(Byte* t,int typeSize,bool typeIsArray);			//need to change type cons - array cons
	// Type(Bool* t,int typeSize,bool typeIsArray);			//need to change type cons - array cons
	
	// bool operator==(const Type& t) const{
	
	// 	if(this->type == t.type && this->size == t.size && this->isArray == t.isArray)		//added "arrayness" check
	// 		return true;
		
	// 	return false;
	// }
	
};

class Symbol{
    public:
    string name;
    string type;											//now is Type class and not a string
    int offset;
    bool isFunc;
    vector<string> args;
    string ret;											//changed to Type instead of strign ret type
    
	Symbol(): name(""), type(""), offset(0), isFunc(false){}
    
	Symbol(const Symbol& sym) : name(sym.name), type(sym.type), offset(sym.offset),
		isFunc(sym.isFunc), ret(sym.ret), args(sym.args) {}
	
    Symbol(string name, string type ,int offset) : name(name), type(type),
		offset(offset), isFunc(false) {}
    
	Symbol(string name, string type ,int offset,vector<string>& args,string ret ) : 		//Type ret !
		name(name), type(type), offset(offset),args(args),ret(ret), isFunc(true) {}
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
    string type;								//now is Type class and not a string
    int value;
	
    Num(char* yytext) : type("INT"), value(atoi(yytext)) {}
	
};

class String : public Node {//TODO WHY DO WE NEED THIS STRUCT??
    public:
    string type;								//now is Type class and not a string
    string value;
	
    String(const char* yytext) :	 type("STRING"), value(yytext) {}
		
};

class Exp;

class ExpList : public Node {
	public:
	vector<string> types;							//vector of type classes

	ExpList() : types( vector<string>() ) {}
	ExpList(Exp* expression);
	ExpList(Exp* expression, ExpList* expList);
};

class Call : public Node {
	public:
	string id;

	Call(){}
	Call(Id* id);
	Call(Id* id, ExpList* expList);
};

class Exp : public Node {
	public:
	string type;												//now has a TYPE *class* - and not string !! (need to change accord)

	Exp();
	Exp(String* s,bool isAPrintFunc);
	Exp(Exp* exp);
	Exp(Exp* exp1, Binop* binop, Exp* exp2);
	Exp(Id* id,Exp* exp);							//new cons for Array exp !				
	Exp(Id* id);
	Exp(Call* call);
	Exp(Num* num);
	Exp(Num* num, b* byte);
	Exp(False* f);
	Exp(True* t);
	Exp(Not* notOp, Exp* exp2);
	Exp(Exp* expression1, And* andOp, Exp* expression2);
	Exp(Exp* exp1, Or* orOp, Exp* exp2);
	Exp(Exp* exp1, Relop* relop, Exp* exp2);
};

class Statements;

class Statement : public Node {
	public:
	
	Statement(){}
	Statement(Statements* statements);
	Statement(Call* call);
	Statement(Exp* expression);
	Statement(Type* type, Id* id);
	Statement(Id* id, Exp* expression);
	Statement(Id* id, Exp* expression1,Exp* expression2);		//cons for Array assignment (example: a[7]=4)
	Statement(Exp* expression, Statement* statement);
	Statement(Type* type, Id* id, Exp* expression);
	Statement(Type* type, Id* id, Num* num);					//new cons for Array statement
	Statement(Type* type, Id* id, Num* num, b* byte); 			//new cons for Array statement
	Statement(Exp* expression, Statement* ifStatment, Statement* elseStatement);
};

class Statements : public Node {
	public:

	Statements(){}
	Statements(Statement* statement);
	Statements(Statements* statements, Statement* statement); //recursion

};

class FormalDecl : public Node {
	public:
	string type;											//now is Type class and not a string
	string id;

	FormalDecl(){}
	FormalDecl(Type* type, Id* id);

	//those two are only array option so we dont send a flag with them
	FormalDecl(Type* type, Id* id,Num* num);						//cons for array : bool a[5]; 5 is num 
	FormalDecl(Type* type, Id* id,Num* num, b* byte );				//cons for array : bool a[5]; 5 is numB
};

class FormalsList : public Node {
	public:
	vector<FormalDecl*> list;
	
	FormalsList(){}
	FormalsList(FormalDecl* formalDecl);
	FormalsList(FormalsList* formalsList, FormalDecl* formalsDecl); //recursion

};

class Formals : public Node {
	public:
	vector<FormalDecl*> formals;

	Formals(){}
	Formals(FormalsList* formalsList);
};

class RetType : public Node {
	public:
	string type;							//now is Type class and not a string

	RetType(){}
	RetType(Void* voidNode);
	RetType(Type* type);
};

class Func : public Node {
	public:
	string id;
	string ret;						//now is Type class and not a string
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

void StacksInit(stack<SymbolTable>& StackTable, stack<int>& OffsetStack) ;

void addScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack) ;

void printScope(SymbolTable& scope);
//need TODO
void addFormalsToScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack,
                       Formals* formals, int lineno); 
					   
void newVarScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack,
                        string type, Id* id, int lineno,int currentOff) ;

void addFuncToScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack,
                   RetType* ret, Id* id, Formals* formals, int lineno);
#endif
