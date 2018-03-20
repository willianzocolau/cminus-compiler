/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static char * savedName2; /* for use in assignments */
static int savedNumber;
static TokenType savedOp;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);
static int yyerror(char*);

%}

%token IF ELSE WHILE VOID INT RETURN 
%token ID NUM 
%token ASSIGN LT LTEQ GT GTEQ EQ NEQ PLUS MINUS TIMES OVER 
%token LPAREN RPAREN SEMI COMMA LBRACK RBRACK LBRACE RBRACE 
%token ERROR 

%% /* Grammar for CMINUS */

programa    : decl_lista
                 { savedTree = $1;} 
			;

decl_lista	: decl_lista decl
				{
					YYSTYPE t = $1;
					if (t != NULL)
					{
						while (t->sibling != NULL)
							t = t->sibling;
						t->sibling = $2;
						$$ = $1; 
					} else $$ = $2;
                }
			| decl { $$ = $1; }
			;

decl		: var_decl { $$ = $1; }
			| fun_decl { $$ = $1; }
			;
			
salva_nome  : ID
				{
				savedName = copyString(tokenString);
				savedLineNo = lineno;
				}
            ;
			
salva_num 	: NUM
				{
					savedNumber = atoi(tokenString);
					savedLineNo = lineno;
                }
            ;

var_decl	: tipo_espec salva_nome SEMI
				{
					$$ = newDeclNode(VarK);
					$$->child[0] = $1; /* type */
					$$->lineno = lineno;
					$$->attr.name = savedName;
                }
			| tipo_espec salva_nome LBRACK salva_num RBRACK SEMI
				{
					$$ = newDeclNode(ArrVarK);
					$$->child[0] = $1; /* type */
					$$->lineno = lineno;
					$$->attr.arr.name = savedName;
					$$->attr.arr.size = savedNumber;
                }
			;
			
tipo_espec	: INT
				{
					$$ = newTypeNode(TypeNameK);
					$$->attr.type = INT;
                }
			| VOID
				{
					$$ = newTypeNode(TypeNameK);
					$$->attr.type = VOID;
                }
			;
			
fun_decl	: tipo_espec salva_nome 
				{
					$$ = newDeclNode(FuncK);
					$$->lineno = lineno;
					$$->attr.name = savedName;
				}
			  LPAREN params RPAREN comp_decl
				{
					$$ = $3;
					$$->child[0] = $1; /* type */
					$$->child[1] = $5; /* parameters */
					$$->child[2] = $7; /* body */
				}
			;
			
params		: param_lista { $$ = $1; }
			| VOID
				{
					$$ = newTypeNode(TypeNameK);
					$$->attr.type = VOID;
                }
			;
			
param_lista : param_lista COMMA param
				{
					YYSTYPE t = $1;
					if (t != NULL)
					{
						while (t->sibling != NULL)
							t = t->sibling;
						t->sibling = $3;
						$$ = $1;
					}
                    else $$ = $3; 
                }
			| param { $$ = $1; }
			;
			
param		: tipo_espec salva_nome
				{
					$$ = newParamNode(NonArrParamK);
					$$->child[0] = $1;
					$$->attr.name = savedName;
                }
			| tipo_espec salva_nome LBRACK RBRACK
				{
					$$ = newParamNode(ArrParamK);
					$$->child[0] = $1;
					$$->attr.name = savedName;
                }
			;
			
comp_decl	: LBRACE local_decl stmt_lista RBRACE
				{
					$$ = newStmtNode(CompK);
					$$->child[0] = $2; /* local variable declarations */
					$$->child[1] = $3; /* statements */
                }
			;

local_decl	: local_decl var_decl
				{
					YYSTYPE t = $1;
					if (t != NULL)
					{
						while (t->sibling != NULL)
							t = t->sibling;
						t->sibling = $2;
						$$ = $1;
					}
                    else $$ = $2;
				}
			| var_decl { $$ = $1; }
			| { $$ = NULL; }
			;
			
