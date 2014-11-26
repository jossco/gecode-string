#!/usr/bin/env python2.7

from pyparsing import *
import pprint
from reparser import re2dfa
import logging
import argparse
from os import path
from FAdo.fa import *


logger = logging.getLogger(__name__)
text =r"""
STR_35 := "string" . STR_5;
type STR_34 : CSTR;
ASSERT(BOOL_8);
BOOL_1 := !BOOL_3;
LHS_2 := !true;
INT_1 := 0x003 + INT_2; BOOL_1 := !BOOL_2;
INT_10 := INT_1 + 0xfff;

15 == INT_21;
INT_22 <= 16;
INT_5 < INT_6;
BOOL_1 := TEMP_4 != TEMP_3;
STR_2 \notin CapturedBrack (/complicated\\regex/, 0);
15 < Len(STR_1);
LEN_1 == Len(STR_1);
LEN_2 == Len("constant");
14 < Len("constant");"""

class CstrDesc:
    def __init__(self, cstr, parameter, model):
        self.cstr = cstr
        self.parameter = parameter
        self.model = model
    def header(self):
        return getattr(self, self.cstr + "Header")(*self.parameter)
    def __eq__(self, other):
        return self.cstr == other.cstr and self.parameter == other.parameter
    def __str__(self):
        return getattr(self, self.cstr)(*self.parameter)
    def predicate(self, boolvar):
        return "constraint {} == true;".format(boolvar)
    def rel(self, lhs, rel, rhs, reif=False):
        if lhs in self.model.vType and self.model.vType[lhs] == "str":
            if rel == "==":
                if reif and self.model.bounded:
                    return "constraint str_equal_reif({x}, len_{x}, {y}, len_{y});".format(x=lhs, y=rhs)
                else:
                    return "constraint str_equal({x}, len_{x}, {y}, len_{y});".format(x=lhs, y=rhs)
            elif rel == "!=":
                return "constraint str_nequal({x}, len_{x}, {y}, len_{y});".format(x=lhs, y=rhs)
            else:
                raise TypeError("String rel constraint only supports (dis)equality, not '{}'".format(rel))
        return "constraint {} {} {};".format(lhs, rel, rhs)
    def boolops(self, lhs, rhs):
        return "constraint {} == not {};".format(lhs, rhs)
    def concat(self, lhs, t1, t2):
        return "constraint str_concat({x}, len_{x}, {y},  len_{y}, {z}, len_{z} );".format(x=t1,y=t2,z=lhs)
    def length(self, lhs, rel, rhs):
        return "constraint {} {} len_{};".format(lhs, rel, rhs)
    def intops(self, lhs, t1, op, t2):
        return "constraint {} = ({} {} {});".format(lhs, t1, op, t2)
    def compops(self, lhs, t1, comp, t2):
        if self.model.isStrType(t1):
            if comp == "==":
                if self.model.bounded:
                    return "constraint {lhs} <-> str_equal_reif({x}, len_{x}, {y}, len_{y});".format(lhs=lhs, x=t1, y=t2)
                else:
                    return "constraint {lhs} <-> str_equal({x}, len_{x}, {y}, len_{y});".format(lhs=lhs, x=t1, y=t2)
            elif comp == "!=":
                return "constraint {lhs} <-> str_nequal({x}, len_{x}, {y}, len_{y});".format(lhs=lhs, x=t1, y=t2)
            else:
                raise TypeError("string comp defined for (dis)equality only: {} <-> {} {} {}".format(lhs, t1, comp, t2))
        else:
            return "constraint {} <-> {} {} {};".format(lhs, t1, comp, t2)
    def regular(self, lhs, memop, language):
        return "constraint str_regular({var_name}, len_{var_name}, {dfa});".format(var_name=lhs, dfa=self.model.dfa[(memop, language)].parameterString())
    def ite(self, condition, tblock, eblock):
        t="\n  /\ ".join(map(lambda s: str(s)[11:].rstrip(';'),tblock))
        e="\n  /\ ".join(map(lambda s: str(s)[11:].rstrip(';'),eblock))
        if self.model.version > 1:
            return "constraint if {condition} \n  then (\n  {tblock}\n)\n  else (\n  {eblock}\n) endif;".format(condition=condition, tblock=t, eblock=e)
        else:
            return "constraint {condition} <-> (\n  {tblock}\n);\n  constraint not {condition} <-> (\n  {eblock}\n);".format(condition=condition, tblock=t, eblock=e)
