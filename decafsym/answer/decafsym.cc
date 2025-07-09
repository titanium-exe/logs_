#include "default-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>
#include "symbol_table.h"
SymbolStack gSym;  

#ifndef YYTOKENTYPE
#endif

using namespace std;

// Helper Func 
inline void printIndent(std::ostream& out, int indent) {
    for (int i = 0; i < indent; ++i) out << "  ";
}

class decafAST {
protected:
    int line;
public:
    decafAST(int l = -1) : line(l) {}
    virtual ~decafAST() {}
    virtual std::string str() { return ""; }
    virtual void Analyze() {}
    virtual void prettyPrint(std::ostream& out, int indent = 0) {}
    int getLine() const { return line; }


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
  for (typename list<T>::iterator i = vec.begin(); i != vec.end(); ++i) {
    s = s + (s.empty() ? string("") : string(",")) + (*i)->str();
  }
  if (s.empty()) {
    s = string("None");
  }
  return s;
}
// Analyze for these ?? 
class IntTypeAST : public decafAST {
public:
  void prettyPrint(std::ostream& out, int indent = 0) override {
    out << "int"; 
  }
  string str()  override  { return string("IntType"); }
};

class BoolTypeAST : public decafAST {
public:
  void prettyPrint(std::ostream& out, int indent = 0) override {
    out << "bool"; 
  }
  string str()  override { return "BoolType"; }
};

class StringTypeAST : public decafAST {
public:
  void prettyPrint(std::ostream& out, int indent = 0) override {
    out << "string"; 
  }
  string str() override  { return "StringType"; }
};

class VoidTypeAST : public decafAST {
public:
  void prettyPrint(std::ostream& out, int indent = 0) override {
    out << "void"; 
  }
  string str()  override  { return string("VoidType"); }
};



// Helper Functions 
inline DecafType astToType(decafAST* t) {

  if (dynamic_cast<IntTypeAST*>(t))    return TYPE_INT;
  if (dynamic_cast<BoolTypeAST*>(t))   return TYPE_BOOL;
  if (dynamic_cast<StringTypeAST*>(t)) return TYPE_STRING;
  if (dynamic_cast<VoidTypeAST*>(t))   return TYPE_VOID;
  return TYPE_UNKNOWN;
}



class VarDeclAST : public decafAST {
    std::string name;
    decafAST   *type;
public:
    VarDeclAST(const std::string& id, decafAST* t, int l)
        : decafAST(l), name(id), type(t) {}
    ~VarDeclAST() { delete type; }

    void Analyze() override {
        DecafType dtype = astToType(type);
        if (!gSym.insert(name, dtype, getLine())) {
            std::cerr << "Error: parameter '" << name
                      << "' redeclared (line " << getLine() << ")\n";
        } else {                                        
            std::cerr << "defined variable: " << name
                      << ", with type: " << typeToString(dtype)
                      << ", on line number: " << getLine() << '\n';
        }
    }
    
    std::string getName()  const { return name; }
    std::string getTypeString() const {
        if (dynamic_cast<IntTypeAST*>(type))    return "int";
        if (dynamic_cast<BoolTypeAST*>(type))   return "bool";
        if (dynamic_cast<StringTypeAST*>(type)) return "string";
        return "unknown";
    }

    

    void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent);
      out << "var " << name << " ";
      if (type) type->prettyPrint(out, 0);
      out << "; \n";
    }

    std::string str() override {
        return "VarDef(" + name + "," + getString(type) + ")";
    }
};


class decafStmtList : public decafAST {
  std::list<decafAST *> stmts;
public:
  decafStmtList(int l = -1) : decafAST(l) {}
  ~decafStmtList() {
    for (auto *p : stmts) delete p;
  }
  int size() { return stmts.size(); }
  void push_front(decafAST *e) { stmts.push_front(e); }
  void push_back(decafAST *e) { stmts.push_back(e); }
  const std::list<decafAST*>& getStmts() const { return stmts; }
  void merge(decafStmtList *other) {
    if (!other) return;
    stmts.splice(stmts.end(), other->stmts);
  }
  void Analyze() override {
    for (auto *stmt : stmts) if (stmt) stmt->Analyze();
  }

