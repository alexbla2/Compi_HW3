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

"void"  { 	return VOID; }
"int"   { 	return INT;}
"byte"  { 	return BYTE;}
"b"     { 	return B;}
"bool"  { 	return BOOL;}
"and"   {	return AND;}
"or"    { 	return OR;}
"not"   { 	return NOT;}
"true"  { 	return TRUE;}
"false" {	return FALSE;}
"return" {	return RETURN; }
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
{realop} { 	 return RELOP;}
{binop}  {   return BINOP;}
{id} 	 {yylval= new Id(yytext); 		return ID;}
{num}    {yylval= new Num(yytext); 		return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\" {yylval= new String(yytext); return STRING;}				
{comment} 				;//ignore comments
{newline}|{whitespace}	;
.	{output::errorLex(yylineno);exit(0);};
%%

