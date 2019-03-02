/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{ switch (token)
  { case IF: fprintf(listing, "(TK_IF,%s)\n",tokenString); break;
    case THEN: fprintf(listing, "(TK_THEN,%s)\n", tokenString); break;
    case ELSE: fprintf(listing, "(TK_ELSE,%s)\n", tokenString); break;
    case END: fprintf(listing, "(TK_END,%s)\n", tokenString); break;
    case REPEAT: fprintf(listing, "(TK_REPEAT,%s)\n", tokenString); break;
    case UNTIL: fprintf(listing, "(TK_UNTIL,%s)\n", tokenString); break;
    case READ: fprintf(listing, "(TK_READ,%s)\n", tokenString); break;
    case WRITE: fprintf(listing, "(TK_WRITE,%s)\n", tokenString); break;
		//新增的TINY+保留字
	case TK_INT:fprintf(listing, "(TK_INT,%s)\n", tokenString); break;
	case TK_TURE:fprintf(listing, "(TK_TRUE,%s)\n", tokenString); break;
	case TK_FALSE:fprintf(listing, "(TK_FALSE,%s)\n", tokenString); break;
	case TK_AND:fprintf(listing, "(TK_AND,%s)\n", tokenString); break;
	case TK_BOOL:fprintf(listing, "(TK_BOOL,%s)\n", tokenString); break;
	case TK_STRING:fprintf(listing, "(TK_STRING,%s)\n", tokenString); break;
	case STRING:fprintf(listing, "(STRING,%s)\n", tokenString); break;
	case TK_WHILE:fprintf(listing, "(TK_WHILE,%s)\n", tokenString); break;
	case TK_OR:fprintf(listing, "(TK_OR,%s)\n", tokenString); break;
	case TK_NOT:fprintf(listing, "(TK_NOT,%s)\n", tokenString); break;
	case TK_DO:fprintf(listing, "(TK_DO,%s)\n", tokenString); break;
		//结束
//      fprintf(listing,
//         "reserved word: %s\n",tokenString);
//      break;
    case ASSIGN: fprintf(listing,"( TK_ASSIGN, := )\n"); break;
    case LT: fprintf(listing,"( TK_LT, < )\n"); break;
    case EQ: fprintf(listing,"( TK_EQ, = )\n"); break;
    case LPAREN: fprintf(listing,"( TK_LPAREN, ( )\n"); break;
    case RPAREN: fprintf(listing,"( TK_RPAREN, ) )\n"); break;
    case SEMI: fprintf(listing,"( TK_SEMI, ; )\n"); break;
    case PLUS: fprintf(listing,"( TK_PLUS, + )\n"); break;
    case MINUS: fprintf(listing,"( TK)MINUS, - )\n"); break;
    case TIMES: fprintf(listing,"( TK_TIMES, * )\n"); break;
		//新增符号识别
	case TK_GER:fprintf(listing, "( TK_GER, > )\n"); break;
	case TK_GEQ:fprintf(listing, "( TK_GEQ, >= )\n"); break;
	case TK_LEQ:fprintf(listing, "( TK_LEQ, <= )\n"); break;
	case TK_COMMA:fprintf(listing, "( TK_COMMA, , )\n"); break;
	case TK_QUEOT:fprintf(listing, "( TK_QUEOT, ' )\n"); break;
		//结束
    case OVER: fprintf(listing,"( TK_OVER, / )\n"); break;
    case ENDFILE: fprintf(listing,"( TK_ENDFILE, EOF )\n"); break;
    case NUM:
      fprintf(listing,
          "(NUM, val= %s)\n",tokenString);
      break;
    case ID:
      fprintf(listing,
          "(ID, name= %s)\n",tokenString);
      break;
    case ERROR:
      fprintf(listing,
          "(ERROR: %s)\n",tokenString);
      break;
    default: /* should never happen */
      fprintf(listing,"(Unknown token: %d)\n",token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = (char *)malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { switch (tree->kind.stmt) {
		//新增 while
	    case WHILEK:
		  fprintf(listing, "While\n");
		  break;
        case IfK:
          fprintf(listing,"If\n");
          break;
        case RepeatK:
          fprintf(listing,"Repeat\n");
          break;
        case AssignK:
          fprintf(listing,"Assign to: %s\n",tree->attr.name);
          break;
        case ReadK:
          fprintf(listing,"Read: %s\n",tree->attr.name);
          break;
        case WriteK:
          fprintf(listing,"Write\n");
          break;
        default:
          fprintf(listing,"Unknown StmtNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { switch (tree->kind.exp) {
		//新增 bool 型 string 型
	    case BoolK:
		  fprintf(listing, "Bool: ");
		  printToken(tree->attr.op, "\0");
		  break;
		case StringK:
		  fprintf(listing, "String: %s\n",tree->attr.name);
		  break;
        case OpK:
          fprintf(listing,"Op: ");
          printToken(tree->attr.op,"\0");
          break;
        case ConstK:
          fprintf(listing,"Const: %d\n",tree->attr.val);
          break;
        case IdK:
          fprintf(listing,"Id: %s\n",tree->attr.name);
          break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
