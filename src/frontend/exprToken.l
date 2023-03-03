%{
#define YY_NO_UNISTD_H
#include<stdio.h>
%}

IDENT [_a-zA-Z][_a-zA-Z0-9]*
NUM (([1-9][0-9]*"."?[0-9]*)|(0"."?[0-9]*))(e-?(([1-9][0-9]*)|"0"))?
STRING \"([^"]|\\\")*\"
SYM .|&&|"||"|>=|<=|==|->|!=|>>|<<
SPACE " "|\r|\n|\t
KEYWORD and|or|not|if|else|while|for|do|func|var

%%

{STRING}    {printf("string ");}
{NUM}       {printf("num ");}
{KEYWORD}   {printf("keyword ");}
{IDENT}     {printf("ident ");}
{SPACE}     {}
{SYM}       {printf("sym ");}

%%

int main(){
    yylex();
    yywrap();
}
int yywrap(){return 1;}