stmt_lista	: stmt_lista stmt
				{
					YYSTYPE t = $1;
					if (t != NULL)
					{
						while (t->sibling != NULL)
							t = t->sibling;
						t->sibling = $2;
						$$ = $1;
					}
					else $$ = $2;
				}
			| stmt { $$ = $1; }
			| { $$ = NULL; }
			;
			
stmt		: expr_decl { $$ = $1; }
			| comp_decl { $$ = $1; }
			| selec_decl { $$ = $1; }
			| iter_decl { $$ = $1; }
			| ret_decl { $$ = $1; }
			;
			
expr_decl	: exp SEMI { $$ = $1; }
			| SEMI { $$ = NULL; }
			;

selec_decl	: IF LPAREN exp RPAREN stmt
				{
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = NULL;
				}
			| IF LPAREN exp RPAREN stmt ELSE stmt
				{
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = $7;
				}
			;
			
iter_decl	: WHILE LPAREN exp RPAREN stmt
				{
					$$ = newStmtNode(IterK);
					$$->child[0] = $3;
					$$->child[1] = $5;
				}
			;
			
ret_decl	: RETURN SEMI
				{
					$$ = newStmtNode(RetK);
					$$->child[0] = NULL;
				}
			| RETURN exp SEMI
				{
					$$ = newStmtNode(RetK);
					$$->child[0] = $2;
				}
			;
			
exp			: var ASSIGN exp
				{
					$$ = newExpNode(AssignK);
					$$->child[0] = $1;
					$$->child[1] = $3;
				}
			| simple_exp { $$ = $1; }
			;
			
var			: salva_nome
				{
					savedName2 = savedName;
				}
			 LBRACK exp RBRACK
				{
					$$ = newExpNode(ArrIdK);
					$$->attr.name = savedName2;
					$$->child[0] = $4;
				}
			| salva_nome
				{
					$$ = newExpNode(IdK);
					$$->attr.name = savedName;
				}
			;
			
simple_exp	: soma_exp relacional soma_exp
				{
					$$ = newExpNode(OpK);
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = savedOp;
				}
			| soma_exp  { $$ = $1; }
			;
			
relacional	: LTEQ { savedOp = LTEQ; }
			| LT { savedOp = LT; }
			| GT { savedOp = GT; }
			| GTEQ { savedOp = GTEQ; }
			| EQ { savedOp = EQ; }
			| NEQ { savedOp = NEQ; }
			;
			
soma_exp	: soma_exp soma termo
				{
					$$ = newExpNode(OpK);
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = savedOp;
				}
			| termo { $$ = $1; }
			;
			
soma		: PLUS { savedOp = PLUS; }
			| MINUS { savedOp = MINUS; }
			;
			
termo		: termo mult fator
				{
					$$ = newExpNode(OpK);
					$$->child[0] = $1;
					$$->child[1] = $3;
					$$->attr.op = savedOp;
				}
			| fator { $$ = $1; }
			;
			
mult		: TIMES { savedOp = TIMES; }
			| OVER { savedOp = OVER; }
			;
			
fator		: LPAREN exp RPAREN { $$ = $2; }
			| var { $$ = $1; }
			| ativacao { $$ = $1; }
			| NUM
				{
					$$ = newExpNode(ConstK);
					$$->attr.val = atoi(tokenString);
				}
			;
			
ativacao	: salva_nome
				{
					$$ = newExpNode(CallK);
					$$->attr.name = savedName;
				}
			  LPAREN args RPAREN
				{
					$$ = $2;
					$$->child[0] = $4;
				}
			;
			
args        : arg_lista { $$ = $1; }
            | { $$ = NULL; }
            ;

arg_lista	: arg_lista COMMA exp
				{
					YYSTYPE t = $1;
					if (t != NULL)
					{
						while (t->sibling != NULL)
							t = t->sibling;
						t->sibling = $3;
						$$ = $1;
					}
                    else $$ = $3;
				}
			| exp { $$ = $1; }
			;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

