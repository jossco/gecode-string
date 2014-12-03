'''
gecode-string:	bounded-length string constraints for Gecode

url: 		github.com/jossco/gecode-string
author: 	Joseph D. Scott, Uppsala University
========
file: 		./kaluzish/reparser.py
version: 	0.2.1
date: 		Wed Dec  3 17:58:46 CET 2014
========
'''

from FAdo.reex import *
from FAdo.fa import *
from pyparsing import *
import pprint
import logging

module_logger = logging.getLogger('reparser')
metas = r"\/(){}[].*?+|"
literals = "".join(c for c in printables if c not in metas) + " \t"
maxSymbol = 96

def mapLiteral(c):
    return ord(c) - 31
def generateLiteral(t):
    return regexp(mapLiteral(t))

def generateDisj(a,b):
    return disj(a,b)

def generateConcat(a,b):
    return concat(a,b)
    
def generateRange(r):
    return reduce( generateDisj, map(generateLiteral, list(srange(r).encode('utf-8'))) )
    
def generateInverseRange(r):
    excluded = map(mapLiteral, list(srange(r).encode('utf-8')))
    included = [i for i in range(maxSymbol) if i not in excluded]
    return reduce(generateDisj, map(regexp, included))
    # excluded = list(srange(r).encode('utf-8'))
#     included = [c for c in literals if c not in excluded]
#     return reduce( generateDisj, map(generateLiteral, included) )
    
def handleRange(toks):
    t = toks[0]
    if t[1] == "^":
        return generateInverseRange(t)
    return generateRange(t)
    
def handleRepetition(toks):
    t = toks[0]
    if t[1] == "*":
        return star(t[0])
    elif t[1] == "+":
        return concat(t[0], star(t[0]))
    elif t[1] == "?":
        return disj(epsilon(), t[0])
    elif "count" in t:
        return reduce(generateConcat, [t[0]] * int(t.count))
    elif "minCount" in t:
        mincount = int(t.minCount)
        required = reduce(generateConcat, [t[0]] * mincount)
        maxcount = int(t.maxCount)
        optcount = maxcount - mincount
        if optcount:
            opts = map(lambda c: disj(epsilon(),c), [t[0]] * optcount)
            opt = reduce(generateConcat, opts)
            return concat(required, opt)
        else:
            return required

def handleLiteral(toks):
    t = toks[0]
    if t[0] == "\\":
        if t[1] == "t":
            return generateLiteral("\t")
        else:
            return generateLiteral(t[1])
    else:
        return generateLiteral(toks[0]) 

def handleMacro(toks):
    macroChar = toks[0][1]
    if macroChar == "d":
        return generateRange("[0-9]")
    elif macroChar == "w":
        return generateRange("[A-Za-z0-9_]")
    elif macroChar == "s":
        return generateLiteral(' ')
    else:
        raise ParseFatalException("",0,"unsupported macro character (" + macroChar + ")")
    
def handleSequence(toks):
    return reduce(generateConcat, toks[0])

def handleDot(toks):
    return reduce( generateDisj, map(regexp,range(96)) )

def handleAlternative(toks):
    return reduce(generateDisj, toks[0])

def handleGroup(toks):
    print toks
    
def parser():
    module_logger.info("Generating parser...")
    ParserElement.setDefaultWhitespaceChars("")
    lbrack,rbrack,lbrace,rbrace,lparen,rparen = map(Literal,"[]{}()")
    
    reMacro = Combine("\\" + oneOf(list("dws")))
    escapedChar = ~reMacro + Combine("\\" + oneOf(list(metas)))#oneOf(list(printables)))
    reLiteralChar = literals
    
    reRange = Combine(lbrack + SkipTo(rbrack,ignore=escapedChar) + rbrack)
    reLiteral = ( escapedChar | oneOf(list(reLiteralChar)) )
    reDot = Literal(".")
    repetition = (
        ( lbrace + Word(nums).setResultsName("count") + rbrace ) |
        ( lbrace + Word(nums).setResultsName("minCount")+","+ Word(nums).setResultsName("maxCount") + rbrace ) |
        oneOf(list("*+?")) 
        )
        
    reRange.setParseAction(handleRange)
    reLiteral.setParseAction(handleLiteral)
    reMacro.setParseAction(handleMacro)
    reDot.setParseAction(handleDot)
    
    reExpr = Forward()
    reTerm = ( lparen.suppress() + OneOrMore(reExpr) + rparen.suppress() | reLiteral | reRange | reMacro | reDot )
    reExpr << operatorPrecedence( reTerm,
        [
        (repetition, 1, opAssoc.LEFT, handleRepetition),
        (None, 2, opAssoc.LEFT, handleSequence),
        (Literal('|').suppress(), 2, opAssoc.LEFT, handleAlternative),
        ]
        )
    _parser = reExpr
    return _parser

def nfa2dfa(re, negated=False):
    module_logger.info("Determinizing, minimizing, renaming states...")
    dfa = re.nfaPosition().toDFA()
    if negated:
        dfa = ~dfa
    dfa = dfa.minimal()
    dfa.renameStates(range(len(dfa)))
    return dfa
# def dfa2mzn(dfa):
#     module_logger.info("Converting FAdo representation to minizinc...")
#     S = 96
#     Q = len(dfa.States)
#     d = [ [1]*S for s in range(Q)]
#     for t in dfa.succintTransitions():
#         for symbol in t[1].split(','):
#             d[int(t[0])][int(symbol)-1] = int(t[2])+1
#     q0 = dfa.Initial + 1
#     F = map(lambda f: f+1, dfa.Final)
#     return [Q, S, d, q0, F]
def re2dfa(retext, negated=False):
    p = parser()
    module_logger.info("Parsing /%s/...", retext)
    res = p.parseString(retext)
    module_logger.debug("regexp = %s", res[0])
    return nfa2dfa(res[0], negated)
if __name__ == "__main__":
    unittest.main()
