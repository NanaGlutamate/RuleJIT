import re
import math
import operator
from typing import List, Dict, Tuple

NumToken = re.compile('^[0-9.]+$')
VarToken = re.compile('^[a-zA-Z][a-zA-Z0-9]*$')

ops = {"<" : 10, ">" : 10, "+" : 20, "-" : 20, "*" : 30, "/" : 30}

def isOp(s):
    return s in ops

def isNum(s):
    return NumToken.match(s)

def isIdent(s):
    return VarToken.match(s)

def isExpr(s):
    return isNum(s) or isIdent(s)

def isSym(s):
    return s in {'(', ')', ','}

def isToken(s):
    return isOp(s) or isNum(s) or isIdent(s) or isSym(s)

def splitFirstToken(s):
    tmp = ''
    for ind, i in enumerate(s):
        if str.isspace(i):
            if tmp != '':
                return tmp, s[ind+1:]
        new = tmp + i
        if isToken(tmp) and not isToken(new):
            return tmp, s[ind:]
        tmp = new
    return None, ''

def getTokens(s):
    tmp = ''
    for i in s:
        if str.isspace(i) and tmp != '':
            yield tmp
            tmp = ''
        else:
            new = tmp + i
            if isToken(tmp) and not isToken(new):
                yield tmp
                tmp = i
            else:
                tmp = new
    yield tmp

def processParent(stack):
    stack.pop()
    stack, tree = processExpr(stack)
    if not stack or stack.pop() != ')':
        raise Exception(f'except ")"')
    return stack, tree

def processBinRHS(prec, lhs, stack):
    while True:
        op = stack[-1]
        if not isOp(op):
            return stack, lhs
        if ops[op] < prec:
            return stack, lhs
        stack.pop()
        stack, rhs = processPrim(stack)
        if not stack or not isOp(stack[-1]):
            return stack, (op, lhs, rhs)
        if ops[stack[-1]] > ops[op]:
            stack, rhs = processBinRHS(ops[op] + 1, rhs, stack)
        lhs = (op, lhs, rhs)

def processExpr(stack):
    stack, lhs = processPrim(stack)
    if not stack or not isOp(stack[-1]):
        return stack, lhs
    return processBinRHS(0, lhs, stack)

def processPrim(stack):
    if not stack:
        raise Exception('unexcept eof')
    if stack[-1] == '(':
        return processParent(stack)
    elif isNum(stack[-1]):
        return stack, eval(stack.pop())
    elif isIdent(stack[-1]):
        name = stack.pop()
        if not stack or stack[-1] != '(':
            return stack, name
        stack.pop()
        var = []
        while stack:
            if stack[-1] == ')':
                stack.pop()
                break
            stack, tree = processExpr(stack)
            var.append(tree)
            if not stack:
                raise Exception('unexcept eof')
            exp_end = stack[-1]
            if exp_end != ',' and exp_end != ')':
                raise Exception(f'expect "," or ")", found "{exp_end}"')
            if exp_end == ',':
                stack.pop()
        return stack, (name, *var)

def processStr(s):
    tokens = [i for i in getTokens(s)][::-1]
    return processExpr(tokens)[1]

fun = {
    '+': operator.add,
    '-': operator.sub,
    '*': operator.mul,
    '/': operator.truediv,
    '>': operator.gt,
    '<': operator.lt,
    'sin': math.sin,
    'cos': math.cos,
    'tan': math.tan,
    'asin': math.asin,
    'acos': math.acos,
    'atan': math.atan,
}

sin = math.sin
cos = math.cos
tan = math.tan
asin = math.asin
acos = math.acos
atan = math.atan

def evalTree(tree):
    try:
        tree[0]
    except:
        return tree
    if tree[0] in fun:
        func = fun[tree[0]]
        return func(*[evalTree(i) for i in tree[1:]])
    else:
        return tree[-1]

def test(s: str):
    if not evalTree(processStr(s)) == eval(s):
        print(f'test not pass: \n\t{s}: {evalTree(processStr(s))} != {eval(s)}')
    else:
        print(f'test passed: {s}')

if __name__ == '__main__':
    test('1+2+3*4*5+6')
    test('1+(2+3)*4+5')
    test('1+(2+(3)*4)+5')
    test('1+sin(2+(3)*4)+5')