/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include<stdlib.h>

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int tmpOffset = 0;

//新增状态结构
typedef struct
{
	int LTrue, LFalse;
	int LBegin, LNext;
	int BoolSyb;
} StateCode;

//新增三代码地址生成函数、语句判断生成函数、终结符判断生成函数
static char * cGenT_ACode(TreeNode * tree, StateCode *code);
static void cGenStmt(TreeNode * tree, StateCode *S);
static char * cGenExp(TreeNode * tree, StateCode *S);
//新增标志跟踪
static int LabelIndex = -1;
static int TempIndex = 1;
/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);

//新增标志跟踪器
static int updateLabel()
{
	return ++LabelIndex;
}


static void cGenStmt(TreeNode * tree, StateCode *S)
{
	TreeNode * p1, *p2, *p3;
	char *str1;
	StateCode E, E1, E2;
	switch (tree->kind.stmt)
	{
	case IfK:
		p1 = tree->child[0];
		p2 = tree->child[1];
		p3 = tree->child[2];

		E.LTrue = updateLabel();
		if (p3 == NULL)
		{
			E.LFalse = S->LNext;
			E1.LNext = S->LNext;
			E.BoolSyb = 1;
			cGenExp(p1, &E);
			if (p2 != NULL)
				fprintf(listing, "Label L%d\n", E.LTrue);
			cGenT_ACode(p2, &E1);
		}
		else
		{
			E.LFalse = updateLabel();
			E.BoolSyb = 1;
			E1.LNext = S->LNext;
			E2.LNext = S->LNext;
			cGenExp(p1, &E);
			if (p1 != NULL)
				fprintf(listing, "Label L%d\n", E.LTrue);
			cGenT_ACode(p2, &E1);
			fprintf(listing, "goto L%d\n", S->LNext);
			if (p2 != NULL)
				fprintf(listing, "Label L%d\n", E.LFalse);
			cGenT_ACode(p3, &E2);
		}
		break;
	case WHILEK:
		p1 = tree->child[0];
		p2 = tree->child[1];
		E.BoolSyb = 1;
		S->LBegin = updateLabel();
		E.LTrue = updateLabel();
		E.LFalse = S->LNext;
		E1.LNext = S->LBegin;
		fprintf(listing, "Label L%d\n", S->LBegin);
		cGenExp(p1, &E);
		if (p2 != NULL)
			fprintf(listing, "Label L%d\n", E.LTrue);
		cGenT_ACode(p2, &E1);
		fprintf(listing, "goto L%d\n", S->LBegin);
		break;
	case RepeatK:

		p1 = tree->child[0];
		p2 = tree->child[1];

		S->LBegin = updateLabel();
		E1.LNext = updateLabel();
		E.LTrue = S->LNext;
		E.LFalse = S->LBegin;
		E.BoolSyb = 1;
		if (p1 != NULL)
			fprintf(listing, "Label L%d\n", S->LBegin);
		cGenT_ACode(p1, &E1);
		fprintf(listing, "Label L%d\n", E1.LNext);
		cGenExp(p2, &E);
		break;			
	case AssignK:
		str1 = cGenT_ACode(tree->child[0], S);
		fprintf(listing, "%s:=%s\n", tree->attr.name, str1);
		free(str1);
		break;			
	case ReadK:
		fprintf(listing, "read %s\n", tree->attr.name);
		break;
	case WriteK:
		str1 = cGenT_ACode(tree->child[0], S);
		fprintf(listing, "write %s\n", str1);
		free(str1);
		break;
	}

}