  void prettyPrint(std::ostream& out, int indent = 0) override {
    auto it = stmts.begin();
    while (it != stmts.end()) {
        auto* firstVar = dynamic_cast<VarDeclAST*>(*it);
        if (firstVar) {
            std::vector<std::string> names;
            auto varType = firstVar->getTypeString();
            int line = firstVar->getLine();

            names.push_back(firstVar->getName());

            auto jt = it;
            ++jt;
            while (jt != stmts.end()) {
                auto* nextVar = dynamic_cast<VarDeclAST*>(*jt);
                if (nextVar && nextVar->getTypeString() == varType && nextVar->getLine() == line) {
                    names.push_back(nextVar->getName());
                    ++jt;
                } else {
                    break;
                }
            }

            printIndent(out, indent);
            out << "var ";
            for (size_t i = 0; i < names.size(); ++i) {
                if (i > 0) out << ", ";
                out << names[i];
            }
            out << " " << varType << "; \n";

            it = jt;
        } else {
            if (*it) (*it)->prettyPrint(out, indent);
            ++it;
        }
    }
  }


  string str()  override { return commaList<decafAST *>(stmts); }
};

class PackageAST : public decafAST {
  string Name;
  decafStmtList *FieldDeclList;
  decafStmtList *MethodDeclList;
public:
  PackageAST(const std::string& name, decafStmtList* f, decafStmtList* m, int l) 
      : decafAST(l), Name(name), FieldDeclList(f), MethodDeclList(m) {}
  ~PackageAST() {
    delete FieldDeclList;
    delete MethodDeclList;
  }
  void Analyze() override {
    gSym.push();
    if (FieldDeclList) FieldDeclList->Analyze();
    if (MethodDeclList) MethodDeclList->Analyze();
    gSym.pop();
  }
  void prettyPrint(std::ostream& out, int indent = 0) override {
    printIndent(out, indent); out << "package " << Name << " {\n";
    if (FieldDeclList) FieldDeclList->prettyPrint(out, indent+1);
    if (MethodDeclList) MethodDeclList->prettyPrint(out, indent+1);
    printIndent(out, indent); out << "}\n";
  }

  string str()  override  {
    return string("Package") + "(" + Name + "," + getString(FieldDeclList) + "," + getString(MethodDeclList) + ")";
  }
};

class ProgramAST : public decafAST {
  decafStmtList *ExternList;
  PackageAST *PackageDef;
public:
   ProgramAST(decafStmtList *externs, PackageAST *c, int l)
        : decafAST(l), ExternList(externs), PackageDef(c) {}
  ~ProgramAST() {
    delete ExternList;
    delete PackageDef;
  }
  void Analyze() override {
    gSym.push();
    if (ExternList) ExternList->Analyze();
    if (PackageDef) PackageDef->Analyze();
    gSym.pop();
  }
  void prettyPrint(std::ostream& out, int indent = 0) override {
    if (ExternList) ExternList->prettyPrint(out, indent);
    if (PackageDef) PackageDef->prettyPrint(out, indent);
  }

  string str()  override  { return string("Program") + "(" + getString(ExternList) + "," + getString(PackageDef) + ")"; }
};


class FieldDeclAST : public decafAST {
    std::string Name;
    decafAST   *Type;
    int         len;           

public:
    
    FieldDeclAST(const std::string& n,
                 decafAST*          t,
                 int                l)          
        : decafAST(l), Name(n), Type(t), len(-1) {}

  
    FieldDeclAST(const std::string& n,
                 decafAST*          t,
                 int                size,       
                 int                l)         
        : decafAST(l), Name(n), Type(t), len(size) {}

