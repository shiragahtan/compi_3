%{

#include "nodes.hpp"
#include "output.hpp"
#include <memory>
#include <iostream>
#include <stdlib.h>

// Bison declarations
extern int yylineno;
extern int yylex();

void yyerror(const char*);

std::shared_ptr<ast::Node> program;

using namespace std;
using namespace ast;
using std::dynamic_pointer_cast;

#define YYERROR_VERBOSE 1
#define YYDEBUG 1

%}

%token ID VOID BOOL BYTE INT STRING
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%token RETURN IF ELSE WHILE BREAK CONTINUE
%token ASSIGN
%token ADD SUB MUL DIV
%token LT LE GT GE EQ NE
%token SC COMMA
%token AND OR NOT TRUE FALSE NUM NUM_B

%right ASSIGN

%left OR
%left AND
%left NE
%left EQ
%left LT LE GT GE
%left ADD SUB
%left MUL DIV // in order for *,/ operations to have higher precedence then +,-
%right CAST
%right NOT

%left RBRACE LBRACE   // match { , }
%left LPAREN RPAREN  //left and right parenthesis

%start Program

%%

Program:
    Funcs {
        program = $1;
    }
;

Funcs:
    /*epsilon*/ {
        $$ = std::make_shared<ast::Funcs>();
    }
    | FuncDecl Funcs {
        auto funcs = dynamic_pointer_cast<ast::Funcs>($2);
        funcs->push_front(dynamic_pointer_cast<ast::FuncDecl>($1));
        $$ = funcs;
    }
;

FuncDecl:
    RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE {
        $$ = std::make_shared<ast::FuncDecl>(
            dynamic_pointer_cast<ast::ID>($2),
            dynamic_pointer_cast<ast::Type>($1),
            dynamic_pointer_cast<ast::Formals>($4),
            dynamic_pointer_cast<ast::Statements>($7)
        );
    }
;

RetType:
    Type {
        $$ = $1;
    }
    | VOID {
        $$ = std::make_shared<ast::Type>(ast::BuiltInType::VOID);
    }
;

Formals:
    /*epsilon*/ {
        $$ = std::make_shared<ast::Formals>();
    }
    | FormalsList {
        $$ = $1;
    }
;

FormalsList:
    FormalDecl {
        auto formals = make_shared<ast::Formals>();
        formals->push_front(dynamic_pointer_cast<ast::Formal>($1));
        $$ = formals;
    }
    | FormalDecl COMMA FormalsList {
        auto formals = dynamic_pointer_cast<ast::Formals>($3);
        formals->push_front(dynamic_pointer_cast<ast::Formal>($1));
        $$ = formals;
    }
;

FormalDecl:
    Type ID {
        $$ = std::make_shared<ast::Formal>(
            dynamic_pointer_cast<ast::ID>($2),
            dynamic_pointer_cast<ast::Type>($1)
        );
    }
;

Statements:
    Statement {
        auto statements = std::make_shared<ast::Statements>();
        statements->push_back(dynamic_pointer_cast<ast::Statement>($1));
        $$ = statements;
    }
    | Statements Statement {
        auto statements = dynamic_pointer_cast<ast::Statements>($1);
        statements->push_back(dynamic_pointer_cast<ast::Statement>($2));
        $$ = statements;
    }
;

Statement:
    LBRACE Statements RBRACE { $$ = $2; }
    | Type ID SC { $$ = std::make_shared<ast::VarDecl>(
                        dynamic_pointer_cast<ast::ID>($2),
                        dynamic_pointer_cast<ast::Type>($1)); }
    | Type ID ASSIGN Exp SC { $$ = std::make_shared<ast::VarDecl>(
                        dynamic_pointer_cast<ast::ID>($2),
                        dynamic_pointer_cast<ast::Type>($1),
                        dynamic_pointer_cast<ast::Exp>($4)); }
    | ID ASSIGN Exp SC { std::shared_ptr<ast::ID> id = dynamic_pointer_cast<ast::ID>($1);
                          std::shared_ptr<ast::Exp> exp = dynamic_pointer_cast<ast::Exp>($3);
                          $$ = std::make_shared<ast::Assign>(id, exp); }
    | Call SC { $$ = $1; }
    | RETURN SC { $$ = std::make_shared<ast::Return>(); }
    | RETURN Exp SC { $$ = std::make_shared<ast::Return>(
                        dynamic_pointer_cast<ast::Exp>($2)); }
    | IfStatement
    | WHILE LPAREN Exp RPAREN Statement {
          $$ = std::make_shared<ast::While>(
                        dynamic_pointer_cast<ast::Exp>($3),
                        dynamic_pointer_cast<ast::Statement>($5));
      }
    | BREAK SC { $$ = std::make_shared<ast::Break>(); }
    | CONTINUE SC { $$ = std::make_shared<ast::Continue>(); }
