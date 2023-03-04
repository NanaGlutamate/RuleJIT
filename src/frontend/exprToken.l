%{

/****************************************************************
%option c++
%option yyclass="GeneratedLexer"
%option batch
*/

// CAUTION: input string should end line with '\n', not '\r\n'; no unicode support

enum YY_TOKEN{

};

#define YY_INPUT(buf,result,max_size) 
#define YY_DECL int yylex(int guidence, char* start_ptr)
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
END \0
KEYWORD if|else|while|func|var|type|struct|class|dynamic|interface|extern
LOGIC and|or|not|xor

COMMENT "//"[^\n]*

%%

<<EOF>>|{END}     {printf("EOF ");}
{COMMENT}         {}
{STRING}          {printf("string ");}
{REAL}            {printf("real ");}
{INT}             {printf("int ");}

{LOGIC}           {printf("logic ");}
{KEYWORD}         {printf("keyword ");}
{IDENT}           {printf("ident ");}
{ENDLINE}         {printf("end-of-line ");}
{SPACE}           {}
{SYM}             {printf("sym ");}

%%

int main(){
    yylex();
    yywrap();
}
int yywrap(){return 1;}
