
%option yylineno
%option noyywrap

realop ("=="|"!="|"<"|">"|"<="|">=")
binop ("+"|"-"|"*"|"/")
id ([a-zA-Z][a-zA-Z0-9]*)
num (0|[1-9][0-9]*)
comment (//[^\r\n]*[\r|\n|\r\n]?

whitespace ([\t\n ])
newline 	(\r\n|\r|\n)

%x string

%%

"void" return VOID;
"int" return INT;
"byte" return BYTE;
"b" return B;
"bool" return BOOL;
"and" return AND;
"or" return OR;
"not" return NOT;
"true" return TRUE;
"false" return FALSE;
"return" return RETURN;
"if" return IF;
"else" return ELSE;
"while" return WHILE;
"break" return BREAK;
";" return SC;
"," return COMMA;
"(" return LPAREN;
")" return RPAREN;
"{" return LBRACE;
"}" return RBRACE;
"[" return LBRACK;
"]"	return RBRACK;
"=" return ASSIGN;
{realop} return REALOP;
{binop} return BINOP;
{id} return ID;
{num} return NUM;
{comment} ;		//ignore comments



\" BEGIN(string);yymore(); //need to fix here!
<string>\\ BEGIN(escape);yymore();
<escape>\\|\"|a|b|e|f|n|r|t|v|0|x{hexdig}{hexdig} BEGIN(string);yymore();
<escape>. printf("Error undefined escape sequence %c\n",yytext[yyleng-1]);exit(0);
<string>\" BEGIN(0);return STRING;//showToken("STRING")
<string>{newline} yymore();
<string>{whitespace} yymore();
<string>[[:print:]] yymore();
<string><<EOF>> printf("Error unclosed string\n");exit(0);
<string>. printf("Error %c\n",yytext[yyleng-1]); exit(0);

<<EOF>> return EF;//showToken("EOF"); exit(0);
{newline}|{whitespace}	;
.	printf("Error %s\n",yytext); exit(0);
%%

