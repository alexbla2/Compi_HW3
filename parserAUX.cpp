#include "parser.hpp"

void initializeScopes(stack<SymbolTable>& scopes, stack<int>& offsets) {
  allowStrings = false;
  SymbolTable globalScope;
  scopes.push(globalScope);
  offsets.push(0);
  std::vector<string> argTypesForPrint;
  argTypesForPrint.push_back("STRING");
  Symbol s("print", makeFunctionType("VOID", argTypesForPrint), offsets.top(),
           argTypesForPrint, "VOID");
  scopes.top().push_back(s);
  std::vector<string> argTypesForPrinti;
  argTypesForPrinti.push_back("INT");
  s = Symbol("printi", makeFunctionType("VOID", argTypesForPrinti),
             offsets.top(), argTypesForPrinti, "VOID");
  scopes.top().push_back(s);
}

void addScope(stack<SymbolTable>& scopes, stack<int>& offsets) {
  SymbolTable newScope;
  scopes.push(newScope);
  offsets.push(offsets.top());
}

void addFuncSymbol(stack<SymbolTable>& scopes, stack<int>& offsets,
                   RetType* ret, Id* id, Formals* formals, int lineno) {
  if (isDeclared(scopes, id)) {
    errorDef(lineno, id->text);
    exit(0);
  }
  std::vector<string> argTypes;
  for (vector<FormalDecl*>::reverse_iterator i = formals->formals.rbegin();
       i != formals->formals.rend(); ++i) {
    argTypes.push_back((*i)->type);
  }
  Symbol s(id->text, makeFunctionType(ret->type, argTypes), offsets.top(),
           argTypes, ret->type);
  scopes.top().push_back(s);
}

void printScope(SymbolTable& table) {
  endScope();
  for (SymbolTable::iterator i = table.begin(); i != table.end(); i++) {
    printID((*i).name, (*i).offset, (*i).type);
  }
}

void addFormalToScope(stack<SymbolTable>& scopes, stack<int>& offsets, Symbol s,
                      int lineno) {
  Id id(s.name);
  if (isDeclared(scopes, &id)) {
    errorDef(lineno, id.text);
    exit(0);
  }
  scopes.top().push_back(s);
}

void addFormalsToScope(stack<SymbolTable>& scopes, stack<int>& offsets,
                       Formals* formals, int lineno) {
  int offset = -1;
  for (vector<FormalDecl*>::reverse_iterator i = formals->formals.rbegin();
       i != formals->formals.rend(); ++i) {
    Symbol s((*i)->id, (*i)->type, offset--);
    addFormalToScope(scopes, offsets, s, lineno);
  }
}

void addVariableToScope(stack<SymbolTable>& scopes, stack<int>& offsets,
                        Type* type, Id* id, int lineno) {
  if (isDeclared(scopes, id)) {
    errorDef(lineno, id->text);
    exit(0);
  }
  Symbol s(id->text, type->type, offsets.top());
  scopes.top().push_back(s);
  int offset = offsets.top() + 1;
  offsets.pop();
  offsets.push(offset);
}

SymbolTable::iterator findId(SymbolTable& table, string id) {
  for (SymbolTable::iterator i = table.begin(); i != table.end(); ++i) {
    if ((*i).name == id) {
      return i;
    }
  }
  return table.end();
}

bool canBreak(stack<bool>& breakables) { return !(breakables.empty()); }

bool isDeclared(stack<SymbolTable>& scopes, Id* id) {
  stack<SymbolTable> scopes_copy = scopes;
  while (!scopes_copy.empty()) {
    if (findId(scopes_copy.top(), (id->text)) != scopes_copy.top().end()) {
      return true;
    }
    scopes_copy.pop();
  }
  return false;
}

string getTypeById(stack<SymbolTable>& scopes, Id* id) {
  stack<SymbolTable> scopes_copy = scopes;
  while (!scopes_copy.empty()) {
    if (findId(scopes_copy.top(), (id->text)) != scopes_copy.top().end()) {
      return (*findId(scopes_copy.top(), (id->text))).type;
    }
    scopes_copy.pop();
  }
  return "";
}

Symbol getSymbolById(stack<SymbolTable>& scopes, string id) {
  stack<SymbolTable> scopes_copy = scopes;
  while (!scopes_copy.empty()) {
    if (findId(scopes_copy.top(), id) != scopes_copy.top().end()) {
      return (*findId(scopes_copy.top(), (id)));
    }
    scopes_copy.pop();
  }
  return Symbol();
}

