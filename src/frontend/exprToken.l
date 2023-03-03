%{
#include<stdio.h>
%}

IDENT [_a-zA-Z][_a-zA-Z0-9]*
NUM (([1-9][0-9]*"."?[0-9]*)|(0"."?[0-9]*))(e-?(([1-9][0-9]*)|"0"))?
SYM "+"|"-"|"*"|"/"|"("|")"|"<"|">"|">="|"<="|"=="|"!="|"&&"|"&"|"||"|"|"|"~"|"!"|","|"."|";"|"<<"|">>"|"""|"["|"]"
SPACE " "|\r|\n|\t
KEYWORD and|or|not

%%

{IDENT} {printf("ident ");}
{NUM} {printf("num ");}
{SYM} {printf("sym ");}
{SPACE}
. {printf("unknown: %c", *yytext);}

%%

int main(){
    yylex();
    yywrap();
}
int yywrap(){return 1;}