class DFA:
    counter = 0
    def __init__(self, memop, language):
        dfa = None
        self.logging = logging.getLogger('{}.DFA'.format(__name__))
        self.logging.info("Creating new dfa for %s /%s/", memop, language)
        if "notin" in memop:
            lang = ".*" + str(language) + ".*"
            self.logging.info("parsing RE: ~/%s/", lang)
            dfa = re2dfa(lang, negated=True)
        elif "nin" in memop:
            self.logging.info("parsing RE: ~/%s/", language)
            dfa = re2dfa(language, negated=True)
        else:
            self.logging.info("parsing RE: /%s/", language)
            dfa = re2dfa(language)
        self.logging.debug("DFA: %s", str(dfa))
        self.logging.info("Converting FAdo representation to minizinc...")
        self.alpha = 96
        self.states = len(dfa.States)
        self.transition = [ [0]*self.alpha for s in range(self.states)]
        for t in dfa.succintTransitions():
            for symbol in t[1].split(','):
                self.transition[int(t[0])][int(symbol)-1] = int(t[2])+1
        self.initial = dfa.Initial + 1
        self.final = map(lambda f: f+1, dfa.Final)
        DFA.counter += 1
        self.name = "MZN_DFA_{}".format(DFA.counter)
        self.logging.info("%s created", self.name)
    def parameterString(self):
        return "{0}_Q, max_symbol, {0}_d, {0}_q0, {0}_F".format(self.name)
    def __str__(self):
        decl = """int: {name}_Q = {states};
int: {name}_q0 = {initial}; set of int: {name}_F = {{ {final} }};
array[1..{name}_Q,1..max_symbol] of int: {name}_d =""".format(name=self.name, initial=self.initial,states=self.states,final=", ".join(map(str,self.final)))
        table = [" [|"]
        for i, row in enumerate(self.transition):
            table.append("\n  % State {}\n".format(i + 1))
            fullgroups = len(row)/10
            for g in range(fullgroups):
                table.append("    " + ", ".join(map(str,row[g*10:(g+1)*10])) + ",\n")
            table.append("    " + ", ".join(map(str,row[fullgroups*10:])) + "|") #+ "\t%State {}".format(i)
        table.append("];\n")
        return decl + ''.join(table)
class DFAlist(dict):
    def __init__(self):
        self.logger = logging.getLogger("{}.DFAlist".format(__name__))
    def __getitem__(self,key):
        if not self.has_key(key):
            self[key] = DFA(*key)
        return super(DFAlist, self).__getitem__(key)