    ~FieldDeclAST() { delete Type; }
    void Analyze() override {
        DecafType dtype = TYPE_UNKNOWN;
        if (dynamic_cast<IntTypeAST*>(Type)) dtype = TYPE_INT;
        else if (dynamic_cast<BoolTypeAST*>(Type)) dtype = TYPE_BOOL;
        else if (dynamic_cast<StringTypeAST*>(Type)) dtype = TYPE_STRING;
        else if (dynamic_cast<VoidTypeAST*>(Type)) dtype = TYPE_VOID;
        if (!gSym.insert(Name, dtype, getLine())) {
            std::cerr << "Error: field '" << Name << "' redeclared (line " << getLine() << ")\n";
        } else {                                    
        std::cerr << "defined variable: " << Name
                  << ", with type: "  << typeToString(dtype)
                  << ", on line number: " << getLine() << '\n';
        }
    }
    
    void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent);
      out << "var " << Name << " ";
      if (Type) Type->prettyPrint(out, 0);
      out << ";\n";
    }


    std::string str() override {
        const std::string tail =
            (len < 0 ? "Scalar"
                     : "Array(" + std::to_string(len) + ")");
        return "FieldDecl(" + Name + "," + getString(Type) + "," + tail + ")";
    }
};


class FieldDeclArrayAST : public decafAST {
    std::string Name;
    decafAST   *Type;
    int         Size;
public:
    FieldDeclArrayAST(const std::string& n,
                      decafAST*          t,
                      int                sz,
                      int                l)             
        : decafAST(l), Name(n), Type(t), Size(sz) {}

    ~FieldDeclArrayAST() { delete Type; }

    void Analyze() override {
        DecafType dtype = TYPE_UNKNOWN;
        if (dynamic_cast<IntTypeAST*>(Type)) dtype = TYPE_INT;
        else if (dynamic_cast<BoolTypeAST*>(Type)) dtype = TYPE_BOOL;
        if (!gSym.insert(Name, dtype, getLine())) {
            std::cerr << "Error: array field '" << Name << "' redeclared (line " << getLine() << ")\n";
        } else {                                    
        std::cerr << "defined variable: " << Name
                  << ", with type: "  << typeToString(dtype)
                  << ", on line number: " << getLine() << '\n';
       }
    }

    std::string str() override {
        std::ostringstream os;
        os << "FieldDecl(" << Name << "," << getString(Type)
           << ",Array(" << Size << "))";
        return os.str();
    }
};


class ArrayLocExprAST : public decafAST {
    std::string name;  decafAST *index;   int declLine = -1;      
public:
    ArrayLocExprAST(const std::string& n, decafAST* idx, int l) : decafAST(l), name(n), index(idx) {}
    ~ArrayLocExprAST() { delete index; }
    void Analyze() override {
      if (auto *sym = gSym.lookup(name)) {
            declLine = sym->lineDeclared;     
        } else {
          std::cerr << "Error: array variable '" << name << "' not declared (line " << getLine() << ")\n";
      }
      if (index) index->Analyze();
    }

    std::string str()  override  {
        return "ArrayLocExpr(" + name + "," + getString(index) + ")";
    }
};

class AssignArrayLocAST : public decafAST {
    std::string name;  decafAST *index;  decafAST *expr;   int declLine = -1;      
public:
    AssignArrayLocAST(const std::string& n, decafAST* idx, decafAST* e, int l) : decafAST(l), name(n), index(idx), expr(e) {}
    ~AssignArrayLocAST() { delete index; delete expr; }
    void Analyze() override {
        if (auto *sym = gSym.lookup(name)) {
            declLine = sym->lineDeclared;     
        } else {
            std::cerr << "Error: array '" << name << "' not declared (line " << getLine() << ")\n";
        }
        if (index) index->Analyze();
        if (expr) expr->Analyze();
    }

    void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent);
      out << name << "[";
      if (index) index->prettyPrint(out, 0);
      out << "] = ";
      if (expr) expr->prettyPrint(out, 0);
      out << ";\n";
    }

    std::string str()  override {
        return "AssignArrayLoc(" + name + "," +
               getString(index) + "," + getString(expr) + ")";
    }
};