;

IfStatement:
    IfWithoutElse
    | IfWithElse
;

IfWithoutElse:
    IF LPAREN Exp RPAREN Statement %prec LOWER_THAN_ELSE{
        $$ = std::make_shared<ast::If>(
            dynamic_pointer_cast<ast::Exp>($3),
            dynamic_pointer_cast<ast::Statement>($5)
        );
    }
;

IfWithElse:
    IF LPAREN Exp RPAREN Statement ELSE Statement {
        $$ = std::make_shared<ast::If>(
            dynamic_pointer_cast<ast::Exp>($3),
            dynamic_pointer_cast<ast::Statement>($5),
            dynamic_pointer_cast<ast::Statement>($7)
        );
    }
;

Call:
    ID LPAREN ExpList RPAREN {$$ = std::make_shared<ast::Call>(dynamic_pointer_cast<ast::ID>($1), dynamic_pointer_cast<ast::ExpList>($3));}
    | ID LPAREN RPAREN {$$ = std::make_shared<ast::Call>(dynamic_pointer_cast<ast::ID>($1));}
;

ExpList:
    Exp {$$ = std::make_shared<ast::ExpList>(dynamic_pointer_cast<ast::Exp>($1));}
    | Exp COMMA ExpList {
            auto expList = dynamic_pointer_cast<ast::ExpList>($3);
            expList->push_front(dynamic_pointer_cast<ast::Exp>($1));
            $$ = expList;
        }
;

Type:
    INT     {$$ = std::make_shared<ast::Type>(ast::BuiltInType::INT);}
    | BYTE  {$$ = std::make_shared<ast::Type>(ast::BuiltInType::BYTE);}
    | BOOL  {$$ = std::make_shared<ast::Type>(ast::BuiltInType::BOOL);}
;

Exp:
    LPAREN Exp RPAREN { $$= $2;}
    | Exp ADD Exp   {$$ = std::make_shared<ast::BinOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::BinOpType::ADD);}
    | Exp SUB Exp   {$$ = std::make_shared<ast::BinOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::BinOpType::SUB);}
    | Exp MUL Exp   {$$ = std::make_shared<ast::BinOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::BinOpType::MUL);}
    | Exp DIV Exp   {$$ = std::make_shared<ast::BinOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::BinOpType::DIV);}
    | ID            {$$ = $1;}
    | Call          {$$ = $1;}
    | NUM           {$$ = $1;}
    | NUM_B         {$$ = $1;}
    | STRING        {$$ = $1;}
    | TRUE          {$$ = std::make_shared<ast::Bool>(true);}
    | FALSE         {$$ = std::make_shared<ast::Bool>(false);}
    | NOT Exp       {$$ = std::make_shared<ast::Not>(dynamic_pointer_cast<ast::Exp>($2));}
    | Exp AND Exp   {$$ = std::make_shared<ast::And>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3));}
    | Exp OR Exp    {$$ = std::make_shared<ast::Or>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3));}
    | Exp EQ Exp    {$$ = std::make_shared<ast::RelOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::RelOpType::EQ);}
    | Exp NE Exp    {$$ = std::make_shared<ast::RelOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::RelOpType::NE);}
    | Exp LT Exp    {$$ = std::make_shared<ast::RelOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::RelOpType::LT);}
    | Exp GT Exp    {$$ = std::make_shared<ast::RelOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::RelOpType::GT);}
    | Exp LE Exp    {$$ = std::make_shared<ast::RelOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::RelOpType::LE);}
    | Exp GE Exp    {$$ = std::make_shared<ast::RelOp>(dynamic_pointer_cast<ast::Exp>($1), dynamic_pointer_cast<ast::Exp>($3), ast::RelOpType::GE);}
    | LPAREN Type RPAREN Exp %prec CAST {std::shared_ptr<ast::Exp> exp = dynamic_pointer_cast<ast::Exp>($4);
                                         std::shared_ptr<ast::Type> type = dynamic_pointer_cast<ast::Type>($2);
                                         $$ = std::make_shared<ast::Cast>(exp, type);}
;



%%

void yyerror(const char *s) {
    output::errorSyn(yylineno);
}