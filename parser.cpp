#include "parser.hpp"


void initializeStacks(stack<SymbolTable>& StackTable, stack<int>& OffsetStack) {
  allowStrings = false;  //??????????
  SymbolTable globalScope;
  StackTable.push(globalScope);
  offsetStack.push(0);
  std::vector<string> argTypesForPrint;

  argTypesForPrint.push_back("STRING");
  Symbol s("print", makeFunctionType("VOID", argTypesForPrint), offsetStack.top(),
           argTypesForPrint, "VOID");
  StackTable.top().push_back(s);

  std::vector<string> argTypesForPrinti;

  argTypesForPrinti.push_back("INT");
  s = Symbol("printi", makeFunctionType("VOID", argTypesForPrinti),
             offsetStack.top(), argTypesForPrinti, "VOID");
  StackTable.top().push_back(s);
}