static char * cGenExp(TreeNode * tree, StateCode * S)
{
	TreeNode * p1, *p2;
	char *str1, *str2;
	char *str = malloc(20);
	StateCode E1, E2;
	switch (tree->kind.exp)
	{
	case ConstK:
		sprintf(str, "%d", tree->attr.val);
		break;			
	case StringK:                         
	case IdK:
		strcpy(str, tree->attr.name);
		break;							 
	case OpK:
		p1 = tree->child[0];
		p2 = tree->child[1];
		switch (tree->attr.op)
		{
		case TK_AND:
			E1.LTrue = updateLabel();
			E1.LFalse = S->LFalse;
			E2.LTrue = S->LTrue;
			E2.LFalse = S->LFalse;
			E1.BoolSyb = 1;
			E2.BoolSyb = 1;
			str1 = cGenExp(p1, &E1);
			fprintf(listing, "Label L%d\n", E1.LTrue);
			str2 = cGenExp(p2, &E2);
			break;
		case TK_OR:
			E1.LTrue = S->LTrue;
			E1.LFalse = updateLabel();
			E2.LTrue = S->LTrue;
			E2.LFalse = S->LFalse;
			str1 = cGenExp(p1, &E1);
			fprintf(listing, "Label L%d\n", S->LFalse);
			str2 = cGenExp(p2, &E2);
			break;
		case TK_NOT:
			E1.LTrue = S->LFalse;
			E1.LFalse = S->LTrue;
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			break;
		case TK_GER:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			if (S->BoolSyb == 1)
			{
				fprintf(listing, "if %s > %s goto L%d\n", str1, str2, S->LTrue);
				fprintf(listing, "goto L%d\n", S->LFalse);
			}
			else
				fprintf(listing, "t%d := %s > %s\n", TempIndex++, str1, str2);
			break;
		case LT:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			if (S->BoolSyb == 1)
			{
				fprintf(listing, "if %s < %s goto L%d\n", str1, str2, S->LTrue);
				fprintf(listing, "goto L%d\n", S->LFalse);
			}
			else
			{
				fprintf(listing, "t%d := %s < %s\n", TempIndex++, str1, str2);
			}
			break;
		case EQ:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			if (S->BoolSyb == 1)
			{
				fprintf(listing, "if %s := %s goto L%d\n", str1, str2, S->LTrue);
				fprintf(listing, "goto L%d\n", S->LFalse);
			}
			else
				fprintf(listing, "t%d := %s =  %s\n", TempIndex++, str1, str2);
			break;
		case TK_GEQ:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			if (S->BoolSyb == 1)
			{
				fprintf(listing, "if %s >= %s goto L%d\n", str1, str2, S->LTrue);
				fprintf(listing, "goto L%d\n", S->LFalse);
			}
			else
				fprintf(listing, "t%d := %s >= %s\n", TempIndex++, str1, str2);
			break;
		case TK_LEQ:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			if (S->BoolSyb == 1)
			{
				fprintf(listing, "if %s <= %s goto L%d\n", str1, str2, S->LTrue);
				fprintf(listing, "goto L%d\n", S->LFalse);
			}
			else
				fprintf(listing, "t%d :=%s <= %s\n", TempIndex++, str1, str2);
			break;
		case PLUS:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			fprintf(listing, "t%d = %s + %s\n", TempIndex++, str1, str2);
			break;
		case MINUS:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			fprintf(listing, "t%d = %s - %s\n", TempIndex++, str1, str2);
			break;
		case TIMES:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			fprintf(listing, "t%d = %s * %s\n", TempIndex++, str1, str2);
			break;
		case OVER:
			str1 = cGenExp(p1, &E1);
			str2 = cGenExp(p2, &E2);
			fprintf(listing, "t%d = %s / %s\n", TempIndex++, str1, str2);
			break;
		}
		free(str1);
		free(str2);
		sprintf(str, "%s%d", "t", TempIndex);
		break;
	}
	return str;
}

char * cGenT_ACode(TreeNode * tree, StateCode *code)
{
	char * str = NULL;
	StateCode E, E1;
	if (tree != NULL)
	{
		switch (tree->nodekind) {
		case StmtK:
			switch (tree->kind.stmt)
			{
			case RepeatK:
			case WHILEK:
			case IfK:
			case AssignK:
			case ReadK:
			case WriteK:
				E.LNext = updateLabel();
				E1.LNext = code->LNext;
				cGenStmt(tree, &E);
				fprintf(listing, "Label L%d\n", E.LNext);
				break;
			}
			break;
		case ExpK:
			str = cGenExp(tree, code);
			break;
		default:
			break;
		}
		cGenT_ACode(tree->sibling, &E1);
	}
	return str;
}


/* Procedure genStmt generates code at a statement node */

