#include "parser.hpp"
using namespace output;

void StacksInit(stack<SymbolTable>& StackTable, stack<int>& OffsetStack) {
	SymbolTable global;
	StackTable.push(global);
	OffsetStack.push(0);
	
	std::vector<string> PrintargTypes;
	std::vector<string> PrintIargTypes;

	PrintargTypes.push_back("STRING");
	PrintIargTypes.push_back("INT");

	string s1=(makeFunctionType("VOID", PrintargTypes));
	string s2=(makeFunctionType("VOID", PrintIargTypes));
	string ret1= "VOID";

	Symbol sym1("print", s1, OffsetStack.top(), PrintargTypes, ret1);		//maybe need to send a & ?
	Symbol sym2("printi", s2, OffsetStack.top(), PrintIargTypes, ret1);
	
	StackTable.top().push_back(sym1);
	StackTable.top().push_back(sym2);
	
}


SymbolTable::iterator findId(SymbolTable& scope, string id) {
  
  for (SymbolTable::iterator it = scope.begin(); it != scope.end(); it++) {
	if (it->name == id) 
		return it;
  }
  return scope.end();
}


bool symDeclared(stack<SymbolTable>& StackTable, Id* id) {
	
	stack<SymbolTable> stackCopy = StackTable;
	SymbolTable currentScope;
	string idName= id->text;
	while ( stackCopy.size() ) {
		currentScope = stackCopy.top();
		if (findId(currentScope, idName) != currentScope.end()) 
			return true;
		stackCopy.pop();
	}
	return false;
}


void addScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack) {
	  
	OffsetStack.push(OffsetStack.top());
	SymbolTable newScope;
	StackTable.push(newScope);
}

void printScope(SymbolTable& scope){
  
  endScope();
  for (SymbolTable::iterator it = scope.begin(); it != scope.end(); it++)
    printID(it->name, it->offset, it->type);
}

//need TODO
void addFormalsToScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack,
                       Formals* formals, int lineno) {
  
  int offset = -1;
  
  for (vector<FormalDecl*>::reverse_iterator it = formals->formals.rbegin();
       it != formals->formals.rend(); it++) {

		Id id( (*it)->id );
		if (symDeclared(StackTable, &id)) {
			errorDef(lineno, id.text);
			exit(0);
		}
		Symbol sym( (*it)->id, (*it)->type, offset);	
		StackTable.top().push_back(sym);
		offset--;		
  }
}

void newVarScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack,
                        string type, Id* id, int lineno) {
 
	if (symDeclared(StackTable, id)) {
		errorDef(lineno, id->text);
		exit(0);
	}
	
	Symbol sym(id->text, type, OffsetStack.top());    //???????????????
	StackTable.top().push_back(sym);
	
	int newOffset = OffsetStack.top() + 1;
	OffsetStack.pop();
	OffsetStack.push(newOffset);
}


void addFuncToScope(stack<SymbolTable>& StackTable, stack<int>& OffsetStack,
                   RetType* ret, Id* id, Formals* formals, int lineno) {

	if (symDeclared(StackTable, id)) {  //func is already declared
		errorDef(lineno, id->text);
		exit(0);
	}

	std::vector<string> argTypes;
	for (vector<FormalDecl*>::reverse_iterator it = formals->formals.rbegin();
	   it != formals->formals.rend(); it++) {
			argTypes.push_back((*it)->type);
	}
	string funcType=makeFunctionType(ret->type,argTypes);
	Symbol sym(id->text, funcType, OffsetStack.top(),argTypes, ret->type);
	StackTable.top().push_back(sym);
}

bool isArray(string name){
	int pos1=name.find("[");
	return (pos1 != -1);
}


Type getIdType(stack<SymbolTable>& StackTable, Id* id) {
  
	stack<SymbolTable> stackCopy = StackTable;
	string name = id->text;
	SymbolTable::iterator it;
	while ( stackCopy.size() ) {
	 
	it = findId(stackCopy.top(), name);
	if (it != stackCopy.top().end()) 
		return it->type;
	stackCopy.pop();
	}

	return Type();
}

