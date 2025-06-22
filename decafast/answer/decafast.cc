
#include "default-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>

#ifndef YYTOKENTYPE
#include "decafast.tab.h"
#endif

using namespace std;

/// decafAST - Base class for all abstract syntax tree nodes.
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

/// decafStmtList – list of AST nodes
class decafStmtList : public decafAST {
    std::list<decafAST *> stmts;
public:
    decafStmtList() {}
    ~decafStmtList() {
        for (auto *p : stmts) delete p;
    }

    /* ---------- helpers ---------- */
    int  size()        { return stmts.size(); }
    void push_front(decafAST *e) { stmts.push_front(e); }
    void push_back (decafAST *e) { stmts.push_back (e); }

    /**  NEW: splice another decafStmtList into this one  */
    void merge(decafStmtList *other) {
        if (!other) return;               // safety check
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
		if (FieldDeclList != NULL) { delete FieldDeclList; }
		if (MethodDeclList != NULL) { delete MethodDeclList; }
	}
	string str() { 
		return string("Package") + "(" + Name + "," + getString(FieldDeclList) + "," + getString(MethodDeclList) + ")";
	}
};

/// ProgramAST - the decaf program
class ProgramAST : public decafAST {
	decafStmtList *ExternList;
	PackageAST *PackageDef;
public:
	ProgramAST(decafStmtList *externs, PackageAST *c) : ExternList(externs), PackageDef(c) {}
	~ProgramAST() { 
		if (ExternList != NULL) { delete ExternList; } 
		if (PackageDef != NULL) { delete PackageDef; }
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
	~FieldDeclAST() { if (Type != NULL) delete Type; }
	string str() {
        return "FieldDecl(" + Name + "," + getString(Type) + ",Scalar)";
    }

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
		if (v != NULL) {
			Name = v->getName();
			delete v;
		}
	}
	~AssignAST() {
		if (Expr != NULL) delete Expr;
	}
	string str() {
		return string("AssignVar") + "(" + Name + "," + getString(Expr) + ")";
	}
};


class MethodBlockAST : public decafAST {
    decafStmtList *varList;
    decafStmtList *stmtList;
public:
    MethodBlockAST(decafStmtList *vars, decafStmtList *stmts) : varList(vars), stmtList(stmts) {}
    ~MethodBlockAST() {
        if (varList) delete varList;
        if (stmtList) delete stmtList;
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
		if (Args) delete Args;
		if (ReturnType) delete ReturnType;
		if (Block) delete Block;
	}
	string str() {
		return string("Method") + "(" + Name + "," + getString(ReturnType) + "," + getString(Args) + "," + getString(Block) + ")";
	}
};

class MethodCallAST : public decafAST {
    string                name;
    decafStmtList        *args;
public:
    MethodCallAST(const string &n, decafStmtList *a)
        : name(n), args(a) {}
    ~MethodCallAST() { if (args) delete args; }
    string str() {
        return "MethodCall(" + name + "," + getString(args) + ")";
    }
};

// Expression ASTs

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
	~UnaryMinusAST() { if (Expr) delete Expr; }
	string str() {
		return string("UnaryExpr(UnaryMinus,") + getString(Expr) + ")";
	}
};

class NotAST : public decafAST {
	decafAST *Expr;
public:
	NotAST(decafAST *e) : Expr(e) {}
	~NotAST() { if (Expr) delete Expr; }
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

class VarDeclAST : public decafAST {
    string name;
    decafAST *type;
public:
    VarDeclAST(string id, decafAST *t) : name(id), type(t) {}
    ~VarDeclAST() { if (type) delete type; }
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
        : varDecls(decls), stmts(stmts) {}
    ~BlockAST() {
        if (varDecls) delete varDecls;
        if (stmts) delete stmts;
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
        if (cond) delete cond;
        if (stmt) delete stmt;
    }
    string str() {
        return "WhileStmt(" + getString(cond) + "," + getString(stmt) + ")";
    }
};

class BreakStmtAST : public decafAST {
public:
    string str() { return "BreakStmt"; }
};

class ContinueStmtAST : public decafAST {
public:
    string str() { return "ContinueStmt"; }
};



class IfStmtAST : public decafAST {
    decafAST *cond, *thenBlk, *elseBlk;          // elseBlk == nullptr ⇢ “no else”
public:
    IfStmtAST(decafAST *c, decafAST *t, decafAST *e)
        : cond(c), thenBlk(t), elseBlk(e) {}
    ~IfStmtAST() { delete cond; delete thenBlk; delete elseBlk; }
    string str() {
        return "IfStmt(" + getString(cond) + "," +
                           getString(thenBlk) + "," +
                           getString(elseBlk) + ")";
    }
};

class ReturnStmtAST : public decafAST {
    decafAST *value;                 // nullptr ⇢ “return;”
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
        if (rettype) delete rettype;
        if (params) delete params;
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
    ~VarDefAST() { if (type) delete type; }
    string str() {
        return "VarDef(" + name + "," + getString(type) + ")";
    }
};




// Binary operations

#define MAKE_BINOP_CLASS(CLASSNAME, LABEL) \
class CLASSNAME : public decafAST { \
	decafAST *LHS, *RHS; \
public: \
	CLASSNAME(decafAST *lhs, decafAST *rhs) : LHS(lhs), RHS(rhs) {} \
	~CLASSNAME() { if (LHS) delete LHS; if (RHS) delete RHS; } \
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
MAKE_BINOP_CLASS(LessThanAST, "LessThan")
MAKE_BINOP_CLASS(GreaterThanAST, "GreaterThan")
MAKE_BINOP_CLASS(LessEqualAST, "LessEqual")
MAKE_BINOP_CLASS(GreaterEqualAST, "GreaterEqual")
MAKE_BINOP_CLASS(EqualAST, "Equal")
MAKE_BINOP_CLASS(NotEqualAST, "NotEqual")
MAKE_BINOP_CLASS(AndAST, "And")
MAKE_BINOP_CLASS(OrAST, "Or")