static void genStmt( TreeNode * tree)
{ TreeNode * p1, * p2, * p3;
  int savedLoc1,savedLoc2,currentLoc;
  int loc;
  switch (tree->kind.stmt) {

      case IfK :
         if (TraceCode) emitComment("-> if") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         p3 = tree->child[2] ;
         /* generate code for test expression */
         cGen(p1);
         savedLoc1 = emitSkip(1) ;
         emitComment("if: jump to else belongs here");
         /* recurse on then part */
         cGen(p2);
         savedLoc2 = emitSkip(1) ;
         emitComment("if: jump to end belongs here");
         currentLoc = emitSkip(0) ;
         emitBackup(savedLoc1) ;
         emitRM_Abs("JEQ",ac,currentLoc,"if: jmp to else");
         emitRestore() ;
         /* recurse on else part */
         cGen(p3);
         currentLoc = emitSkip(0) ;
         emitBackup(savedLoc2) ;
         emitRM_Abs("LDA",pc,currentLoc,"jmp to end") ;
         emitRestore() ;
         if (TraceCode)  emitComment("<- if") ;
         break; /* if_k */

      case RepeatK:
         if (TraceCode) emitComment("-> repeat") ;
         p1 = tree->child[0] ;
         p2 = tree->child[1] ;
         savedLoc1 = emitSkip(0);
         emitComment("repeat: jump after body comes back here");
         /* generate code for body */
         cGen(p1);
         /* generate code for test */
         cGen(p2);
         emitRM_Abs("JEQ",ac,savedLoc1,"repeat: jmp back to body");
         if (TraceCode)  emitComment("<- repeat") ;
         break; /* repeat */

      case AssignK:
         if (TraceCode) emitComment("-> assign") ;
         /* generate code for rhs */
         cGen(tree->child[0]);
         /* now store value */
         loc = st_lookup(tree->attr.name);
         emitRM("ST",ac,loc,gp,"assign: store value");
         if (TraceCode)  emitComment("<- assign") ;
         break; /* assign_k */

      case ReadK:
         emitRO("IN",ac,0,0,"read integer value");
         loc = st_lookup(tree->attr.name);
         emitRM("ST",ac,loc,gp,"read: store value");
         break;
      case WriteK:
         /* generate code for expression to write */
         cGen(tree->child[0]);
         /* now output it */
         emitRO("OUT",ac,0,0,"write ac");
         break;
      default:
         break;
    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{ int loc;
  TreeNode * p1, * p2;
  switch (tree->kind.exp) {

    case ConstK :
      if (TraceCode) emitComment("-> Const") ;
      /* gen code to load integer constant using LDC */
      emitRM("LDC",ac,tree->attr.val,0,"load const");
      if (TraceCode)  emitComment("<- Const") ;
      break; /* ConstK */
    
    case IdK :
      if (TraceCode) emitComment("-> Id") ;
      loc = st_lookup(tree->attr.name);
      emitRM("LD",ac,loc,gp,"load id value");
      if (TraceCode)  emitComment("<- Id") ;
      break; /* IdK */

    case OpK :
         if (TraceCode) emitComment("-> Op") ;
         p1 = tree->child[0];
         p2 = tree->child[1];
         /* gen code for ac = left arg */
         cGen(p1);
         /* gen code to push left operand */
         emitRM("ST",ac,tmpOffset--,mp,"op: push left");
         /* gen code for ac = right operand */
         cGen(p2);
         /* now load left operand */
         emitRM("LD",ac1,++tmpOffset,mp,"op: load left");
         switch (tree->attr.op) {
            case PLUS :
               emitRO("ADD",ac,ac1,ac,"op +");
               break;
            case MINUS :
               emitRO("SUB",ac,ac1,ac,"op -");
               break;
            case TIMES :
               emitRO("MUL",ac,ac1,ac,"op *");
               break;
            case OVER :
               emitRO("DIV",ac,ac1,ac,"op /");
               break;
            case LT :
               emitRO("SUB",ac,ac1,ac,"op <") ;
               emitRM("JLT",ac,2,pc,"br if true") ;
               emitRM("LDC",ac,0,ac,"false case") ;
               emitRM("LDA",pc,1,pc,"unconditional jmp") ;
               emitRM("LDC",ac,1,ac,"true case") ;
               break;
            case EQ :
               emitRO("SUB",ac,ac1,ac,"op ==") ;
               emitRM("JEQ",ac,2,pc,"br if true");
               emitRM("LDC",ac,0,ac,"false case") ;
               emitRM("LDA",pc,1,pc,"unconditional jmp") ;
               emitRM("LDC",ac,1,ac,"true case") ;
               break;
            default:
               emitComment("BUG: Unknown operator");
               break;
         } /* case op */
         if (TraceCode)  emitComment("<- Op") ;
         break; /* OpK */

    default:
      break;
  }
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree)
{ if (tree != NULL)
  { switch (tree->nodekind) {
      case StmtK:
        genStmt(tree);
        break;
      case ExpK:
        genExp(tree);
        break;
      default:
        break;
    }
    cGen(tree->sibling);
  }
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
/*
void codeGen(TreeNode * syntaxTree, char * codefile)
{  char * s = (char *)malloc(strlen(codefile)+7);
   strcpy(s,"File: ");
   strcat(s,codefile);
   emitComment("TINY Compilation to TM Code");
   emitComment(s);
   emitComment("Standard prelude:");
   emitRM("LD",mp,0,ac,"load maxaddress from location 0");
   emitRM("ST",ac,0,ac,"clear location 0");
   emitComment("End of standard prelude.");
   cGen(syntaxTree);
   emitComment("End of execution.");
   emitRO("HALT",0,0,0,"");
}
*/
void codeGen(TreeNode * syntaxTree, char * codefile)
{  
	StateCode code;
	code.LBegin = LabelIndex;
	code.LNext = LabelIndex;
	code.LTrue = LabelIndex;
	code.LFalse = LabelIndex;
	cGenT_ACode(syntaxTree, &code);
}