class ArrayFieldDeclAST : public FieldDeclAST {
public:
    using FieldDeclAST::FieldDeclAST;   // inherit ctor
    std::string str()  override  {                
        return "ArrayFieldDecl" + FieldDeclAST::str().substr(10);
    }
};


class VariableAST : public decafAST {
    std::string Name;
    int declLine = -1;          
public:
    explicit VariableAST(const std::string& name, int l = -1)
        : decafAST(l), Name(name) {}

    const std::string& getName() const { return Name; }
    int getDeclLine()  const { return declLine; }   

    void Analyze() override {
        if (auto *sym = gSym.lookup(Name)) {
            declLine = sym->lineDeclared;     
        } else {
            std::cerr << "Error: variable '" << Name
                      << "' not declared (line " << getLine() << ")\n";
        }
    }

    void prettyPrint(std::ostream& out, int /*indent*/ = 0) override {
        out << Name;
    }

    std::string str() override { return "VariableExpr(" + Name + ")"; }
};



class AssignAST : public decafAST {
    std::string Name;
    decafAST   *Expr;
    int declLine = -1;            
public:
    AssignAST(decafAST *lval, decafAST *expr, int l)
        : decafAST(l), Expr(expr) {
        auto *v = dynamic_cast<VariableAST*>(lval);
        Name = v ? v->getName() : "";
        delete lval;
    }
    ~AssignAST() override { delete Expr; }
    
    void Analyze() override {
        if (auto *sym = gSym.lookup(Name))
            declLine = sym->lineDeclared;
        else
            std::cerr << "Error: variable '" << Name
                      << "' not declared (line " << getLine() << ")\n";
        if (Expr) Expr->Analyze();
    }

    void prettyPrint(std::ostream& out, int indent = 0) override {
        printIndent(out, indent);
        out << Name << " = ";
        if (Expr) Expr->prettyPrint(out, 0);
        out << "; // using decl on line: " << declLine << "\n\n";
    }
};

class MethodBlockAST : public decafAST {
    decafStmtList* varList;
    decafStmtList* stmtList;
public:
    MethodBlockAST(decafStmtList* vars,
                   decafStmtList* stmts,
                   int            l)         
        : decafAST(l),
          varList(vars ? vars : new decafStmtList(l)),
          stmtList(stmts ? stmts : new decafStmtList(l)) {}

    ~MethodBlockAST() { delete varList; delete stmtList; }

    void Analyze() override {
    gSym.push();
    if (varList) varList->Analyze();
    if (stmtList) stmtList->Analyze();
    gSym.pop();
    }

    void prettyPrint(std::ostream &out, int indent = 0) override
    {
        if (varList)  varList->prettyPrint(out, indent);
        if (stmtList) stmtList->prettyPrint(out, indent);
        if (varList && stmtList && stmtList->size() > 0) out << "\n";
    }


    std::string str() override {
        return "MethodBlock(" + getString(varList) + "," +
                              getString(stmtList) + ")";
    }
};

class BlockAST : public decafAST {
  decafStmtList* varDecls;
  decafStmtList* stmts;
public:
   BlockAST(decafStmtList* decls, decafStmtList* stmts, int l)
        : decafAST(l), varDecls(decls), stmts(stmts) {}
  ~BlockAST() {
    delete varDecls;
    delete stmts;
  }

  void Analyze() override {
    gSym.push();
    if (varDecls) varDecls->Analyze();
    if (stmts) stmts->Analyze();
    gSym.pop();
  }

  void prettyPrint(std::ostream& out, int indent = 0) override {
    printIndent(out, indent); out << "{\n";
    if (varDecls) varDecls->prettyPrint(out, indent+1);
    if (stmts) stmts->prettyPrint(out, indent+1);
    printIndent(out, indent); out << "}\n";
  }
  
  string str() override  {
    return "Block(" + getString(varDecls) + "," + getString(stmts) + ")";
  }
};

