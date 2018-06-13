/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

static char *funcName;
static int preserveLastScope = FALSE;

/* counter for variable memory locations */

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode *t,
                     void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *))
{
  if (t != NULL)
  {
    preProc(t);
    {
      int i;
      for (i = 0; i < MAXCHILDREN; i++)
        traverse(t->child[i], preProc, postProc);
    }
    postProc(t);
    traverse(t->sibling, preProc, postProc);
  }
}

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode *t)
{
  if (t == NULL)
    return;
  else
    return;
}

static void symbolError(TreeNode *t, char *message)
{
  fprintf(listing, "Symbol error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

char *format_str(char *a, char *b)
{
  if (b != NULL)
  {
    char *c = malloc(strlen(a) + strlen(b) + 1);
    if (c != NULL)
    {
      strcpy(c, a);
      strcat(c, b);
      return c;
    }
  }
  return NULL;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      if (preserveLastScope)
      {
        preserveLastScope = FALSE;
      }
      else
      {
        Scope scope = sc_create(funcName);
        sc_push(scope);
      }
      t->attr.scope = sc_top();
      break;
    default:
      break;
    }
    break;
  case ExpK:
    switch (t->kind.exp)
    {
    case IdK:
      if (st_lookup_kind(t->attr.name, VarK) == -1)
        /* not yet in table, error */
        symbolError(t, format_str("undeclared variable: ", t->attr.name));
      else
        /* already in table, so ignore location,
             add line number of use only */
        st_add_lineno(t->attr.name, t->lineno);
      break;
    case ArrIdK:
      if (st_lookup_kind(t->attr.name, ArrVarK) == -1)
        /* not yet in table, error */
        symbolError(t, format_str("undeclared array: ", t->attr.name));
      else
        /* already in table, so ignore location,
             add line number of use only */
        st_add_lineno(t->attr.name, t->lineno);
      break;
    case CallK:
      if (st_lookup_kind(t->attr.name, FuncK) == -1)
        /* not yet in table, error */
        symbolError(t, format_str("undeclared function: ", t->attr.name));
      else
        /* already in table, so ignore location,
             add line number of use only */
        st_add_lineno(t->attr.name, t->lineno);
      break;
    default:
      break;
    }
    break;
  case DeclK:
    switch (t->kind.decl)
    {
    case FuncK:
      funcName = t->attr.name;
      if (st_lookup_top(funcName) >= 0)
      {
        /* already in table, so it's an error */
        symbolError(t, format_str("function already declared: ", t->attr.name));
        break;
      }
      st_insert(funcName, t->lineno, addLocation(), t);
      sc_push(sc_create(funcName));
      preserveLastScope = TRUE;
      switch (t->child[0]->attr.type)
      {
      case INT:
        t->type = Integer;
        break;
      case VOID:
      default:
        t->type = Void;
        break;
      }
      break;
    case VarK:
    case ArrVarK:
    {
      char *name;

      if (t->child[0]->attr.type == VOID)
      {
        symbolError(t, format_str("variable should have non-void type: ", t->attr.arr.name));
        break;
      }

      if (t->kind.decl == VarK)
      {
        name = t->attr.name;
        t->type = Integer;
      }
      else
      {
        name = t->attr.arr.name;
        t->type = IntegerArray;
      }

      if (st_lookup_top(name) < 0 && st_global_lookup_top(name) < 0)
        st_insert(name, t->lineno, addLocation(), t);
      else
        symbolError(t, format_str("symbol already declared for current scope: ", name));
    }
    break;
    default:
      break;
    }
    break;
  case ParamK:
    if (t->child[0]->attr.type == VOID)
      symbolError(t->child[0], format_str("void type parameter is not allowed: ", t->attr.name));
    if (st_lookup(t->attr.name) == -1)
    {
      st_insert(t->attr.name, t->lineno, addLocation(), t);
      if (t->kind.param == NonArrParamK)
        t->type = Integer;
    }
    else
      symbolError(t, format_str("symbol already declared for current scope: ", t->attr.name));
    break;
  default:
    break;
  }
}

