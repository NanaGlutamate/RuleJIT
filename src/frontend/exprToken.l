%{

// CAUTION: input string should end line with '\n', not '\r\n'

// #define YY_NO_UNISTD_H
#include<stdio.h>

%}

%option c++
%option yyclass="GeneratedLexer"
%option batch

INT [1-9][0-9]*|0
REAL (([1-9][0-9]*"."[0-9]*)|(0"."[0-9]*)|("."[0-9]+))(e-?(([1-9][0-9]*)|"0"))?
STRING \"([^"]|\\\")*\"

IDENT ("[]")*[_a-zA-Z\x80-\xff][_a-zA-Z0-9\x80-\xff]*
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
