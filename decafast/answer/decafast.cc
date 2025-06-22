#include "default-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>

#ifndef YYTOKENTYPE
#endif

using namespace std;

class decafAST {
public:
  virtual ~decafAST() {}
  virtual string str() { return string(""); }
};

string getString(decafAST *d) {
  if (d != NULL) {
    return d->str();
  } else {
    return string("None");
  }
}

template <class T>
string commaList(list<T> vec) {
  string s("");
  for (typename list<T>::iterator i = vec.begin(); i != vec.end(); i++) {
    s = s + (s.empty() ? string("") : string(",")) + (*i)->str();
  }
  if (s.empty()) {
    s = string("None");
  }
  return s;
}

class decafStmtList : public decafAST {
  std::list<decafAST *> stmts;
public:
  decafStmtList() {}
  ~decafStmtList() {
    for (auto *p : stmts) delete p;
  }
  int size() { return stmts.size(); }
  void push_front(decafAST *e) { stmts.push_front(e); }
  void push_back(decafAST *e) { stmts.push_back(e); }
  void merge(decafStmtList *other) {
    if (!other) return;
    stmts.splice(stmts.end(), other->stmts);
  }
  string str() { return commaList<decafAST *>(stmts); }
};

class PackageAST : public decafAST {
  string Name;
  decafStmtList *FieldDeclList;
  decafStmtList *MethodDeclList;
public:
  PackageAST(string name, decafStmtList *fieldlist, decafStmtList *methodlist)
    : Name(name), FieldDeclList(fieldlist), MethodDeclList(methodlist) {}
  ~PackageAST() {
    delete FieldDeclList;
    delete MethodDeclList;
  }
  string str() {
    return string("Package") + "(" + Name + "," + getString(FieldDeclList) + "," + getString(MethodDeclList) + ")";
  }
};

class ProgramAST : public decafAST {
  decafStmtList *ExternList;
  PackageAST *PackageDef;
public:
  ProgramAST(decafStmtList *externs, PackageAST *c) : ExternList(externs), PackageDef(c) {}
  ~ProgramAST() {
    delete ExternList;
    delete PackageDef;
  }
  string str() { return string("Program") + "(" + getString(ExternList) + "," + getString(PackageDef) + ")"; }
};

class IntTypeAST : public decafAST {
public:
  string str() { return string("IntType"); }
};

class BoolTypeAST : public decafAST {
public:
  string str() { return "BoolType"; }
};

class StringTypeAST : public decafAST {
public:
  string str() { return "StringType"; }
};

class FieldDeclAST : public decafAST {
  string Name;
  decafAST *Type;
public:
  FieldDeclAST(string name, decafAST *type) : Name(name), Type(type) {}
  ~FieldDeclAST() { delete Type; }
  string str() { return "FieldDecl(" + Name + "," + getString(Type) + ",Scalar)"; }
};

class VoidTypeAST : public decafAST {
public:
  string str() { return string("VoidType"); }
};

class VariableAST : public decafAST {
  string Name;
public:
  VariableAST(string name) : Name(name) {}
  string getName() { return Name; }
  string str() { return string("VariableExpr") + "(" + Name + ")"; }
};

class AssignAST : public decafAST {
  string Name;
  decafAST *Expr;
public:
  AssignAST(decafAST *lval, decafAST *expr) : Expr(expr) {
    VariableAST *v = dynamic_cast<VariableAST *>(lval);
    Name = v ? v->getName() : "";
    delete lval;  // always delete lval once it's cast and used
  }
  ~AssignAST() {
    delete Expr;
  }
  string str() {
    return string("AssignVar") + "(" + Name + "," + getString(Expr) + ")";
  }
};

class MethodBlockAST : public decafAST {
  decafStmtList *varList;
  decafStmtList *stmtList;
public:
  MethodBlockAST(decafStmtList *vars, decafStmtList *stmts)
    : varList(vars ? vars : new decafStmtList()), stmtList(stmts ? stmts : new decafStmtList()) {}
  ~MethodBlockAST() {
    delete varList;
    delete stmtList;
  }
  string str() {
    return "MethodBlock(" + getString(varList) + "," + getString(stmtList) + ")";
  }
};