Symbol getIdSymbol(stack<SymbolTable>& StackTable, string id) {

	stack<SymbolTable> stackCopy = StackTable;
	SymbolTable::iterator it;
	while ( stackCopy.size() ) {
		
		it = findId(stackCopy.top(), id);
		if (it != stackCopy.top().end()) 
			return *it;
		stackCopy.pop();
	}
	return Symbol();
}

//TODO
bool validTypes(vector<string>& types1,vector<string>& types2){

	 if( types1.size() != types2.size() ){
		return false;
	 }

	for(int i=0; i < types1.size(); i++){

		if(types1[i] != types2[i]){
			if(types1[i] == "INT" && types2[i] == "BYTE")
				continue;
			//array case
			int pos1 = types1[i].find("INT");
			int pos2=types2[i].find("BYTE");
			if(pos1 == -1 || pos2 == -1){
				return false;
			}
			pos1=types1[i].find("[");
			pos2=types2[i].find("[");
			if(pos1 == -1 || pos2 == -1){
				return false;
			}
			string temp1=types1[i].substr(pos1);
			string temp2=types2[i].substr(pos2);
			if(temp1 != temp2){
				return false;
			}
		}
	}
	return true;
}

Program::Program(FuncList* funcs) {
	
	this->sons.push_back(funcs);
	this->funcs = funcs->funcs;
	Symbol mainFuncSymbol = getIdSymbol(TableStack,"main");
	
	if(mainFuncSymbol.name!="main"){
		errorMainMissing();
		exit(0);
	}

	if(!(mainFuncSymbol.args.empty())){
		errorMainMissing();
		exit(0);
	}
	
	if(mainFuncSymbol.ret !="VOID"){
		errorMainMissing();
		exit(0);
	}
}


FuncList::FuncList() : funcs( vector<Func*>() ){}  // for epsilon rule


FuncList::FuncList(FuncList* list, Func* func) : funcs( vector<Func*>(list->funcs) ){
  
	this->sons.push_back(list);
	this->sons.push_back(func);
	this->funcs.push_back(func);
}


Func::Func(RetType* ret, Id* id, Formals* formals, Statements* statements) :
id(id->text), ret(ret->type), formals(formals) {
	
	this->sons.push_back(ret);
	this->sons.push_back(id);
	this->sons.push_back(formals);
	this->sons.push_back(statements);
}


RetType::RetType(Void* voidNode) : type("Void"){
	
	this->sons.push_back(voidNode);
}


RetType::RetType(Type* t) : type(t->type){
	
	this->sons.push_back(t);
}


Formals::Formals(FormalsList* formalsList) : formals(formalsList->list){
	
	this->sons.push_back(formalsList);
}


FormalsList::FormalsList(FormalDecl* formalDecl) : list(vector<FormalDecl*>()){
	
	this->sons.push_back(formalDecl);
	this->list.push_back(formalDecl);
}


FormalsList::FormalsList(FormalsList* formalsList, FormalDecl* formalsDecl) :
list(vector<FormalDecl*>(formalsList->list)) {
  
	this->sons.push_back(formalsList);
	this->sons.push_back(formalsDecl);
	this->list.push_back(formalsDecl);
}


FormalDecl::FormalDecl(Type* t, Id* id) : type(t->type), id(id->text){
  
	this->sons.push_back(t);
	this->sons.push_back(id);
}

//added for array
FormalDecl::FormalDecl(Type* t, Id* id,Num* num) {
	this->sons.push_back(t);
	this->sons.push_back(id);
	this->sons.push_back(num);

	if(num->value <1 || num->value >255){
		errorInvalidArraySize(yylineno,id->text);
		exit(0);
	}
	this->type = makeArrayType(t->type,num->value);
}

FormalDecl::FormalDecl(Type* t, Id* id,Num* num, B* byte ){
	this->sons.push_back(t);
	this->sons.push_back(id);
	this->sons.push_back(num);

	if(num->value <1 || num->value >255){
		output::errorInvalidArraySize(yylineno,id->text);
		exit(0);
	}

	this->type = makeArrayType(t->type,num->value);
}





Statements::Statements(Statement* statement) {
	
	this->sons.push_back(statement);
}


Statements::Statements(Statements* statements, Statement* statement) {
 
	this->sons.push_back(statements);
	this->sons.push_back(statement);
}


Statement::Statement(Statements* statements) {

	this->sons.push_back(statements);
}