class MznModel:
    declarations = {
        'bool'  : "var bool: {};",
        'int'   : "var int: {};",
        'str': "var 0..wordlen: len_{0}; array[widx] of var symbols: {0};\nconstraint str_pad({0}, len_{0});",
        None: "assert(false) % untyped variable {};"
    }
    def __init__(self, version, bounded):
        self.version = version
        self.bounded = bounded
        self.logging = logging.getLogger('{}.MznModel'.format(__name__))
        self.header =list("""\
include "gecode-string-{}.mzn";
include "gecode-string-ascii.mzn";
        
int: wordlen;
set of int: widx = 1..wordlen;""".format("open" if bounded else "pad").splitlines())
        self.footer = """

solve satisfy;

output ["{0}=\\""] ++ [ascii[fix({0}[i])] | i in 1..fix(len_{0})] ++ ["\\"\\n"];
        """
        self.reif = ['boolops', 'intops', 'rel', 'predicate', 'length']
        self.constraint = []
        self.vType = {}
        self.constStr = {}
        self.ecList = []
        self.ecMap = {}
        self.tempVarCtr = 1
        self.constStrCtr = 1
        self.dfa = DFAlist()
        self.vOut = ""
    def newTempVar( self ):
        temp = "MZN_GEN_{}".format(self.tempVarCtr)
        self.tempVarCtr += 1
        return temp
    def isStrType(self, v):
        if v in self.vType:
            return self.vType[v] == "str"
        else:
            return v in self.constStr
    def constantType(self, name):
        if type(name) is int:
            return "int"
        elif name in self.constStr:
            return "str"
        elif name == "true" or name == "false":
            return "bool"
        else:
            raise TypeError("TYPE({}) is unknown!".format(name))
    def setTypeOfEquivalents( self, var ):
        if var in self.ecMap:
            varSet = self.ecMap[var]
            for v in varSet:
                del self.ecMap[v]
                self.setType(v,self.vType[var])
            self.ecList.remove(varSet)
    def addVariable(self, toks):
        name = toks[0]
        if name not in self.vType:
            self.vType[name] = None
            if "INPUT" in name:
                self.vOut = name
                self.logging.debug("output variable located: %s", name)
            self.logging.info("VAR: %s", name)
    def setType( self, var, vType):
        if var in self.vType:
            if vType == None:
                return
            if self.vType[var] == None:
                self.vType[var] = vType
                self.setTypeOfEquivalents(var)
            elif self.vType[var] == vType:
                return
            else:
                raise TypeError("TYPE({}) redeclared from <{}> to <{}>".format(var, self.vType[id], vType))
        else:
            cType = self.constantType(var)
            if cType != vType:
                raise TypeError("Found <{}>:'{}'  where <{}> expected".format(cType, var, vType))
        self.logging.info("TYPE(%s) = <%s>",var, vType)
    def setTypesEqual(self, term1, term2):
        self.setTypeEqualTo(term1, term2)
        self.setTypeEqualTo(term2, term1)
    def setTypeEqualTo( self, term1, term2, force=False ):
        vType = None
        if force:
            self.vType[term1] = None
        if term1 in self.vType:
            if term2 in self.vType:
                if self.vType[term1] == None and self.vType[term2] == None:
                    self.addEquivalency(term1, term2)
                    return
                else:
                    vType = self.vType[term2]
            elif type(term2) is int or type(term2) is long:
                vType = "int"
            elif term2 in self.constStr:
                vType = "str"
            elif term2 == "true" or term2 == "false":
                vType = "bool"
            else:
                raise TypeError("Failed to type {} from {}".format(term1, term2))
            self.setType(term1, vType)
            self.logging.info("TYPE(%s) := TYPE(%s) == <%s>", term1, term2, vType)
    def addEquivalency(self, var1, var2):
        if var1 in self.ecMap:
            if var2 in self.ecMap:
                if self.ecMap[var1] != self.ecMap[var2]:
                    var2ec = self.ecMap[var2].copy()
                    self.ecMap[var1] |= var2ec
                    for v in var2ec:
                        self.ecMap[v] = self.ecMap[var1]
                        self.ecList.remove(var2ec)
            else:
                self.ecMap[var2] = self.ecMap[var1]
                self.ecMap[var1].add(var2)
        elif var2 in self.ecMap:
            self.ecMap[var1] = self.ecMap[var2]
            self.ecMap[var2].add(var1)
        else:
            ec = set([var1, var2])
            self.ecList.append(ec)
            self.ecMap[var1] = ec
            self.ecMap[var2] = ec
        self.logging.info("TYPE-CLASS: {%s}", ", ".join(self.ecMap[var1]))
    def addConstString( self, toks ):
        rawstr = toks[0].lstrip('"').rstrip('"')
        for (var, string) in self.constStr.items():
            if string == rawstr:
                return var
        var = "MZN_CONST_STR_{}".format(self.constStrCtr)
        self.constStrCtr += 1
        self.constStr[var] = rawstr
        self.logging.info("CONST_STR[%s] := '%s'", var, rawstr)
        return var
    def predicateConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr predicate: %s", s)
        self.setType(s.boolvar, "bool")
        self.constraint.append(CstrDesc("predicate", [s.boolvar], self))
    def regularConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr regular: %s", s)
        if s.position > 0:
            raise ValueError("regular: captured brackets not supported")
        self.setType(s.lhs, "str")
        dfa = self.dfa[(s.memop, s.language)]
        self.logging.info("dfa: %s", dfa.name)
        self.constraint.append(CstrDesc("regular", [s.lhs, s.memop, s.language], self))
    def relConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr rel: %s", s)
        if ('<' in s.relation) or ('>' in s.relation):
            self.setType(s.lhs, "int")
            self.setType(s.rhs, "int")
        else:
            self.setTypesEqual(s.rhs, s.lhs)
        self.constraint.append(CstrDesc("rel", [s.lhs, s.relation, s.rhs], self))
    def concatConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr concat: %s", s)
        self.setType(s.lhs, "str")
        self.setType(s.rhs1, "str")
        self.setType(s.rhs2, "str")
        if not(self.isStrType(s.rhs1)):
            raise TypeError("'{}' not a string in: {} = {} . {}".format(s.rhs1, s.lhs, s.rhs1, s.rhs2))
        if not(self.isStrType(s.rhs2)):
            raise TypeError("'{}' not a string in: {} = {} . {}".format(s.rhs2, s.lhs, s.rhs1, s.rhs2))
        self.constraint.append(CstrDesc("concat", [s.lhs, s.rhs1, s.rhs2], self))
    def lengthConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr length: %s", s)
        self.setType(s.lhs, "int")
        self.setType(s.rhs, "str")
        self.constraint.append(CstrDesc("length", [s.lhs, s.relation, s.rhs], self))
    def compopsConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr compops: %s", s)
        self.setType(s.lhs, "bool")
        if "<" in s.comp or ">" in s.comp:
            self.setType(s.term1, "int")
            self.setType(s.term2, "int")
        else:
            self.setTypesEqual(s.term1, s.term2)
        self.constraint.append(CstrDesc("compops", [s.lhs, s.term1, s.comp, s.term2], self))
    def intopsConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr intops: %s", s)
        self.setType(s.lhs, "int")
        self.setType(s.term1, "int")
        self.setType(s.term2, "int")
        self.constraint.append(CstrDesc("intops", [s.lhs, s.term1, s.op, s.term2], self))
    def boolopsConstraint( self, toks ):
        s = toks[0]
        self.logging.info("cstr boolops: %s", s)
        self.setType(s.lhs, "bool")
        self.setType(s.rhs, "bool")
        self.constraint.append(CstrDesc("boolops", [s.lhs, s.rhs], self))
    def ifelseStmt( self, toks ):
        ite = toks[0]
        self.logging.info("cstr ITE: %s", ite)
        self.setType(ite.condition, "bool")
        estart = -1 * len(ite.else_.block)
        tstart = -1 * (len(ite.else_.block) + len(ite.then_.block))
        eblock = self.constraint[estart:]
        tblock = self.constraint[tstart:estart]
        outside = []
        temp = {}
        newblock = []
        self.logging.info("===eblock start (%d) ===", estart)
        for stmt in eblock:
            if stmt in tblock:
                # constraint in both then and else, so promote to top level
                outside.append(stmt)
                tblock.remove(stmt)
            elif stmt.cstr in self.reif:
                if stmt.cstr == "rel" and self.bounded:
                    stmt.parameter.append(True)
                # constraint is fine to occur in reified context
                newblock.append(stmt)
            else:
                # constraint can't be reified, so introduce temps and reified channeling
                newParameter = []
                for term in stmt.parameter:
                    if term in self.vType:
                        if not term in temp:
                            temp[term] = self.newTempVar()
                            self.setTypeEqualTo(temp[term], term, force=True)
                            newblock.append(CstrDesc("rel", [temp[term], "==", term, True], self))
                            self.logging.info("cstr rel: ['%s', '==', '%s']", temp[term], term)
                        newParameter.append(temp[term])
                    else:
                        newParameter.append(term)
                outside.append(CstrDesc(stmt.cstr, newParameter, self))
        eblock = newblock[:]
        newblock = []
        temp = {}
        self.logging.info("===tblock start (%d) ===", tstart)
        for stmt in tblock:
            if stmt.cstr in self.reif:
                if stmt.cstr == "rel" and self.bounded:
                    stmt.parameter.append(True)
                # constraint is fine to occur in reified context
                newblock.append(stmt)
            else:
                newParameter = []
                for term in stmt.parameter:
                    if term in self.vType:
                        if not term in temp:
                            temp[term] = self.newTempVar()
                            self.setTypeEqualTo(temp[term], term, force=True)
                            newblock.append(CstrDesc("rel", [temp[term], "==", term, True], self))
                            self.logging.info("cstr rel: ['%s', '==', '%s']", temp[term], term)
                        newParameter.append(temp[term])
                    else:
                        newParameter.append(term)
                outside.append(CstrDesc(stmt.cstr, newParameter, self))
        self.logging.info("=== tempvar constraints start===")
        self.constraint = self.constraint[:tstart] + outside
        for stmt in outside:
            self.logging.info("cstr %s: %s", stmt.cstr, str(stmt.parameter))
        self.logging.info("=== tempvar constraints end===")
        self.constraint.append(CstrDesc("ite", [ite.condition, newblock, eblock], self))
    def typeDecl( self, toks ):
        s = toks[0]
        self.logging.info("typeDecl: %s", s)
        self.setType(s.var, s.type)
    def regularHeader(self):
        if len(self.dfa) > 0:
            decl = ["%% DFAs"]
            for k in self.dfa.keys():
                dfadecl = "% {op} /{re}/\n{decl}".format(op=k[0],re=k[1],decl=self.dfa[k])
                decl.append(dfadecl)
            return decl
        return ""
    def __str__(self):
        retval =  "\n".join(self.header) + "\n\n"
        decl = []
        for (k, v) in self.vType.items():
            decl.append(MznModel.declarations[v].format(k))
        retval += "\n".join(decl) + "\n\n" 
        retval += "\n".join(self.regularHeader())
        decl = ["%%% constant strings"]
        for (var, cstr) in self.constStr.items():
            decl.append("% {} = '{}'".format(var, cstr))
            decl.append("array[1..{}] of symbols: {} = {};".format(len(cstr) + 1, var, map(lambda c: ord(c)-31, list(cstr)) + [96]))
            decl.append("int: len_{} = {};".format(var, len(cstr)))
        retval += "\n".join(decl) + "\n\n"
        retval += "\n".join(map(str, self.constraint))
        retval += self.footer.format(self.vOut)
        return retval
