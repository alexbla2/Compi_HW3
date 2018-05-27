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

"void"  {yylval=new Void(); 	return VOID; }
"int"   {yylval=new Int(); 		return INT;}
"byte"  {yylval= new Byte(); 	return BYTE;}
"b"     {yylval= new B(); 		return B;}
"bool"  {yylval= new Bool(); 	return BOOL;}
"and"   {yylval= new And();		return AND;}
"or"    {yylval= new Or(); 		return OR;}
"not"   {yylval= new Not(); 	return NOT;}
"true"  {yylval= new True(); 	return TRUE;}
"false" {yylval= new False();	return FALSE;}
"return" {	 return RETURN;}
"if"    {    return IF;}
"else"  { 	 return ELSE;}
"while" { 	 return WHILE;}
"break" { 	 return BREAK;}
";" 	{    return SC; }
"," 	{    return COMMA;}
"(" 	{    return LPAREN;}
")" 	{    return RPAREN;}
"{" 	{    return LBRACE;}
"}" 	{    return RBRACE;}
"[" 	{	 return LBRACK;}
"]"		{	 return RBRACK;}
"=" 	{    return ASSIGN;}
{realop} {yylval= new Relop(); 			return RELOP;}
{binop}  {yylval= new Binop();  		return BINOP;}
{id} 	 {yylval= new Id(yytext); 		return ID;}
{num}    {yylval= new Num(yytext); 		return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\" {yylval= new String(yytext); return STRING;}				
{comment} 				;//ignore comments
{newline}|{whitespace}	;
.	{errorLex(yylineno);exit(0);};
%%

