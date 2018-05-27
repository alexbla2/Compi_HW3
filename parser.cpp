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

	Type sym1Type(makeFunctionType("VOID", PrintargTypes));
	Type sym2Type(makeFunctionType("VOID", PrintIargTypes));
	Type returnType("VOID");

	Symbol sym1("print", sym1Type, OffsetStack.top(), PrintargTypes, returnType);		//maybe need to send a & ?
	Symbol sym2("printi", sym2Type, OffsetStack.top(), PrintIargTypes, returnType);
	
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
    printID(it->name, it->offset, it->type.type);
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
                        Type* type, Id* id, int lineno) {
 
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
		   if((*it)->type.isArray){
				argTypes.push_back(makeArrayType((*it)->type.type,(*it)->type.size));
			}else{
				argTypes.push_back((*it)->type.type);
			}
	}
	Type funcType(makeFunctionType(ret->type.type, argTypes)); //only RetType ret type
	Symbol sym(id->text, funcType, OffsetStack.top(),argTypes, ret->type);
	StackTable.top().push_back(sym);
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
bool validTypes(vector<string>& types,vector<Type*>& types2){

	vector<string> newVec;
	string temp;
	int byteArray[types2.size()];
	int i=0;
	for (vector<Type*>::iterator it = types2.begin();
       it != types2.end(); it++) {
		   if((*it)->isArray){
			   temp = makeArrayType((*it)->type,(*it)->size);
		   }
		   else{
			   	temp = (*it)->type;
		   }
		   newVec.push_back(temp);

		   if((*it)->type == "BYTE" && (*it)->isArray ==true){
			   byteArray[i]=(*it)->size;
		   }else{
			   byteArray[i]=0;
		   }
		   i++;
	}
	 if( types.size() != newVec.size() ){
		return false;
	 }
	for(int i=0; i < types.size(); i++){

		if(types[i] != newVec[i]){
			if(types[i] == "INT" && newVec[i] == "BYTE")
				continue;
			//array case
			int stringSize=types[i].size();
			int stringSize2=newVec[i].size();
			if(types[i][0] == 'I' && newVec[i][0] == 'B' && newVec[i][1] == 'Y'){
				for(int j=4; j<stringSize, j+1 <stringSize2; j++){
					if(types[i][j] != newVec[i][j+1]){
						return false;
					} 
				}
			}
			return false;
		}
	}
	return true;
}


Type::Type(Int* type,int typeSize,bool typeIsArray) : type("INT"),size(typeSize),isArray(typeIsArray){
	this->sons.push_back(type);
}


Type::Type(Bool* type,int typeSize,bool typeIsArray) : type("BOOL"),size(typeSize),isArray(typeIsArray){
	this->sons.push_back(type);
}


Type::Type(Byte* type,int typeSize,bool typeIsArray) : type("BYTE"),size(typeSize),isArray(typeIsArray){
	this->sons.push_back(type);
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
	
	if(mainFuncSymbol.ret.type!="VOID"){
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


RetType::RetType(Type* t) : type(*t){
	
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


FormalDecl::FormalDecl(Type* t, Id* id) : type(*t), id(id->text){
  
	this->sons.push_back(t);
	this->sons.push_back(id);
}

//added for array
FormalDecl::FormalDecl(Type* t, Id* id,Num* num) {
	this->sons.push_back(t);
	this->sons.push_back(id);
	this->sons.push_back(num);

	if(num->value <1 || num->value >255){
		output::errorInvalidArraySize(yylineno,id->text);
		exit(0);
	}

	this->type.type=t->type;
	this->type.size=num->value;
	this->type.isArray=true;
}

FormalDecl::FormalDecl(Type* t, Id* id,Num* num, B* byte ){
	this->sons.push_back(t);
	this->sons.push_back(id);
	this->sons.push_back(num);

	if(num->value <1 || num->value >255){
		output::errorInvalidArraySize(yylineno,id->text);
		exit(0);
	}

	this->type.type=t->type;
	this->type.size=num->value;
	this->type.isArray=true;
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

Statement::Statement(Id* id, Exp* expression1,Exp* expression2){
	this->sons.push_back(id);
	this->sons.push_back(expression1);
	this->sons.push_back(expression2);

	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}

	string idType = getIdType(TableStack, id).type;	//is array!
	if ( idType != expression2->type.type && expression2->type.isArray == false ) {
	
		if ( idType == "INT" && expression2->type.type == "BYTE") {
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
	
	if (type->type != expression->type.type) {
		
		if (type->type == "INT" && expression->type.type == "BYTE") {
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


ExpList::ExpList(Exp* expression) : types(vector<Type*>()){

	this->sons.push_back(expression);
	this->types = vector<Type*>();
  	this->types.push_back(&expression->type);
}


ExpList::ExpList(Exp* expression, ExpList* expList) :
	types(vector<Type*>()){
  
	this->sons.push_back(expression);
	this->sons.push_back(expList);

	this->types.push_back(&expression->type);
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


Exp::Exp(String* expression) {
	/*TODO
	if (allowStrings != true) {
	errorMismatch(yylineno);
	exit(0);
	}*/
	this->sons.push_back(expression);
	Type type1(expression->type);
	this->type = type1;
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
	
	if ((expression1->type.type != "INT" && expression1->type.type != "BYTE") ||
	  (expression2->type.type != "INT" && expression2->type.type != "BYTE")) {
		errorMismatch(yylineno);
		exit(0);
	}
	
	if ("INT" == expression1->type.type || "INT" == expression2->type.type) {
		this->type.type = "INT";
	} 
	else {
		this->type.type = "BYTE";
	}
}


Exp::Exp(Id* id) {
  
	this->sons.push_back(id);
	if (!symDeclared(TableStack, id)) {
		errorUndef(yylineno, id->text);
		exit(0);
	}
	
	this->type = getIdType(TableStack, id);
}


Exp::Exp(Call* call) {
	
	this->sons.push_back(call);
	this->type = getIdSymbol(TableStack, call->id).ret;
}


Exp::Exp(Num* num, B* byte) {
	
	this->sons.push_back(num);
	this->sons.push_back(byte);
	this->type = Type("BYTE");
	
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
	
	if ((expression2->type.type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}
}


Exp::Exp(Exp* expression1, And* andOp, Exp* expression2) 
	: type("BOOL"){
 
	this->sons.push_back(expression1);
	this->sons.push_back(andOp);
	this->sons.push_back(expression2);
	
	if ((expression1->type.type != "BOOL") || (expression2->type.type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}
}


Exp::Exp(Exp* expression1, Or* orOp, Exp* expression2)
	: type("BOOL"){
 
	this->sons.push_back(expression1);
	this->sons.push_back(orOp);
	this->sons.push_back(expression2);
	
if ((expression1->type.type != "BOOL") || (expression2->type.type != "BOOL")) {
		errorMismatch(yylineno);
		exit(0);
	}

}

Exp::Exp(Exp* expression1, Relop* relop, Exp* expression2) 
	: type("BOOL"){
 
	this->sons.push_back(expression1);
	this->sons.push_back(relop);
	this->sons.push_back(expression2);
	if ((expression1->type.type != "INT" && expression1->type.type != "BYTE") ||
	  (expression2->type.type != "INT" && expression2->type.type != "BYTE")) {
		errorMismatch(yylineno);
		exit(0);
	}

}