static void afterInsertNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      sc_pop();
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *syntaxTree)
{
  globalScope = sc_create(NULL);
  sc_push(globalScope);
  traverse(syntaxTree, insertNode, afterInsertNode);
  if (st_lookup_top("main") == -1)
  {
    fprintf(listing, "Type error: missing main function\n");
    Error = TRUE;
  }
  sc_pop();
  if (TraceAnalyze)
  {
    fprintf(listing, "\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode *t, char *message)
{
  fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

static void beforeCheckNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case DeclK:
    switch (t->kind.decl)
    {
    case FuncK:
      funcName = t->attr.name;
      break;
    default:
      break;
    }
    break;
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      sc_push(t->attr.scope);
      break;
    default:
      break;
    }
  default:
    break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      sc_pop();
      break;
    case IterK:
      if (t->child[0]->type == Void)
        /* while test should be void function call */
        typeError(t->child[0], "while test has void value");
      break;
    case RetK:
    {
      BucketList bucket = st_bucket(funcName);
      if (bucket == NULL)
        break;
      const TreeNode *funcDecl = bucket->treeNode;
      const ExpType funcType = funcDecl->type;
      const TreeNode *expr = t->child[0];

      if (funcType == Void &&
          (expr != NULL && expr->type != Void))
      {
        typeError(t, "expected no return value");
        //ValueReturned = TRUE;
      }
      else if (funcType == Integer &&
               (expr == NULL || expr->type == Void))
      {
        typeError(t, "expected return value");
      }
    }
    break;
    default:
      break;
    }
    break;
  case ExpK:
    switch (t->kind.exp)
    {
    case AssignK:
      if (t->child[0]->type == IntegerArray)
        /* no value can be assigned to array variable */
        typeError(t->child[0], "assignment to array variable");
      else if (t->child[1]->type == Void)
        /* r-value cannot have void type */
        typeError(t->child[0], "assignment of void value");
      else
        t->type = t->child[0]->type;
      break;
    case OpK:
    {
      ExpType leftType, rightType;
      TokenType op;

      leftType = t->child[0]->type;
      rightType = t->child[1]->type;
      op = t->attr.op;

      if (leftType == Void ||
          rightType == Void)
        typeError(t, "two operands should have non-void type");
      else if (leftType == IntegerArray &&
               rightType == IntegerArray)
        typeError(t, "not both of operands can be array");
      else if (op == MINUS &&
               leftType == Integer &&
               rightType == IntegerArray)
        typeError(t, "invalid operands to binary expression");
      else if ((op == TIMES || op == OVER) &&
               (leftType == IntegerArray ||
                rightType == IntegerArray))
        typeError(t, "invalid operands to binary expression");
      else
      {
        t->type = Integer;
      }
    }
    break;
    case ConstK:
      t->type = Integer;
      break;
    case IdK:
    case ArrIdK:
    {
      char *symbolName = t->attr.name;
      const BucketList bucket =
          st_bucket(symbolName);
      TreeNode *symbolDecl = NULL;

      if (bucket == NULL)
        break;
      symbolDecl = bucket->treeNode;

      if (t->kind.exp == ArrIdK)
      {
        if (symbolDecl->kind.decl != ArrVarK &&
            symbolDecl->kind.param != ArrParamK)
          typeError(t, "expected array symbol");
        else if (t->child[0]->type != Integer)
          typeError(t, "index expression should have integer type");
        else
          t->type = Integer;
      }
      else
      {
        t->type = symbolDecl->type;
      }
    }
    break;
    case CallK:
    {
      char *callingFuncName = t->attr.name;
      BucketList bucket = st_bucket_kind(callingFuncName, FuncK);
      if (bucket == NULL)
        break;
      const TreeNode *funcDecl = bucket->treeNode;
      TreeNode *arg;
      TreeNode *param;

      if (funcDecl == NULL)
        break;

      arg = t->child[0];
      param = funcDecl->child[1];

      while (arg != NULL)
      {
        if (param == NULL)
          /* the number of arguments does not match to
                 that of parameters */
          typeError(arg, "the number of parameters is wrong");
        /*else if (arg->type == IntegerArray &&
                  param->type != IntegerArray)
                typeError(arg,"expected non-array value");
              else if (arg->type == Integer &&
                  param->type == IntegerArray)
                typeError(arg,"expected array value");*/
        else if (arg->type == Void)
          typeError(arg, "void value cannot be passed as an argument");
        else
        { // no problem!
          arg = arg->sibling;
          param = param->sibling;
          continue;
        }
        /* any problem */
        break;
      }

      if (arg == NULL && param != NULL)
        /* the number of arguments does not match to
               that of parameters */
        typeError(t->child[0], "the number of parameters is wrong");

      t->type = funcDecl->type;
    }
    break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree)
{
  sc_push(globalScope);
  traverse(syntaxTree, beforeCheckNode, checkNode);
  sc_pop();
}