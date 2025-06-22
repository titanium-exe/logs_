
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

/// decafStmtList - List of Decaf statements
class decafStmtList : public decafAST {
	list<decafAST *> stmts;
public:
	decafStmtList() {}
	~decafStmtList() {
		for (list<decafAST *>::iterator i = stmts.begin(); i != stmts.end(); i++) { 
			delete *i;
		}
	}
	int size() { return stmts.size(); }
	void push_front(decafAST *e) { stmts.push_front(e); }
	void push_back(decafAST *e) { stmts.push_back(e); }
	string str() { return commaList<class decafAST *>(stmts); }
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

class FieldDeclAST : public decafAST {
	string Name;
	decafAST *Type;
public:
	FieldDeclAST(string name, decafAST *type) : Name(name), Type(type) {}
	~FieldDeclAST() { if (Type != NULL) delete Type; }
	string str() {
		return string("FieldDecl") + "(" + Name + "," + getString(Type) + ")";
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
	string str() { return string("VariableExpr") + "(" + Name + ")"; }
};

class AssignAST : public decafAST {
	decafAST *LHS, *RHS;
public:
	AssignAST(decafAST *lhs, decafAST *rhs) : LHS(lhs), RHS(rhs) {}
	~AssignAST() { if (LHS) delete LHS; if (RHS) delete RHS; }
	string str() {
		return string("AssignVar") + "(" + getString(LHS) + "," + getString(RHS) + ")";
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
    string str() { return "VarDecl(" + name + "," + getString(type) + ")"; }
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
