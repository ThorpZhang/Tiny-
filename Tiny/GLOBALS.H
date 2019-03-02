/****************************************************/
/* File: globals.h                                  */
/* Global types and vars for TINY compiler          */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 18

typedef enum 
    /* book-keeping tokens */
   {ENDFILE,ERROR,
    /* reserved words */
    IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,
	/* Tiny+ 新增保留字 */
	TK_TURE,TK_FALSE,TK_OR,TK_AND,TK_NOT,TK_INT,TK_BOOL,TK_STRING,TK_WHILE,TK_DO,
    /* multicharacter tokens */
    ID,NUM,STRING,//新增类型STRING
    /* special symbols */
    ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI,
	/*Tiny+ 新增符号 */
	TK_GER, TK_LEQ, TK_GEQ, TK_COMMA, TK_QUEOT,
	UNDEFINED
   } TokenType;

extern FILE* source; /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum {StmtK,ExpK} NodeKind;
typedef enum {IfK,RepeatK,AssignK,ReadK,WriteK,WHILEK} StmtKind;//新增 WHILEK
typedef enum {OpK,ConstK,IdK,BoolK,StringK} ExpKind; //新增BOOLkey 和 STRINGkey

/* ExpType is used for type checking */
typedef enum {Void,Integer,Boolean,STRing} ExpType;//新增STRING

#define MAXCHILDREN 3

typedef struct treeNode
   { struct treeNode * child[MAXCHILDREN];
     struct treeNode * sibling;
     int lineno;
     NodeKind nodekind;
     union { StmtKind stmt; ExpKind exp;} kind;
     union { TokenType op;
             int val;
             char * name; } attr;
     ExpType type; /* for type checking of exps */
   } TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error; 
#endif

/*
//TINY+ 新增的结构

typedef struct {
	TokenType kind;

	int val;

	char *sval;
}Token;


typedef struct {
	Token *tk;
	ObjType objType;
	ValType valType;
	int addr;
	int size;
	int level;
}Sym;

typedef enum {
	OT_FUN,
	OT_VAR,
	OT_CONST
}ObjType;

typedef enum {
	VT_INT,
	VT_BOOL,
	VT_STRING
}ValType;


//结束
*/