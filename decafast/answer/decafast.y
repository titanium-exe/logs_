%{
#include <iostream>
#include <ostream>
#include <string>
#include <cstdlib>
#include "default-defs.h"   


int yylex(void);
int yyerror(const char *); 


bool printAST = true;

#include "decafast.cc"

using namespace std;

%}

%code requires {
    
    #include <list>
    #include <string>
}

%define parse.error verbose

%union{
    class decafAST *ast;
    std::string    *sval;
    std::list<std::string> *slst;   /* NEW – a list of ids */
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
%nonassoc LOWER_THAN_ELSE   
%nonassoc T_ELSE

%type <ast> extern_list extern_decl
%type <ast> extern_typelist extern_typelist_opt extern_type
%type <ast> decafpackage fielddecl_list fielddecl
%type <ast> methoddecl methoddecl_list rettype
%type <ast> stmt_list stmt assign lvalue block expr
%type <ast> vardecl vardecl_list
%type <ast> param_list param param_list_opt
%type <ast> methodcall arg_list arg_list_opt
%type <slst> id_list 



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

decafpackage:
    T_PACKAGE T_ID T_LCB fielddecl_list methoddecl_list T_RCB {
        $$ = new PackageAST(*$2, (decafStmtList *)$4, (decafStmtList *)$5);
        delete $2;
    }
;
   

fielddecl_list
    : /* empty */                       { $$ = new decafStmtList(); }
    | fielddecl_list fielddecl          { ((decafStmtList*)$1)->merge((decafStmtList*)$2); delete $2; $$ = $1; }
    ;



id_list
    : T_ID                     { $$ = new std::list<std::string>(); $$->push_back(*$1); delete $1; }
    | id_list T_COMMA T_ID     { $$ = $1;                          $$->push_back(*$3); delete $3; }
    ;


fielddecl
    : T_VAR id_list T_INTTYPE T_SEMICOLON
      {
          decafStmtList *lst = new decafStmtList();
          for (auto &name : *$2)
              lst->push_back(new FieldDeclAST(name, new IntTypeAST()));
          delete $2;
          $$ = lst;                /* return the whole list */
      }
    ;

stmt:
      assign T_SEMICOLON                 { $$ = $1; }
    | methodcall T_SEMICOLON             { $$ = $1; }
    | block                              { $$ = $1; }
    | T_WHILE T_LPAREN expr T_RPAREN stmt
        { $$ = new WhileStmtAST($3, $5); }
    | T_IF T_LPAREN expr T_RPAREN stmt            %prec LOWER_THAN_ELSE
        { $$ = new IfStmtAST($3, $5, nullptr); }
    | T_IF T_LPAREN expr T_RPAREN stmt T_ELSE stmt
        { $$ = new IfStmtAST($3, $5, $7); }
    | T_RETURN T_SEMICOLON
        { $$ = new ReturnStmtAST(nullptr); }
    | T_RETURN T_LPAREN expr T_RPAREN T_SEMICOLON
        { $$ = new ReturnStmtAST($3); }
    | T_BREAK T_SEMICOLON                       
        { $$ = new BreakStmtAST(); }
    | T_CONTINUE T_SEMICOLON                     
        { $$ = new ContinueStmtAST(); }
    ;


methodcall
    : T_ID T_LPAREN arg_list_opt T_RPAREN
        {
            $$ = new MethodCallAST(*$1,(decafStmtList*)$3);
            delete $1;
        }
;

arg_list_opt
    : /* ε */                         { $$ = new decafStmtList(); }
    | arg_list                        { $$ = $1; }
;

arg_list
    : expr                            { auto l=new decafStmtList(); l->push_back($1); $$ = l; }
    | arg_list T_COMMA expr           { ((decafStmtList*)$1)->push_back($3); $$ = $1; }
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

methoddecl
    : T_FUNC T_ID T_LPAREN param_list_opt T_RPAREN rettype
      T_LCB vardecl_list stmt_list T_RCB
      {
          MethodBlockAST *mb =
              new MethodBlockAST((decafStmtList *)$8, (decafStmtList *)$9);
          $$ = new MethodDeclAST(*$2, (decafStmtList *)$4, $6, mb);
          delete $2;
      }
    ;

rettype:
      T_VOID { $$ = new VoidTypeAST(); }
    | T_INTTYPE { $$ = new IntTypeAST(); }
;

block:
    T_LCB stmt_list T_RCB { $$ = new BlockAST(new decafStmtList(), (decafStmtList *)$2); }
;


vardecl_list
    : /* empty */                       { $$ = new decafStmtList(); }
    | vardecl_list vardecl              { ((decafStmtList*)$1)->merge((decafStmtList*)$2); delete $2; $$ = $1; }
;

vardecl
    : T_VAR id_list T_INTTYPE T_SEMICOLON
      {
          decafStmtList *lst = new decafStmtList();
          for (auto &name : *$2)
              lst->push_back(new VarDeclAST(name, new IntTypeAST()));
          delete $2;
          $$ = lst;
      }
    ;

extern_list
    : /* empty */                           { $$ = new decafStmtList(); }
    | extern_list extern_decl               { ((decafStmtList *)$1)->push_back($2); $$ = $1; }
;

extern_decl
    : T_EXTERN T_FUNC T_ID
      T_LPAREN extern_typelist_opt T_RPAREN
      rettype T_SEMICOLON
        {
            $$ = new ExternFunctionAST(*$3, $7, (decafStmtList *)$5);
            delete $3;
        }
;

extern_typelist_opt
    : /* empty */                     { $$ = new decafStmtList(); }
    | extern_typelist                 { $$ = $1; }
;

extern_typelist
    : extern_type                     {
          auto lst = new decafStmtList();
          lst->push_back($1); $$ = lst;
      }
    | extern_typelist T_COMMA extern_type {
          ((decafStmtList *)$1)->push_back($3); $$ = $1;
      }
;


extern_type
    : T_STRINGTYPE                    { $$ = new StringTypeAST(); }
    | T_INTTYPE                       { $$ = new IntTypeAST(); }
    | T_BOOLTYPE                      { $$ = new BoolTypeAST(); }
;

param_list_opt
    : param_list                     { $$ = $1; }
    | /* ε */                        { $$ = new decafStmtList(); }
    ;

param_list
    : param                          { auto lst = new decafStmtList();
                                       lst->push_back($1); $$ = lst; }
    | param_list T_COMMA param       { ((decafStmtList*)$1)->push_back($3);
                                       $$ = $1; }
    ;

param
    : T_ID T_INTTYPE                 { $$ = new VarDefAST(*$1,
                                       new IntTypeAST()); delete $1; }
    | T_ID T_BOOLTYPE                { $$ = new VarDefAST(*$1,
                                       new BoolTypeAST()); delete $1; }
    | T_ID T_STRINGTYPE              { $$ = new VarDefAST(*$1,
                                       new StringTypeAST()); delete $1; }
    ;




expr:
   methodcall                      { $$ = $1; }    
  | T_INTCONSTANT                         { $$ = new IntConstantAST($1); }
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
  | T_TRUE                                { $$ = new BoolConstantAST(true); }
  | T_FALSE                               { $$ = new BoolConstantAST(false); }


;

%%

int main() {
  // parse the input and create the abstract syntax tree
  int retval = yyparse();
  return(retval >= 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}

