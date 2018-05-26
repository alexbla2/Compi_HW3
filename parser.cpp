#include "parser.hpp"


void initializeStacks(stack<SymbolTable>& StackTable, stack<int>& OffsetStack) {
	allowStrings = false;  //??????????
	
	SymbolTable global;
	StackTable.push(global);
	offsetStack.push(0);
	
	std::vector<string> argTypesForPrint;
	std::vector<string> argTypesForPrinti;

	argTypesForPrint.push_back("STRING");
	argTypesForPrinti.push_back("INT");
	
	Symbol s1("print", makeFunctionType("VOID", argTypesForPrint), offsetStack.top(),
		   argTypesForPrint, "VOID");
	Symbol s2("printi", makeFunctionType("VOID", argTypesForPrinti),
			 offsetStack.top(), argTypesForPrinti, "VOID");
	
	StackTable.top().push_back(s1);
	StackTable.top().push_back(s2);
	
}


SymbolTable::iterator findId(SymbolTable& scope, string id) {
  
  for (SymbolTable::iterator it = scope.begin(); it != scope.end(); it++) {
   
	if (it->name == id) 
		return it;
    
  }
  
  return scope.end();
}


bool symDeclared(stack<SymbolTable>& scopes, Id* id) {
	
	stack<SymbolTable> scopesCopy = scopes;
	SymbolTable currentScope;
	while ( scopesCopy.size() ) {
		
		currentScope = scopesCopy.top();
		if (findId(currentScope, (id->text)) != currentScope.end()) 
			return true;
		
		scopesCopy.pop();
	}
	
	return false;
}


void addScope(stack<SymbolTable>& scopes, stack<int>& offsets) {
	  
	offsets.push(offsets.top());
	
	SymbolTable newScope;
	scopes.push(newScope);

}


void printScope(SymbolTable& scope){
  
  endScope();
  
  for (SymbolTable::iterator it = scope.begin(); i != scope.end(); it++)
    printID(it->name, it->offset, it->type);
 
}


void addFormalsToScope(stack<SymbolTable>& scopes, stack<int>& offsets,
                       Formals* formals, int lineno) {
  
  int offset = -1;
  
  for (vector<FormalDecl*>::reverse_iterator it = formals->formals.rbegin();
       it != formals->formals.rend(); it++) {

		Id id( (*it)->id );
		if (symDeclared(scopes, &id)) {
			errorDef(lineno, id.text);
			exit(0);
		}
	
		Symbol s( (*it)->id, (*it)->type, offset);	
		scopes.top().push_back(s);
		offset--;		
  }
  
}


void addVarToScope(stack<SymbolTable>& scopes, stack<int>& offsets,
                        Type* type, Id* id, int lineno) {
 
	if (symDeclared(scopes, id)) {
	errorDef(lineno, id->text);
	exit(0);
	}
	
	Symbol s(id->text, type->type, offsets.top());
	scopes.top().push_back(s);
	
	int offset = offsets.top() + 1;
	offsets.pop();
	offsets.push(offset);
}


void addFuncToScope(stack<SymbolTable>& scopes, stack<int>& offsets,
                   RetType* ret, Id* id, Formals* formals, int lineno) {

	if (symDeclared(scopes, id)) {
	errorDef(lineno, id->text);
	exit(0);
	}

	std::vector<string> argTypes;
	for (vector<FormalDecl*>::reverse_iterator it = formals->formals.rbegin();
	   it != formals->formals.rend(); it++) 
	argTypes.push_back((*it)->type);

	string funcType = makeFunctionType(ret->type, argTypes);
	Symbol s(id->text, funcType, offsets.top(),argTypes, ret->type);
	scopes.top().push_back(s);
}


string getIdType(stack<SymbolTable>& scopes, Id* id) {
  
	stack<SymbolTable> auxScopes = scopes;
	string name = id->text;
	SymbolTable::iterator iter;

	while ( auxScopes.size() ) {
	 
	iter = findId(auxScopes.top(), name);
	if (iter != auxScopes.top().end()) 
		return iter->type;

	auxScopes.pop();
	}

	return "";
}

Symbol getIdSymbol(stack<SymbolTable>& scopes, string id) {

	stack<SymbolTable> auxScopes = scopes;
	string name = id->text;
	SymbolTable::iterator iter;

	while ( auxScopes.size() ) {
		
		iter = findId(auxScopes.top(), id);
		if (iter != auxScopes.top().end()) 
			return *iter;
		
		auxScopes.pop();
	}
	
	return Symbol();
}


bool validTypes(vector<string>& types1,vector<string>& types2){
  
	if( types1.size() != types2.size() )
		return false;

	for(int i=0; i < types1.size(); i++){
		
		if(types1[i]!=types2[i]){
			if(types1[i] == "INT" && types2[i] == "BYTE")
				continue;
		
			return false;
		}
	}

	return true;
}