class MethodDeclAST : public decafAST {
  string Name;
  decafStmtList *Args;
  decafAST *ReturnType;
  MethodBlockAST *Block;
public:
   MethodDeclAST(const std::string& name, decafStmtList *args, decafAST *rtype,
                  MethodBlockAST *block, int l)
        : decafAST(l), Name(name), Args(args), ReturnType(rtype), Block(block) {}
  ~MethodDeclAST() {
    delete Args;
    delete ReturnType;
    delete Block;
  }
  
  void Analyze() override {
    DecafType rtype = astToType(ReturnType);
       if (!gSym.insert(Name, rtype, getLine())) {
        std::cerr << "Error: method '" << Name << "' redeclared (line " << getLine() << ")\n";
    }
    gSym.push(); 
    if (Args) Args->Analyze();
    if (Block) Block->Analyze();
    gSym.pop();
  }

  void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent + 1); 
      out << "func " << Name << "(";
      if (Args) {
          bool first = true;
          for (auto *arg : Args->getStmts()) {
              if (!first) out << ", ";
              arg->prettyPrint(out, 0);
              first = false;
          }
      }
      out << ") ";
      if (ReturnType) ReturnType->prettyPrint(out, 0);
      out << " {\n";
      if (Block) Block->prettyPrint(out, indent + 3); 
      printIndent(out, indent + 1);
      out << "}\n";
  }


  string str()  override {
    return string("Method") + "(" + Name + "," + getString(ReturnType) + "," + getString(Args) + "," + getString(Block) + ")";
  }
};


class MethodCallAST : public decafAST {
    std::string    name;
    decafStmtList *args;
    int declLine = -1;    
    int argLine = -1;                  
public:
    MethodCallAST(const std::string& n,
                  decafStmtList*     a,
                  int l)          
        : decafAST(l), name(n),
          args(a ? a : new decafStmtList(l)) {}

    ~MethodCallAST() { delete args; }
    void Analyze() override {
        
        if (auto *sym = gSym.lookup(name))
            declLine = sym->lineDeclared;

        if (args) {
            args->Analyze();
            for (auto *a : args->getStmts()) {
                if (auto *v = dynamic_cast<VariableAST*>(a)) {
                    argLine = v->getDeclLine();
                    break;
                }
                if (auto *ar = dynamic_cast<ArrayLocExprAST*>(a)) {
                    argLine = ar->getLine();  
                    break;
                }
            }
        }
    }

    void prettyPrint(std::ostream &out, int indent = 0) override {
        printIndent(out, indent);
        out << name << "(";
        bool first = true;
        for (auto *a : args->getStmts()) {
            if (!first) out << ", ";
            a->prettyPrint(out, 0);
            first = false;
        }
        out << ") // using decl on line: "
            << (argLine != -1 ? argLine : declLine) << "\n";
        out << ";";
    }
};


class ContinueStmtAST : public decafAST {
  public:
    ContinueStmtAST(int l) : decafAST(l) {}
    string str()  override { return "ContinueStmt"; }
    void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent); out << "continue;\n";
    }

};

class IntConstantAST : public decafAST {
    int Value;
public:
    explicit IntConstantAST(int val, int l = -1)
        : decafAST(l), Value(val) {}
    std::string str() override {
        std::ostringstream os; os << "NumberExpr(" << Value << ")";
        return os.str();
    }
    void prettyPrint(std::ostream& out, int indent = 0) override {
      out << Value;
    }

};

class UnaryMinusAST : public decafAST {
    decafAST *Expr;
public:
    explicit UnaryMinusAST(decafAST* e, int l)
        : decafAST(l), Expr(e) {}
    ~UnaryMinusAST() { delete Expr; }
    void Analyze() override {
      if (Expr) Expr->Analyze();
    }
    std::string str() override {
        return "UnaryExpr(UnaryMinus," + getString(Expr) + ")";
    }

