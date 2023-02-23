%{
#include<stdio.h>
%}

IDENT [_a-zA-Z][_a-zA-Z0-9]*
NUM (([1-9][0-9]*"."?[0-9]*)|(0"."?[0-9]*))(e-?(([1-9][0-9]*)|"0"))?
SYM "+"|"-"|"*"|"/"|"("|")"|"<"|">"|">="|"<="|"=="|"!="|"&&"|"&"|"||"|"|"|"~"|"!"|","|"."|";"

%%

{IDENT} {printf("ident ");}
{NUM} {printf("num ");}
{SYM} {printf("sym ");}
<<EOF>> {printf("EOF ");}
.

%%

int main(){
    yylex();
    yywrap();
}
int yywrap(){return 1;}
