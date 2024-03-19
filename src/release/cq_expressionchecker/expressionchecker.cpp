#include <iostream>

#include "expressionchecker.h"

#define CHECK(s) {auto [msg, is_error] = checker.checkExpression(s); std::cout << (is_error ? msg : "no_error") << std::endl;}

int main(){
    using namespace std;

    ExpressionChecker checker;

    checker.checkXML("D:\\Desktop\\rule.xml");

    // checker.addTypeDef("Vector3", {{"x", "float64"}, {"y", "float64"}, {"z", "float64"}});
    // checker.addVarDef("add", "float64");
    // checker.addVarDef("v", "Vector3");

    // CHECK("a + b");
    // CHECK("add + v");
    // CHECK("{var a = 1; a + add}");

    // checker.removeTypeDef("Vector3");

    // CHECK("add");
    // CHECK("v");

    // checker.removeVarDef("add");

    // CHECK("add");

    return 0;
}