    void prettyPrint(std::ostream& out, int indent = 0) override {
      out << ("-");
      if (Expr) Expr->prettyPrint(out, 0);
    }

};

class NotAST : public decafAST {
    decafAST *Expr;
public:
    explicit NotAST(decafAST* e, int l)
        : decafAST(l), Expr(e) {}
    ~NotAST() { delete Expr; }
    void Analyze() override {
      if (Expr) Expr->Analyze();
    }
    std::string str() override {
        return "UnaryExpr(Not," + getString(Expr) + ")";
    }
    void prettyPrint(std::ostream& out, int indent = 0) override {
      out << ("!");
      if (Expr) Expr->prettyPrint(out, 0);
    }
};

class CharConstantAST : public decafAST {
    char val;
public:
    explicit CharConstantAST(char v, int l = -1)
        : decafAST(l), val(v) {}
    std::string str() override { return "CharExpr(" + std::string(1, val) + ")"; }
};



class BoolExprAST : public decafAST {
    bool Val;
public:
    explicit BoolExprAST(bool v, int l = -1)
        : decafAST(l), Val(v) {}
    std::string str() override {
        return "BoolExpr(" + std::string(Val ? "True" : "False") + ")";
    }
};


class StringConstantAST : public decafAST {
    std::string val;
public:
    explicit StringConstantAST(const std::string& v,
                               int                l = -1)   // ← add l
        : decafAST(l), val(v) {}

    std::string str() override { return "StringConstant(" + val + ")"; }
};


class TypeOnlyVarDefAST : public decafAST {
  decafAST *type;
public:
  explicit TypeOnlyVarDefAST(decafAST *t) : type(t) {}
  ~TypeOnlyVarDefAST() { delete type; }
  std::string str()  override { return "VarDef(" + getString(type) + ")"; }
  void prettyPrint(std::ostream& out, int /*indent*/ = 0) override {
    if (type) type->prettyPrint(out, 0);
  }

};

class BoolConstantAST : public decafAST {
    bool Value;
public:
    explicit BoolConstantAST(bool val, int l = -1)
        : decafAST(l), Value(val) {}
    std::string str() override {
        return "BoolExpr(" + std::string(Value ? "True" : "False") + ")";
    }
    void prettyPrint(std::ostream& out, int indent = 0) override {
      out << (Value ? "true" : "false");
    }

};

class AssignGlobalVarAST : public decafAST {
    std::string name;
    decafAST   *type;
    decafAST   *init;
public:
    AssignGlobalVarAST(const std::string& id,
                       decafAST*          t,
                       decafAST*          val,
                       int                l)
        : decafAST(l), name(id), type(t), init(val) {}
    ~AssignGlobalVarAST() { delete type; delete init; }

    void Analyze() override {
        DecafType dtype = TYPE_UNKNOWN;
        if (dynamic_cast<IntTypeAST*>(type)) dtype = TYPE_INT;
        else if (dynamic_cast<BoolTypeAST*>(type)) dtype = TYPE_BOOL;
        if (!gSym.insert(name, dtype, getLine())) {
            std::cerr << "Error: global variable '" << name << "' redeclared (line " << getLine() << ")\n";
        } else {                                    
            std::cerr << "defined variable: " << name
                      << ", with type: "  << typeToString(dtype)
                      << ", on line number: " << getLine() << '\n';
        }
        if (init) init->Analyze();
    }

    std::string str() override {
        return "AssignGlobalVar(" + name + "," +
               getString(type) + "," + getString(init) + ")";
    }
};

class WhileStmtAST : public decafAST {
  decafAST *cond;
  decafAST *stmt;
public:
     WhileStmtAST(decafAST *c, decafAST *s, int l)
        : decafAST(l), cond(c), stmt(s) {}
  ~WhileStmtAST() {
    delete cond;
    delete stmt;
  }