class MethodDeclAST : public decafAST {
  string Name;
  decafStmtList *Args;
  decafAST *ReturnType;
  MethodBlockAST *Block;
public:
  MethodDeclAST(string name, decafStmtList *args, decafAST *rtype, decafAST *block)
    : Name(name), Args(args), ReturnType(rtype), Block((MethodBlockAST *)block) {}
  ~MethodDeclAST() {
    delete Args;
    delete ReturnType;
    delete Block;
  }
  string str() {
    return string("Method") + "(" + Name + "," + getString(ReturnType) + "," + getString(Args) + "," + getString(Block) + ")";
  }
};


class MethodCallAST : public decafAST {
  string name;
  decafStmtList *args;
public:
  MethodCallAST(const string &n, decafStmtList *a) : name(n), args(a ? a : new decafStmtList()) {}
  ~MethodCallAST() { delete args; }
  string str() {
    return "MethodCall(" + name + "," + getString(args) + ")";
  }
};

class ContinueStmtAST : public decafAST {
  string str() { return "ContinueStmt"; }
};

class IntConstantAST : public decafAST {
  int Value;
public:
  IntConstantAST(int val) : Value(val) {}
  string str() {
    ostringstream oss;
    oss << "NumberExpr(" << Value << ")";
    return oss.str();
  }
};

class UnaryMinusAST : public decafAST {
  decafAST *Expr;
public:
  UnaryMinusAST(decafAST *e) : Expr(e) {}
  ~UnaryMinusAST() { delete Expr; }
  string str() {
    return string("UnaryExpr(UnaryMinus,") + getString(Expr) + ")";
  }
};

class NotAST : public decafAST {
  decafAST *Expr;
public:
  NotAST(decafAST *e) : Expr(e) {}
  ~NotAST() { delete Expr; }
  string str() {
    return string("UnaryExpr(Not,") + getString(Expr) + ")";
  }
};

class CharConstantAST : public decafAST {
  char val;
public:
  CharConstantAST(char v) : val(v) {}
  string str() { return "CharExpr(" + string(1, val) + ")"; }
};

class BoolExprAST : public decafAST {
  bool Val;
public:
  BoolExprAST(bool v) : Val(v) {}
  string str() { return string("BoolExpr(") + (Val ? "True" : "False") + ")"; }
};

class StringConstantAST : public decafAST {
  std::string val;
public:
  explicit StringConstantAST(const std::string &v) : val(v) {}
  std::string str() { return "StringConstant(" + val + ")"; }
};

class TypeOnlyVarDefAST : public decafAST {
  decafAST *type;
public:
  explicit TypeOnlyVarDefAST(decafAST *t) : type(t) {}
  ~TypeOnlyVarDefAST() { delete type; }
  std::string str() { return "VarDef(" + getString(type) + ")"; }
};

class VarDeclAST : public decafAST {
  string name;
  decafAST *type;
public:
  VarDeclAST(string id, decafAST *t) : name(id), type(t) {}
  ~VarDeclAST() { delete type; }
  string str() {
    return "VarDef(" + name + "," + getString(type) + ")";
  }
};

class BoolConstantAST : public decafAST {
  bool Value;
public:
  BoolConstantAST(bool val) : Value(val) {}
  string str() { return string("BoolExpr(") + (Value ? "True" : "False") + ")"; }
};

class BlockAST : public decafAST {
  decafStmtList* varDecls;
  decafStmtList* stmts;
public:
  BlockAST(decafStmtList* decls, decafStmtList* stmts)
    : varDecls(decls ? decls : new decafStmtList()), stmts(stmts ? stmts : new decafStmtList()) {}
  ~BlockAST() {
    delete varDecls;
    delete stmts;
  }
  string str() {
    return "Block(" + getString(varDecls) + "," + getString(stmts) + ")";
  }
};

