%{
#include <iostream>
#include <ostream>
#include <string>
#include <cstdlib>
#include "default-defs.h"   

int yylex(void);
int yyerror(const char *); 

// print AST?
bool printAST = true;

#include "decafast.cc"

using namespace std;

%}



%define parse.error verbose

%union{
    class decafAST *ast;
    std::string *sval;
    int ival;
    char cval;
 }



%token T_FUNC T_VAR T_EXTERN T_RETURN T_IF T_ELSE T_FOR T_WHILE
%token T_BREAK T_CONTINUE T_TRUE T_FALSE T_NULL
%token T_STRINGTYPE T_BOOLTYPE

%token T_LPAREN T_RPAREN T_SEMICOLON T_COMMA
%token T_LSB T_RSB T_DOT
%token T_ASSIGN T_PLUS T_MINUS T_MULT T_DIV T_MOD
%token T_LEFTSHIFT T_RIGHTSHIFT T_LT T_GT T_LEQ T_GEQ
%token T_EQ T_NEQ T_AND T_OR T_NOT

%token <ival> T_INTCONSTANT
%token <cval> T_CHARCONSTANT
%token <sval> T_STRINGCONSTANT

%token T_INTTYPE                
%token T_VOID             

%token T_PACKAGE
%token T_LCB
%token T_RCB
%token <sval> T_ID

%left T_OR
%left T_AND
%left T_EQ T_NEQ
%left T_LT T_GT T_LEQ T_GEQ
%left T_LEFTSHIFT T_RIGHTSHIFT
%left T_PLUS T_MINUS
%left T_MULT T_DIV T_MOD
%right UMINUS T_NOT


%type <ast> extern_list decafpackage fielddecl_list fielddecl
%type <ast> stmt_list stmt assign lvalue block expr
%type <ast> methoddecl methoddecl_list rettype
%type <ast> vardecl vardecl_list

%%

start: program

program: extern_list decafpackage
    { 
        ProgramAST *prog = new ProgramAST((decafStmtList *)$1, (PackageAST *)$2); 
		if (printAST) {
			cout << getString(prog) << endl;
		}
        delete prog;
    }

extern_list: /* extern_list can be empty */
    { decafStmtList *slist = new decafStmtList(); $$ = slist; }
    ;

decafpackage:
    T_PACKAGE T_ID T_LCB fielddecl_list methoddecl_list T_RCB {
        $$ = new PackageAST(*$2, (decafStmtList *)$4, (decafStmtList *)$5);
        delete $2;
    }
;
   

fielddecl_list:
      /* empty */ { $$ = new decafStmtList(); }
    | fielddecl_list fielddecl { ((decafStmtList *)$1)->push_back($2); $$ = $1; }
    ;

fielddecl:
      T_VAR T_ID T_INTTYPE T_SEMICOLON
        { $$ = new FieldDeclAST(*$2, new IntTypeAST()); delete $2; }
    ;

stmt
    : assign T_SEMICOLON { $$ = $1; }
    ;

assign
    : lvalue T_ASSIGN expr { $$ = new AssignAST($1, $3); }
    ;

lvalue
    : T_ID { $$ = new VariableAST(*$1); delete $1; }
    ;

stmt_list
    : /* empty */ { $$ = new decafStmtList(); }
    | stmt_list stmt { ((decafStmtList *)$1)->push_back($2); $$ = $1; }
    ;

methoddecl_list:
      /* empty */ { $$ = new decafStmtList(); }
    | methoddecl_list methoddecl { ((decafStmtList *)$1)->push_back($2); $$ = $1; }
;

methoddecl:
    T_FUNC T_ID T_LPAREN T_RPAREN rettype T_LCB vardecl_list stmt_list T_RCB {
        MethodBlockAST *mb = new MethodBlockAST((decafStmtList *)$7, (decafStmtList *)$8);
        $$ = new MethodDeclAST(*$2, new decafStmtList(), $5, mb);
        delete $2;
    }
;

rettype:
      T_VOID { $$ = new VoidTypeAST(); }
    | T_INTTYPE { $$ = new IntTypeAST(); }
;

block
    : T_LCB stmt_list T_RCB { $$ = $2; }
    ;

vardecl_list:
      /* empty */ { $$ = new decafStmtList(); }
    | vardecl_list vardecl { ((decafStmtList *)$1)->push_back($2); $$ = $1; }
;

vardecl:
    T_VAR T_ID T_INTTYPE T_SEMICOLON {
        $$ = new VarDeclAST(*$2, new IntTypeAST()); delete $2;
    }
;


expr:
    T_INTCONSTANT                         { $$ = new IntConstantAST($1); }
  | T_ID                                  { $$ = new VariableAST(*$1); delete $1; }
  | expr T_PLUS expr                      { $$ = new PlusAST($1, $3); }
  | expr T_MINUS expr                     { $$ = new MinusAST($1, $3); }
  | expr T_MULT expr                      { $$ = new MultAST($1, $3); }
  | expr T_DIV expr                       { $$ = new DivAST($1, $3); }
  | expr T_MOD expr                       { $$ = new ModAST($1, $3); }
  | expr T_LEFTSHIFT expr                 { $$ = new LeftShiftAST($1, $3); }
  | expr T_RIGHTSHIFT expr                { $$ = new RightShiftAST($1, $3); }
  | expr T_LT expr                        { $$ = new LessThanAST($1, $3); }
  | expr T_GT expr                        { $$ = new GreaterThanAST($1, $3); }
  | expr T_LEQ expr                       { $$ = new LessEqualAST($1, $3); }
  | expr T_GEQ expr                       { $$ = new GreaterEqualAST($1, $3); }
  | expr T_EQ expr                        { $$ = new EqualAST($1, $3); }
  | expr T_NEQ expr                       { $$ = new NotEqualAST($1, $3); }
  | expr T_AND expr                       { $$ = new AndAST($1, $3); }
  | expr T_OR expr                        { $$ = new OrAST($1, $3); }
  | T_MINUS expr %prec UMINUS             { $$ = new UnaryMinusAST($2); }
  | T_NOT expr                            { $$ = new NotAST($2); }
  | T_LPAREN expr T_RPAREN                { $$ = $2; }
  | T_CHARCONSTANT                        { $$ = new IntConstantAST((int)$1); }

;

%%

int main() {
  // parse the input and create the abstract syntax tree
  int retval = yyparse();
  return(retval >= 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}

