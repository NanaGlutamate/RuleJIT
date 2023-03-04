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
SYM [^0-9a-zA-Z_]|&&|"||"|>=|<=|==|->|!=|>>|<<|and|or|not|xor
ENDLINE ;|\n
SPACE " "|\t|\r
END \0
KEYWORD if|else|while|func|var|type|struct|class|dynamic|interface|extern|return

COMMENT "//"[^\n]*

%%

<<EOF>>|{END}     {printf("EOF ");}
{SPACE}           {}
{REAL}            {printf("real ");}
{INT}             {printf("int ");}
{STRING}          {printf("string ");}

{KEYWORD}         {printf("keyword ");}
{SYM}             {printf("sym ");}
{IDENT}           {printf("ident ");}
{ENDLINE}         {printf("end-of-line ");}
{COMMENT}         {}

%%

int main(){
    yylex();
    yywrap();
}
int yywrap(){return 1;}