def hex2short(hexint, neg=False):
    sign = 1
    if neg:
        sign = -1
    h = hexint.zfill(8)
    if int(h[0],16) < 8:
        return int(h,16) * sign
    else:
        return (int(h,16) - 0xffffffff - 1) * sign
def parser(mzn):
    LPAREN, RPAREN, SEMI, LBRACE, RBRACE, COMMA, COLON = map(Suppress, "();{},:")
    ASSERT = Keyword("ASSERT")
    BOOLEAN = (Keyword("true") | Keyword("false"))("bool")
    OPNDTYPE = Keyword("CSTR").setParseAction(replaceWith("str"))
    IF = Keyword("IF")
    ELSE = Keyword("ELSE")
    TYPE = Keyword("type")
    LEN = Keyword("Len")
    VAR = Word(alphas, alphanums+'_').setParseAction(mzn.addVariable)
    NORMSTRING = dblQuotedString
    STRING = (Keyword("symb") | Keyword("conc") | Keyword("SELECT")| NORMSTRING).setParseAction(mzn.addConstString)
    CAPBRACK = Keyword("CapturedBrack").suppress()
    DECNUMBER = Combine(Optional("-") + Word(nums)).setParseAction(lambda tokens : int(tokens[0]))
    HEXNUMBER = Combine(Literal("0x").suppress() + Word(hexnums)).setParseAction(lambda tokens : hex2short(tokens[0]))
    NEGHEXNUMBER = Combine(Literal("-0x").suppress() + Word(hexnums)).setParseAction(lambda tokens : hex2short(tokens[0],True))
    NUMBER = (HEXNUMBER | NEGHEXNUMBER | DECNUMBER)("int")
    REGEX = QuotedString("/", escChar="\\", unquoteResults=True)("regex")
    IN = Keyword("\\in")("in")
    NIN = Keyword("\\nin")("nin")
    NOTIN = Keyword("\\notin")("notin")

    coreopnd = VAR | NUMBER | STRING | BOOLEAN

    # constraints:
    predicate = Group(ASSERT + LPAREN + coreopnd("boolvar") + RPAREN)("predicate")
    boolops = (Group(VAR("lhs") + ":=" + "!" + BOOLEAN("rhs")) \
            | Group(VAR("lhs") + ":=" + "!" + VAR("rhs")))("boolops")
    intops = Group(VAR("lhs") + ":=" + coreopnd("term1") + oneOf("+ - * /")("op") + coreopnd("term2"))("intopts")
    compops = Group(VAR("lhs") + ":=" + coreopnd("term1") + oneOf("== != < <= >= >")("comp") + coreopnd("term2"))("compops")
    length  = (Group(coreopnd("lhs") + Literal("==")("relation") + LEN + LPAREN + coreopnd("rhs") + RPAREN) \
            | Group(NUMBER("lhs") + Literal("<")("relation") + LEN + LPAREN + coreopnd("rhs") + RPAREN))("length")
    rel = Group(coreopnd("lhs") + oneOf("== != < > <= >=")("relation") + coreopnd("rhs"))("rel")
    regular = Group(VAR("lhs") + (IN | NIN | NOTIN)("memop") + \
             CAPBRACK + LPAREN + (REGEX | STRING)("language") + \
             COMMA + NUMBER("position") + RPAREN)("regular")
    concat = Group(VAR("lhs") + ":=" + coreopnd("rhs1") + Literal('.').suppress() + coreopnd("rhs2"))("concat")
    for cname in ("boolops intops compops length predicate rel regular concat".split()):
        v = vars()[cname]
        v.setParseAction(getattr(mzn, cname + "Constraint"))
    typedecl = Group(TYPE + VAR("var") + COLON + OPNDTYPE("type")).setParseAction(mzn.typeDecl)
    stmt = ((typedecl | predicate | concat | length | compops | regular | boolops | intops | rel) + SEMI)
    block = LBRACE + OneOrMore(stmt)("block") + RBRACE
    ifelse = Group(IF + LPAREN + coreopnd("condition") + RPAREN \
             + block("then_") \
             + ELSE + block.setResultsName("else_"))("ite").setParseAction(mzn.ifelseStmt)
    constraints = OneOrMore(ifelse | stmt)("statements")
    return constraints + stringEnd