bool validTypes(vector<string>& types1,vector<string>& types2){
  if(types1.size()!=types2.size()){
    return false;
  }
  for(int i=0;i<types1.size();++i){
    if(types1[i]!=types2[i]){
      if((types1[i] == "INT" && types2[i] == "BYTE")){
          continue;
      }
      else{
        return false;
      }
    }
  }
  return true;
}

Program::Program(FuncList* funcs) {
  this->children.push_back(funcs);
  this->funcs = funcs->funcs;
  Symbol mainFuncSymbol = getSymbolById(scopes,"main");
  if(mainFuncSymbol.name!="main"){
    errorMainMissing();
    exit(0);
  }else if(!(mainFuncSymbol.args.empty())){
    errorMainMissing();
    exit(0);
  }else if(mainFuncSymbol.ret!="VOID"){
    errorMainMissing();
    exit(0);
  }
}

FuncList::FuncList(FuncList* list, Func* func) {
  this->children.push_back(list);
  this->children.push_back(func);
  this->funcs = vector<Func*>(list->funcs);
  this->funcs.push_back(func);
}
FuncList::FuncList() { this->funcs = vector<Func*>(); }  // for epsilon rule

Func::Func(RetType* ret, Id* id, Formals* formals, Statements* statements) {
  // we want to push all those childrens
  this->children.push_back(ret);
  this->children.push_back(id);
  this->children.push_back(formals);
  this->children.push_back(statements);
  this->id = id->text;
  this->ret = ret->type;
  this->formals = formals;
}

Void::Void() { this->text = string("VOID"); }

RetType::RetType(Void* voidNode) {
  this->children.push_back(voidNode);
  type = voidNode->text;
}
RetType::RetType(Type* type) {
  this->children.push_back(type);
  this->type = type->type;
}

Formals::Formals() {}

Formals::Formals(FormalsList* formalsList) {
  this->children.push_back(formalsList);
  this->formals = formalsList->list;
}

FormalsList::FormalsList(FormalDecl* formalDecl) {
  this->children.push_back(formalDecl);
  this->list = vector<FormalDecl*>();
  this->list.push_back(formalDecl);
}
FormalsList::FormalsList(FormalsList* formalsList, FormalDecl* formalsDecl) {
  this->children.push_back(formalsList);
  this->children.push_back(formalsDecl);
  this->list = vector<FormalDecl*>(formalsList->list);
  this->list.push_back(formalsDecl);
}

FormalDecl::FormalDecl(Type* type, Id* id) {
  this->type = type->type;
  this->id = id->text;
  this->children.push_back(type);
  this->children.push_back(id);
}

Type::Type(Int* type) {
  this->children.push_back(type);
  this->type = type->type;
}
Type::Type(Bool* type) {
  this->children.push_back(type);
  this->type = type->type;
}
Type::Type(Byte* type) {
  this->children.push_back(type);
  this->type = type->type;
}

Statements::Statements() {}

Statements::Statements(Statement* statement) {
  this->children.push_back(statement);
}
Statements::Statements(Statements* statements, Statement* statement) {
  this->children.push_back(statements);
  this->children.push_back(statement);
}

Statement::Statement() {}

Statement::Statement(Statements* statements) {
  this->children.push_back(statements);
}

Statement::Statement(Exp* expression) { this->children.push_back(expression); }

Statement::Statement(Call* call) { this->children.push_back(call); }

Statement::Statement(Type* type, Id* id) {
  this->children.push_back(type);
  this->children.push_back(id);
  if (isDeclared(scopes, id)) {
    errorDef(yylineno, id->text);
    exit(0);
  }
}

