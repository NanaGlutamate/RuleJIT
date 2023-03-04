%{

/****************************************************************
%option c++
%option yyclass="GeneratedLexer"
%option batch
*/
// CAUTION: input string should end line with '\n', not '\r\n'; no unicode support

#define YY_NO_UNISTD_H
#include<stdio.h>

%}


INT [1-9][0-9]*|0
REAL (([1-9][0-9]*"."[0-9]*)|(0"."[0-9]*)|("."[0-9]+))(e-?(([1-9][0-9]*)|"0"))?
STRING \"([^"]|\\\")*\"

IDENT [_a-zA-Z][_a-zA-Z0-9]*
SYM .|&&|"||"|>=|<=|==|->|!=|>>|<<
ENDLINE ;|\n
SPACE " "|\t|\r
KEYWORD and|or|not|if|else|while|func|var|type

%%

<<EOF>>     {printf("EOF ");}
{STRING}    {printf("string ");}
{REAL}      {printf("real ");}
{INT}       {printf("int ");}

{KEYWORD}   {printf("keyword ");}
{IDENT}     {printf("ident ");}
{ENDLINE}   {printf("end-of-line ");}
{SPACE}     {}
{SYM}       {printf("sym ");}

%%

int main(){
    yylex();
    yywrap();
}
int yywrap(){return 1;}