Statement::Statement(Exp* expression) { 

	this->sons.push_back(expression);
}


Statement::Statement(Call* call) {
	
	this->sons.push_back(call);
}


Statement::Statement(Type* type, Id* id) {
  
	this->sons.push_back(type);
	this->sons.push_back(id);
	
	if (symDeclared(TableStack, id)) {			//maybe export into outer func
		errorDef(yylineno, id->text);
		exit(0);
	}
}

//array
Statement::Statement(Type* type, Id* id, Num* num){
	this->sons.push_back(type);
	this->sons.push_back(id);

	if(num->value <1 || num->value >255){
		output::errorInvalidArraySize(yylineno,id->text);
		exit(0);
	}

	if (symDeclared(TableStack, id)) {			//maybe export into outer func
		errorDef(yylineno, id->text);
		exit(0);
	}
}

Statement::Statement(Type* type, Id* id, Num* num,B* b){
	this->sons.push_back(type);
	this->sons.push_back(id);
	this->sons.push_back(b);

	if(num->value <1 || num->value >255){
		output::errorInvalidArraySize(yylineno,id->text);
		exit(0);
	}

	if (symDeclared(TableStack, id)) {			//maybe export into outer func
		errorDef(yylineno, id->text);
		exit(0);
	}
}




Statement::Statement(Id* id, Exp* expression) {
	
	this->sons.push_back(id);
	this->sons.push_back(expression);
	
	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}
	
	string idType = getIdType(TableStack, id).type;	
	if ( idType != expression->type ) {
	
		if ( idType == "INT" && expression->type == "BYTE") {
		  // byte assignment to int is ok
		} 
		else {
			errorMismatch(yylineno);
			exit(0);
		}
	}
}

Statement::Statement(Id* id, Exp* expression1,Exp* expression2){
	this->sons.push_back(id);
	this->sons.push_back(expression1);
	this->sons.push_back(expression2);

	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}

	string idType = getIdType(TableStack, id).type;	//is array!
	size_t pos=idType.find("[");
	string idTemp= idType.substr(0,pos);
	if ( idTemp != expression2->type ) {
	
		if ( idTemp == "INT" && expression2->type == "BYTE") {
		  // byte assignment to int is ok
		} 
		else {
			errorMismatch(yylineno);
			exit(0);
		}
	}
}


Statement::Statement(Exp* expression, Statement* statement) {
	
	this->sons.push_back(expression);
	this->sons.push_back(statement);
}


Statement::Statement(Type* type, Id* id, Exp* expression) {
	
	this->sons.push_back(type);
	this->sons.push_back(id);
	this->sons.push_back(expression);
	
	if (symDeclared(TableStack, id)) {
		errorDef(yylineno, id->text);
		exit(0);
	}
	
	if (type->type != expression->type) {
		
		if (type->type == "INT" && expression->type == "BYTE") {
		  // byte assignment to int is ok TODO WHAT ABOUT ARRAYS
		} 
		else {
			errorMismatch(yylineno);
			exit(0);
		}
	}
}


Statement::Statement(Exp* expression, Statement* ifStatment,
                     Statement* elseStatement) {
  
	this->sons.push_back(expression);
	this->sons.push_back(ifStatment);
	this->sons.push_back(elseStatement);
}


ExpList::ExpList(Exp* expression) {

	this->sons.push_back(expression);
	this->types = vector<string>();
  	this->types.push_back(expression->type);
}


ExpList::ExpList(Exp* expression, ExpList* expList){
  
	this->sons.push_back(expression);
	this->sons.push_back(expList);

	this->types = vector<string>();
	this->types.push_back(expression->type);
	for(int i=0; i<expList->types.size(); i++)
		this->types.push_back(expList->types[i]);
}


Call::Call(Id* id) {
	
	this->sons.push_back(id);
	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}

	Symbol sym = getIdSymbol(TableStack, id->text);
	if (!(sym.isFunc)) {
		errorUndefFunc(yylineno, id->text);
		exit(0);
	} 
	else if (!sym.args.empty()) {
		errorPrototypeMismatch(yylineno, id->text, sym.args);
		exit(0);
	}
	this->id = id->text;
}