if __name__ == "__main__":
    import sys
    mznversionstr = {1: "1.6", 2: "2.0"}
    ap = argparse.ArgumentParser(description="Translate a kaluza input file into a minizinc model")
    ap.add_argument("-f", "--filename", type=str, help="kaluza input file name")
    ap.add_argument("-o", "--output", type=str, help="minizinc output file name (default: filename.mzn)")
    ap.add_argument("-l", "--log", type=str, help="set logging level (default: warn)", default="warn")
    ap.add_argument("-v", "--version", type=int, choices = [1, 2], help="version of MiniZinc output", default=1)
    ap.add_argument("-b", "--bounded", help="use gecode-string constraint (bounded length strings)", action="store_true")
    args = ap.parse_args()
    if args.log != None:
        level = getattr(logging, args.log.upper(), None)
        if not isinstance(level, int):
            raise ValueError('Invalid log level: %s' % args.log)
        logging.basicConfig(level=level)
    else:
        logging.basicConfig(level=logging.WARNING)
    outfilename = "output.mzn"
    if args.output != None:
        outfilename = args.output
    elif args.filename != None:
        name, ext = path.splitext(args.filename)
        outfilename = name + ".mzn"
    if args.filename != None:
        f = open(args.filename, 'r')
        text = f.read()
    mzn = MznModel(args.version, args.bounded)
    try:
        result = parser(mzn).parseString(text,parseAll=True)
        out = open(outfilename, 'w')
        out.write("%% Minizinc model for {}\n%% Automatically generated by kaluzish\n%% Requires MiniZinc version >= {}\n".format(args.filename, mznversionstr[args.version]))
        out.write(str(mzn))
    except (ParseException, ParseSyntaxException) as err: 
        logger.error("[%s] parsing error:\n%s\n%s^",args.filename, err.line,
            " " * (err.column - 1))
    except (TypeError, ValueError) as err:
        logger.error("[%s] model generation error:\n\t%s",args.filename, err)
    except:
        logger.error("[%s] mzn generation failed", args.filename)
    