  void Analyze() override {
    gSym.push();
    if (cond) cond->Analyze();
    if (stmt) stmt->Analyze();
    gSym.pop();
  }
  string str()  override {
    return "WhileStmt(" + getString(cond) + "," + getString(stmt) + ")";
  }
  void prettyPrint(std::ostream& out, int indent = 0) override {
    printIndent(out, indent); out << "while (";
    if (cond) cond->prettyPrint(out, 0);
    out << ") ";
    if (stmt) stmt->prettyPrint(out, indent);
  }

};

class BreakStmtAST : public decafAST {
public:
  BreakStmtAST(int l) : decafAST(l) {}
  string str() override  { return "BreakStmt"; }
  void prettyPrint(std::ostream& out, int indent = 0) override {
    printIndent(out, indent); out << "break;\n";
  }

};

class IfStmtAST : public decafAST {
  decafAST *cond, *thenBlk, *elseBlk;
public:
   IfStmtAST(decafAST *c, decafAST *t, decafAST *e, int l)
        : decafAST(l), cond(c), thenBlk(t), elseBlk(e) {}
  ~IfStmtAST() {
    delete cond;
    delete thenBlk;
    delete elseBlk;
  }
  void Analyze() override {
    if (cond) cond->Analyze();
    if (thenBlk) thenBlk->Analyze();
    if (elseBlk) elseBlk->Analyze();
  }
  string str() override  {
    return "IfStmt(" + getString(cond) + "," + getString(thenBlk) + "," + getString(elseBlk) + ")";
  }

  void prettyPrint(std::ostream& out, int indent = 0) override {
    printIndent(out, indent); out << "if ("; 
    if (cond) cond->prettyPrint(out, 0);
    out << ") ";
    if (thenBlk) thenBlk->prettyPrint(out, indent);
    if (elseBlk) {
        printIndent(out, indent); out << "else ";
        elseBlk->prettyPrint(out, indent);
    }
  } 
};

class ReturnStmtAST : public decafAST {
  decafAST *value;
public:
  ReturnStmtAST(decafAST *v, int l) : decafAST(l), value(v) {}
  ~ReturnStmtAST() { delete value; }
  void Analyze() override {
    if (value) value->Analyze();
  }
  string str() override  { return "ReturnStmt(" + getString(value) + ")"; }

  void prettyPrint(std::ostream& out, int indent = 0) override {
    printIndent(out, indent); out << "return";
    if (value) {
        out << " "; value->prettyPrint(out, 0);
    }
    out << ";\n";
  }

};

class ExternFunctionAST : public decafAST {
    std::string    name;
    decafAST      *rettype;
    decafStmtList *params;
public:
    ExternFunctionAST(const std::string& n,     
                      decafAST*          rtype,
                      decafStmtList*     p,
                      int                l)
        : decafAST(l), name(n), rettype(rtype), params(p) {}

    ~ExternFunctionAST() { delete rettype; delete params; }

    void Analyze() override {
      DecafType rtype = astToType(rettype);
      if (!gSym.insert(name, rtype, getLine())) {
          std::cerr << "Error: extern function '" << name << "' redeclared (line " << getLine() << ")\n";
      }
      if (params) params->Analyze(); 
    }

    void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent);
      out << "extern func " << name << "(";

      if (params) {
          bool first = true;
          for (auto *p : params->getStmts()) {
              if (!first) out << ", ";
              p->prettyPrint(out, 0);
              first = false;
          }
      }
      out << ") ";
      if (rettype) rettype->prettyPrint(out, 0);
      out << ";\n";
    }


    std::string str() override {
        return "ExternFunction(" + name + "," +
               getString(rettype) + "," + getString(params) + ")";
    }
};

class VarDefAST : public decafAST {
    std::string name;
    decafAST   *type;
public:
    VarDefAST(const std::string& n, decafAST* t, int l)
        : decafAST(l), name(n), type(t) {}
    ~VarDefAST() { delete type; }
    