Call::Call(Id* id, ExpList* expList) {
	
	this->sons.push_back(id);
	this->sons.push_back(expList);
	
	if (symDeclared(TableStack, id)) {
		Symbol sym = getIdSymbol(TableStack, id->text);
		if (!(sym.isFunc)) {								//make outer func
			errorUndefFunc(yylineno, id->text);
			exit(0);
		} 
		if (!validTypes(sym.args,expList->types)) {
			/*
			cout<<"expecting:"<<endl;
			for (vector<string>::iterator i = s.args.begin();
			   i != s.args.end(); ++i) {
				cout<<(*i)<<endl;
			}
			cout<<"found:"<<endl;
			for (vector<string>::iterator i = expList->types.begin();
			   i != expList->types.end(); ++i) {
				cout<<(*i)<<endl;
			}*/
			errorPrototypeMismatch(yylineno, id->text, sym.args);
			exit(0);
		}
		
	} else {
		errorUndefFunc(yylineno, id->text);
		exit(0);
	}
	
	this->id = id->text;
}

Exp::Exp() {
	type="";
}

Exp::Exp(Id* id,Exp* exp){
	this->sons.push_back(id);
	this->sons.push_back(exp);
	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}
}


Exp::Exp(String* expression,bool isAprintFunc) {
	if (!isAprintFunc) {
		errorMismatch(yylineno);
		exit(0);
	}
	this->sons.push_back(expression);
	this->type = "STRING";
}


Exp::Exp(Num* expression) : type("INT"){
	
  this->sons.push_back(expression);
}


Exp::Exp(Exp* expression) : type(expression->type){
	
  this->sons.push_back(expression);
}


Exp::Exp(Exp* expression1, Binop* binop, Exp* expression2) {
	
	this->sons.push_back(expression1);
	this->sons.push_back(binop);
	this->sons.push_back(expression2);
	
	if ((expression1->type != "INT" && expression1->type != "BYTE") ||
	  (expression2->type != "INT" && expression2->type != "BYTE")) {
		errorMismatch(yylineno);
		exit(0);
	}
	
	if ("INT" == expression1->type || "INT" == expression2->type) {
		this->type = "INT";
	} 
	else {
		this->type = "BYTE";
	}
}


Exp::Exp(Id* id) {
  
	this->sons.push_back(id);
	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}
	
	this->type = getIdType(TableStack, id).type;
}


Exp::Exp(Call* call) {
	
	this->sons.push_back(call);
	this->type = getIdSymbol(TableStack, call->id).ret;
}


Exp::Exp(Num* num, B* byte) {
	
	this->sons.push_back(num);
	this->sons.push_back(byte);
	this->type = "BYTE";
	
	if((num->value)>255){
		stringstream s;
		s << num->value;
		errorByteTooLarge(yylineno,string(s.str()));
		exit(0);
	}
}


Exp::Exp(False* f) : type("BOOL"){
 
	this->sons.push_back(f);
}


Exp::Exp(True* t) : type("BOOL"){
 
	this->sons.push_back(t);
}


Exp::Exp(Not* notOp, Exp* expression2) : type("BOOL"){
 
	this->sons.push_back(notOp);
	this->sons.push_back(expression2);
	
	if ((expression2->type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}
}


Exp::Exp(Exp* expression1, And* andOp, Exp* expression2) 
	: type("BOOL"){
 
	this->sons.push_back(expression1);
	this->sons.push_back(andOp);
	this->sons.push_back(expression2);
	
	if ((expression1->type != "BOOL") || (expression2->type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}
}


Exp::Exp(Exp* expression1, Or* orOp, Exp* expression2)
	: type("BOOL"){
 
	this->sons.push_back(expression1);
	this->sons.push_back(orOp);
	this->sons.push_back(expression2);
	
if ((expression1->type != "BOOL") || (expression2->type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}

}

Exp::Exp(Exp* expression1, Relop* relop, Exp* expression2) 
	: type("BOOL"){
 
	this->sons.push_back(expression1);
	this->sons.push_back(relop);
	this->sons.push_back(expression2);
	if ((expression1->type != "INT" && expression1->type != "BYTE") ||
	  (expression2->type != "INT" && expression2->type != "BYTE")) {
		errorMismatch(yylineno);
		exit(0);
	}

}