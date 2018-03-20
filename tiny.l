/****************************************************/
/* File: tiny.l                                     */
/* Lex specification for TINY                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
static int yylex(void);
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t\r]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"         {return WHILE;}
"void"          {return VOID;}
"int"           {return INT;}
"return"        {return RETURN;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"!="            {return NEQ;}
"<"             {return LT;}
"<="            {return LTEQ;}
">"             {return GT;}
">="            {return GTEQ;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
";"             {return SEMI;}
","             {return COMMA;}
"["             {return LBRACK;}
"]"             {return RBRACK;}
"{"             {return LBRACE;}
"}"             {return RBRACE;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"             { char c; int isCommentEnd=0;
                  do
                  { c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
                    if (isCommentEnd == 1)
                    {
                      if (c == '/') break;
                      isCommentEnd=0;
                    }
                    if (c == '*') isCommentEnd=1;
                  } while (1);
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