    void Analyze() override {
        DecafType dtype = astToType(type);

        if (gSym.lookup(name)) { 
            std::cerr << "Warning: redefining previously defined identifier: "
                      << name << '\n';
            gSym.overwrite(name, dtype, getLine());
        } else {
            gSym.insert(name, dtype, getLine());
        }

        std::cerr << "defined variable: " << name
                  << ", with type: "  << typeToString(dtype)
                  << ", on line number: " << getLine() << '\n';
    }

    


    std::string str() override {
        return "VarDef(" + name + "," + getString(type) + ")";
    }
};

class ForStmtAST : public decafAST {
    decafAST *init;
    decafAST *cond;
    decafAST *incr;
    decafAST *body;
public:
     ForStmtAST(decafAST *i, decafAST *c, decafAST *inc, decafAST *b, int l)
        : decafAST(l), init(i), cond(c), incr(inc), body(b) {}
    ~ForStmtAST() {
        if (init) delete init;
        if (cond) delete cond;
        if (incr) delete incr;
        if (body) delete body;
    }
    void Analyze() override {
    gSym.push();
    if (init) init->Analyze();
    if (cond) cond->Analyze();
    if (incr) incr->Analyze();
    if (body) body->Analyze();
    gSym.pop();
    } 
    string str() override  {
        return "ForStmt(" + getString(init) + "," + getString(cond) + "," + getString(incr) + "," + getString(body) + ")";
    }
    void prettyPrint(std::ostream& out, int indent = 0) override {
      printIndent(out, indent); out << "for (";
      if (init) init->prettyPrint(out, 0); out << "; ";
      if (cond) cond->prettyPrint(out, 0); out << "; ";
      if (incr) incr->prettyPrint(out, 0); out << ") ";
      if (body) body->prettyPrint(out, indent);
    }

};


#define MAKE_BINOP_CLASS(CLASSNAME, LABEL, OPSTR)                \
class CLASSNAME : public decafAST {                              \
    decafAST *LHS, *RHS;                                         \
public:                                                          \
    CLASSNAME(decafAST *lhs, decafAST *rhs, int l)               \
        : decafAST(l), LHS(lhs), RHS(rhs) {}                     \
    ~CLASSNAME() { delete LHS; delete RHS; }                     \
    std::string str() override {                                 \
        return "BinaryExpr(" LABEL "," + getString(LHS) + ","    \
             + getString(RHS) + ")";                             \
    }                                                            \
    void Analyze() override {                                    \
        if (LHS) LHS->Analyze();                                 \
        if (RHS) RHS->Analyze();                                 \
    }                                                            \
    void prettyPrint(std::ostream& out, int indent = 0) override {\
        if (LHS) LHS->prettyPrint(out, 0);                       \
        out << " " << OPSTR << " ";                              \
        if (RHS) RHS->prettyPrint(out, 0);                       \
    }                                                            \
};


MAKE_BINOP_CLASS(PlusAST,        "Plus",         "+")
MAKE_BINOP_CLASS(MinusAST,       "Minus",        "-")
MAKE_BINOP_CLASS(MultAST,        "Mult",         "*")
MAKE_BINOP_CLASS(DivAST,         "Div",          "/")
MAKE_BINOP_CLASS(ModAST,         "Mod",          "%")
MAKE_BINOP_CLASS(LeftShiftAST,   "Leftshift",    "<<")
MAKE_BINOP_CLASS(RightShiftAST,  "Rightshift",   ">>")
MAKE_BINOP_CLASS(LessThanAST,    "Lt",           "<")
MAKE_BINOP_CLASS(GreaterThanAST, "Gt",           ">")
MAKE_BINOP_CLASS(LessEqualAST,   "Leq",          "<=")
MAKE_BINOP_CLASS(GreaterEqualAST,"Geq",          ">=")
MAKE_BINOP_CLASS(EqualAST,       "Eq",           "==")
MAKE_BINOP_CLASS(NotEqualAST,    "Neq",          "!=")
MAKE_BINOP_CLASS(AndAST,         "And",          "&&")
MAKE_BINOP_CLASS(OrAST,          "Or",           "||")