Type::Type(Int* type) : type("INT"),size(0),isArray(false){
	
	this->children.push_back(type);
}


Type::Type(Bool* type) : type("BOOL"),size(0),isArray(false){
  
	this->children.push_back(type);
}


Type::Type(Byte* type) : type("BYTE"),size(0),isArray(false){
  
	this->children.push_back(type);
}


Program::Program(FuncList* funcs) {
	
	this->children.push_back(funcs);
	this->funcs = funcs->funcs;
	Symbol mainFuncSymbol = getIdSymbol(scopes,"main");
	
	if(mainFuncSymbol.name!="main"){
		errorMainMissing();
		exit(0);
	}

	if(!(mainFuncSymbol.args.empty())){
		errorMainMissing();
		exit(0);
	}
	
	if(mainFuncSymbol.ret!="VOID"){
		errorMainMissing();
		exit(0);
	}
}


FuncList::FuncList() : funcs( vector<Func*>() ){}  // for epsilon rule


FuncList::FuncList(FuncList* list, Func* func) : funcs( vector<Func*>(list->funcs) ){
  
	this->children.push_back(list);
	this->children.push_back(func);
	this->funcs.push_back(func);
}


Func::Func(RetType* ret, Id* id, Formals* formals, Statements* statements) :
id(id->text), ret(ret->type), formals(formals) {
	
	this->children.push_back(ret);
	this->children.push_back(id);
	this->children.push_back(formals);
	this->children.push_back(statements);
}


RetType::RetType(Void* voidNode) : type("Void"){
	
	this->children.push_back(voidNode);
}


RetType::RetType(Type* t) : type(t){
	
	this->children.push_back(type);
}


Formals::Formals(FormalsList* formalsList) : formals(formalsList->list){
	
	this->children.push_back(formalsList);
}


FormalsList::FormalsList(FormalDecl* formalDecl) : list(vector<FormalDecl*>()){
	
	this->children.push_back(formalDecl);
	this->list.push_back(formalDecl);
}


