%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "parser.hpp"
	#include "parser.tab.hpp"
%}

%option yylineno
%option noyywrap

realop ("=="|"!="|"<"|">"|"<="|">=")
binop ("+"|"-"|"*"|"/")
id ([a-zA-Z][a-zA-Z0-9]*)
num (0|[1-9][0-9]*)
comment ("//"[^\r\n]*[\r|\n|\r\n]?)
whitespace ([\t\n ])
newline 	(\r\n|\r|\n)

%%

"void" {yyval=new Void(); return VOID; }
"int" {yyval=new Int(); return INT;}
"byte" {yylval= new Byte(); return BYTE;}
"b" {yylval= new b(); return B;}
"bool" {yylval= new Bool(); return BOOL;}
"and" {yylval= new And(); return AND;}
"or" {yylval= new Or(); return OR;}
"not" {yylval= new Not(); return NOT;}
"true" {yylval= new True(); return TRUE;}
"false" {yylval= new False();return FALSE;}
"return" {//yylval= new Return();
	return RETURN;}
"if" {//yylval= new If();
    return IF;}
"else" {//yylval= new Else();
    return ELSE;}
"while" {//yylval= new While();
    return WHILE;}
"break" {//yylval= new Break();
    return BREAK;}
";" {//yylval= new SC;
	return SC; }
"," {//yylval= new Comma();
    return COMMA;}
"(" {//yylval= new Lparen();
    return LPAREN;}
")" {//yylval= new Rparen();
    return RPAREN;}
"{" {//yylval= new Lbrace();
    return LBRACE;}
"}" {//yylval= new Rbrace();
    return RBRACE;}
"[" {//yylval= new Lbrack();
	return LBRACK;}
"]"	{//yylval= new Rbrack();
	return RBRACK;}
"=" {//yylval= new Assign();
    return ASSIGN;}
{realop} {yylval= new Relop();
                return RELOP;}
{binop} {yylval= new Binop();
                return BINOP;}
{id} return {yylval= new Id(yytext);
                        return ID;}
{num} return {yylval= new Num(yytext);
                return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\" {yylval= new String(yytext);
                                return STRING;}				
{comment} ;		//ignore comments
{newline}|{whitespace}	;
.	{errorLex(yylineno);exit(0);};
%%