Statement::Statement(Id* id, Exp* expression) {
  this->children.push_back(id);
  this->children.push_back(expression);
  if (!isDeclared(scopes, id)) {
    errorUndef(yylineno, id->text);
    exit(0);
  }
  if (getTypeById(scopes, id) != expression->type) {
    if (getTypeById(scopes, id) == "INT" && expression->type == "BYTE") {
      // can assign byte to int
    } else {
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
  if (isDeclared(scopes, id)) {
    errorDef(yylineno, id->text);
    exit(0);
  }
  if (type->type != expression->type) {
    if (type->type == "INT" && expression->type == "BYTE") {
      // can assign byte to int
    } else {
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

CaseDec::CaseDec(Num* num) { this->children.push_back(num); }

CaseDec::CaseDec(Num* num, b* b) {
  this->children.push_back(num);
  this->children.push_back(b);
}
ExpList::ExpList(Exp* expression) {
  this->children.push_back(expression);
  this->types = vector<string>();
  this->types.push_back(expression->type);
}

ExpList::ExpList(Exp* expression, ExpList* expList) {
  this->children.push_back(expression);
  this->children.push_back(expList);
  this->types = vector<string>();
  this->types.push_back(expression->type);
  for(int i=0; i<expList->types.size(); i++){
    this->types.push_back(expList->types[i]);
  }
}

Call::Call(Id* id) {
  this->children.push_back(id);
  if (!isDeclared(scopes, id)) {
    errorUndef(yylineno, id->text);
    exit(0);
  }
  Symbol s = getSymbolById(scopes, id->text);
  if (!(s.isFunc)) {
    errorUndefFunc(yylineno, id->text);
    exit(0);
  } else if (!s.args.empty()) {
    errorPrototypeMismatch(yylineno, id->text, s.args);
    exit(0);
  }
  this->id = id->text;
}

Call::Call(Id* id, ExpList* expList) {
  this->children.push_back(id);
  this->children.push_back(expList);
  if (!isDeclared(scopes, id)) {
    errorUndefFunc(yylineno, id->text);
    exit(0);
  } else {
    Symbol s = getSymbolById(scopes, id->text);
    if (!(s.isFunc)) {
      errorUndefFunc(yylineno, id->text);
      exit(0);
    } else if (!validTypes(s.args,expList->types)) {

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
  }
  this->id = id->text;
}

Exp::Exp(String* expression) {
  /*
  if (allowStrings != true) {
    errorMismatch(yylineno);
    exit(0);
  }*/
  this->children.push_back(expression);
  this->type = expression->type;
  this->type = "STRING";
}

Exp::Exp(Num* expression) {
  this->children.push_back(expression);
  this->type = "INT";
}

Exp::Exp(Exp* expression) {
  this->children.push_back(expression);
  this->type = expression->type;
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
  } else {
    this->type = "BYTE";
  }
}
Exp::Exp(Id* id) {
  this->children.push_back(id);
  if (!isDeclared(scopes, id)) {
    errorUndef(yylineno, id->text);
    exit(0);
  }
  this->type = getTypeById(scopes, id);
}
Exp::Exp(Call* call) {
  this->children.push_back(call);
  Symbol s = getSymbolById(scopes, call->id);
  this->type = getSymbolById(scopes, call->id).ret;
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
Exp::Exp(False* f) {
  this->children.push_back(f);
  this->type = "BOOL";
}
Exp::Exp(True* t) {
  this->children.push_back(t);
  this->type = "BOOL";
}
Exp::Exp(Not* notOp, Exp* expression2) {
  this->children.push_back(notOp);
  this->children.push_back(expression2);
  if ((expression2->type != "BOOL")) {
    errorMismatch(yylineno);
    exit(0);
  }
  this->type = "BOOL";
}
Exp::Exp(Exp* expression1, And* andOp, Exp* expression2) {
  this->children.push_back(expression1);
  this->children.push_back(andOp);
  this->children.push_back(expression2);
  if ((expression1->type != "BOOL") || (expression2->type != "BOOL")) {
    errorMismatch(yylineno);
    exit(0);
  }
  this->type = "BOOL";
}
Exp::Exp(Exp* expression1, Or* orOp, Exp* expression2) {
  this->children.push_back(expression1);
  this->children.push_back(orOp);
  this->children.push_back(expression2);
  if ((expression1->type != "BOOL") || (expression2->type != "BOOL")) {
    errorMismatch(yylineno);
    exit(0);
  }
  this->type = "BOOL";
}
Exp::Exp(Exp* expression1, Relop* relop, Exp* expression2) {
  this->children.push_back(expression1);
  this->children.push_back(relop);
  this->children.push_back(expression2);
  if ((expression1->type != "INT" && expression1->type != "BYTE") ||
      (expression2->type != "INT" && expression2->type != "BYTE")) {
    errorMismatch(yylineno);
    exit(0);
  }
  this->type = "BOOL";
}