FormalsList::FormalsList(FormalsList* formalsList, FormalDecl* formalsDecl) :
list(vector<FormalDecl*>(formalsList->list) {
  
	this->children.push_back(formalsList);
	this->children.push_back(formalsDecl);
	this->list.push_back(formalsDecl);
}


FormalDecl::FormalDecl(Type* t, Id* id) : type(t), id(id->text{
  
	this->children.push_back(type);
	this->children.push_back(id);
}


Statements::Statements(Statement* statement) {
	
	this->children.push_back(statement);
}


Statements::Statements(Statements* statements, Statement* statement) {
 
	this->children.push_back(statements);
	this->children.push_back(statement);
}


Statement::Statement(Statements* statements) {

	this->children.push_back(statements);
}


Statement::Statement(Exp* expression) { 

	this->children.push_back(expression);
}


Statement::Statement(Call* call) {
	
	this->children.push_back(call);
}


Statement::Statement(Type* type, Id* id) {
  
	this->children.push_back(type);
	this->children.push_back(id);
	
	if (symDeclared(scopes, id)) {
		errorDef(yylineno, id->text);
		exit(0);
	}
}


Statement::Statement(Id* id, Exp* expression) {
	
	this->children.push_back(id);
	this->children.push_back(expression);
	
	if (!symDeclared(scopes, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}
	
	string idType = getIdType(scopes, id);	
	if ( idType != expression->type.type ) {
	
		if ( idType == "INT" && expression->type.type == "BYTE") {
		  // byte assignment to int is ok
		} 
		else {
			errorMismatch(yylineno);
			exit(0);
		}
	}
}


Statement::Statement(Exp* expression, Statement* statement) {
	
	this->children.push_back(expression);
	this->children.push_back(statement);
}


Statement::Statement(Type* type, Id* id, Exp* expression) {
	
	this->children.push_back(type);
	this->children.push_back(id);
	this->children.push_back(expression);
	
	if (symDeclared(scopes, id)) {
		errorDef(yylineno, id->text);
		exit(0);
	}
	
	if (type != expression->type) {
		
		if (type->type == "INT" && expression->type. == "BYTE") {
		  // byte assignment to int is ok TODO WHAT ABOUT ARRAYS
		} 
		else {
			errorMismatch(yylineno);
			exit(0);
		}
	}
}


Statement::Statement(Exp* expression, CaseList* caseList) {
	
	this->children.push_back(expression);
	this->children.push_back(caseList);
}


Statement::Statement(Exp* expression, Statement* ifStatment,
                     Statement* elseStatement) {
  
	this->children.push_back(expression);
	this->children.push_back(ifStatment);
	this->children.push_back(elseStatement);
}


CaseList::CaseList(CaseList* caseList, CaseStatement* caseStatement) {
	
	this->children.push_back(caseList);
	this->children.push_back(caseStatement);
}


CaseList::CaseList(CaseStatement* caseStatement) {
  
  this->children.push_back(caseStatement);
}


CaseStatement::CaseStatement(CaseDec* caseDec) {
  
  this->children.push_back(caseDec);
}


CaseStatement::CaseStatement(CaseDec* caseDec, Statements* statements) {
  
  this->children.push_back(caseDec);
  this->children.push_back(statements);
}


CaseDec::CaseDec(Num* num) {
	this->children.push_back(num);
}


CaseDec::CaseDec(Num* num, b* b) {
	
	this->children.push_back(num);
	this->children.push_back(b);
}


ExpList::ExpList(Exp* expression) : types(vector<string>()){

	this->children.push_back(expression);
	this->types.push_back(expression->type);
}


ExpList::ExpList(Exp* expression, ExpList* expList) :
	types(vector<string>()){
  
	this->children.push_back(expression);
	this->children.push_back(expList);

	this->types.push_back(expression->type);
	for(int i=0; i<expList->types.size(); i++)
		this->types.push_back(expList->types[i]);
	
}


Call::Call(Id* id) {
	
	this->children.push_back(id);
	if (!symDeclared(scopes, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}

	Symbol s = getIdSymbol(scopes, id->text);
	if (!(s.isFunc)) {
		errorUndefFunc(yylineno, id->text);
		exit(0);
	} 
	else if (!s.args.empty()) {
		errorPrototypeMismatch(yylineno, id->text, s.args);
		exit(0);
	}
	this->id = id->text;
}


Call::Call(Id* id, ExpList* expList) {
	
	this->children.push_back(id);
	this->children.push_back(expList);
	
	if (symDeclared(scopes, id)) {
		Symbol s = getIdSymbol(scopes, id->text);
		if (!(s.isFunc)) {
			errorUndefFunc(yylineno, id->text);
			exit(0);
		} 
		else if (!validTypes(s.args,expList->types)) {

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
			errorPrototypeMismatch(yylineno, id->text, s.args);
			exit(0);
		}
		
	} else {
		errorUndefFunc(yylineno, id->text);
		exit(0);
	}
	
	this->id = id->text;
}


Exp::Exp(String* expression) {
	/*TODO
	if (allowStrings != true) {
	errorMismatch(yylineno);
	exit(0);
	}*/
	this->children.push_back(expression);
	this->type = expression->type;
	this->type = "STRING";
}


Exp::Exp(Num* expression) : type("INT"){
	
  this->children.push_back(expression);
}


Exp::Exp(Exp* expression) : type(expression->type){
	
  this->children.push_back(expression);
}


Exp::Exp(Exp* expression1, Binop* binop, Exp* expression2) {
	
	this->children.push_back(expression1);
	this->children.push_back(binop);
	this->children.push_back(expression2);
	
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
  
	this->children.push_back(id);
	if (!symDeclared(scopes, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}
	
	this->type = Type(getIdType(scopes, id));
}


Exp::Exp(Call* call) {
	
	this->children.push_back(call);
	this->type = getIdSymbol(scopes, call->id).ret;
}


Exp::Exp(Num* num, b* byte) {
	
	this->children.push_back(num);
	this->children.push_back(byte);
	this->type = "BYTE";
	
	if((num->value)>255){
		stringstream ss;
		ss << num->value;
		errorByteTooLarge(yylineno,string(ss.str()));
		exit(0);
	}
}


Exp::Exp(False* f) : type("BOOL"){
 
	this->children.push_back(f);
}


Exp::Exp(True* t) : type("BOOL"){
 
	this->children.push_back(f);
}


Exp::Exp(Not* notOp, Exp* expression2) : type("BOOL"){
 
	this->children.push_back(notOp);
	this->children.push_back(expression2);
	
	if ((expression2->type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}
}


Exp::Exp(Exp* expression1, And* andOp, Exp* expression2) 
	: type("BOOL"){
 
	this->children.push_back(expression1);
	this->children.push_back(andOp);
	this->children.push_back(expression2);
	
	if ((expression1->type != "BOOL") || (expression2->type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}
}


Exp::Exp(Exp* expression1, Or* orOp, Exp* expression2)
	: type("BOOL"){
 
	this->children.push_back(expression1);
	this->children.push_back(orOp);
	this->children.push_back(expression2);
	
	if ((expression1->type != "BOOL") || (expression2->type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}

}

Exp::Exp(Exp* expression1, Relop* relop, Exp* expression2) 
	: type("BOOL"){
 
	this->children.push_back(expression1);
	this->children.push_back(relop);
	this->children.push_back(expression2);
	if ((expression1->type != "INT" && expression1->type != "BYTE") ||
	  (expression2->type != "INT" && expression2->type != "BYTE")) {
		errorMismatch(yylineno);
		exit(0);
	}

}