class WhileStmtAST : public decafAST {
  decafAST *cond;
  decafAST *stmt;
public:
  WhileStmtAST(decafAST *c, decafAST *s) : cond(c), stmt(s) {}
  ~WhileStmtAST() {
    delete cond;
    delete stmt;
  }
  string str() {
    return "WhileStmt(" + getString(cond) + "," + getString(stmt) + ")";
  }
};

class BreakStmtAST : public decafAST {
public:
  string str() { return "BreakStmt"; }
};

class IfStmtAST : public decafAST {
  decafAST *cond, *thenBlk, *elseBlk;
public:
  IfStmtAST(decafAST *c, decafAST *t, decafAST *e)
    : cond(c), thenBlk(t), elseBlk(e) {}
  ~IfStmtAST() {
    delete cond;
    delete thenBlk;
    delete elseBlk;
  }
  string str() {
    return "IfStmt(" + getString(cond) + "," + getString(thenBlk) + "," + getString(elseBlk) + ")";
  }
};

class ReturnStmtAST : public decafAST {
  decafAST *value;
public:
  ReturnStmtAST(decafAST *v) : value(v) {}
  ~ReturnStmtAST() { delete value; }
  string str() { return "ReturnStmt(" + getString(value) + ")"; }
};

class ExternFunctionAST : public decafAST {
  string name;
  decafAST *rettype;
  decafStmtList *params;
public:
  ExternFunctionAST(string n, decafAST *rtype, decafStmtList *p)
    : name(n), rettype(rtype), params(p) {}
  ~ExternFunctionAST() {
    delete rettype;
    delete params;
  }
  string str() {
    return "ExternFunction(" + name + "," + getString(rettype) + "," + getString(params) + ")";
  }
};

class VarDefAST : public decafAST {
  string name;
  decafAST *type;
public:
  VarDefAST(string n, decafAST *t) : name(n), type(t) {}
  ~VarDefAST() { delete type; }
  string str() {
    return "VarDef(" + name + "," + getString(type) + ")";
  }
};

class ForStmtAST : public decafAST {
    decafAST *init;
    decafAST *cond;
    decafAST *incr;
    decafAST *body;
public:
    ForStmtAST(decafAST *i, decafAST *c, decafAST *inc, decafAST *b)
        : init(i), cond(c), incr(inc), body(b) {}
    ~ForStmtAST() {
        if (init) delete init;
        if (cond) delete cond;
        if (incr) delete incr;
        if (body) delete body;
    }
    string str() {
        return "ForStmt(" + getString(init) + "," + getString(cond) + "," + getString(incr) + "," + getString(body) + ")";
    }
};


#define MAKE_BINOP_CLASS(CLASSNAME, LABEL) \
class CLASSNAME : public decafAST { \
  decafAST *LHS, *RHS; \
public: \
  CLASSNAME(decafAST *lhs, decafAST *rhs) : LHS(lhs), RHS(rhs) {} \
  ~CLASSNAME() { delete LHS; delete RHS; } \
  string str() { \
    return string("BinaryExpr(") + LABEL + "," + getString(LHS) + "," + getString(RHS) + ")"; \
  } \
};

MAKE_BINOP_CLASS(PlusAST, "Plus")
MAKE_BINOP_CLASS(MinusAST, "Minus")
MAKE_BINOP_CLASS(MultAST, "Mult")
MAKE_BINOP_CLASS(DivAST, "Div")
MAKE_BINOP_CLASS(ModAST, "Mod")
MAKE_BINOP_CLASS(LeftShiftAST, "Leftshift")
MAKE_BINOP_CLASS(RightShiftAST, "Rightshift")
MAKE_BINOP_CLASS(LessThanAST, "Lt")
MAKE_BINOP_CLASS(GreaterThanAST, "Gt")
MAKE_BINOP_CLASS(LessEqualAST, "Leq")
MAKE_BINOP_CLASS(GreaterEqualAST, "Geq")
MAKE_BINOP_CLASS(EqualAST, "Eq")
MAKE_BINOP_CLASS(NotEqualAST, "Neq")
MAKE_BINOP_CLASS(AndAST, "And")
MAKE_BINOP_CLASS(OrAST, "Or")
