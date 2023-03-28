/**
 * @file lexermain.cpp
 * @author djw
 * @brief 
 * @date 2023-03-28
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#include <iostream>
#include <string>
#include "frontend/lexer.h"

int main(){
    using namespace std;
    string input;
    rulejit::ExpressionLexer lexer;
    while(std::getline(cin, input)){
        input.push_back('\n');
        lexer.load(input);
        while(lexer.tokenType()!=rulejit::TokenType::ENDLINE){
            cout << lexer.tokenType() << '\t';
            if(lexer.tokenType()==rulejit::TokenType::STRING){
                string tmp;
                auto lex = lexer.top();
                lexer >> tmp;
                cout << lex << '\t' << tmp;
            }else if(lexer.tokenType()==rulejit::TokenType::INT){
                long long int tmp;
                auto lex = lexer.top();
                lexer >> tmp;
                cout << lex << '\t' << tmp;
            }else if(lexer.tokenType()==rulejit::TokenType::REAL){
                double tmp;
                auto lex = lexer.top();
                lexer >> tmp;
                cout << lex << '\t' << tmp;
            }else{
                cout << lexer.pop() << '\t';
            }
            cout << endl;
            if(lexer.errorHandler.err){
                cout << "Error: " << lexer.errorHandler.info << endl;
                break;
            }
        }
    }
}