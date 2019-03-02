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

/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case AssignK:
        case ReadK:
			if (st_lookup(t->attr.name) == -1)
				/* not yet in table, so treat as new definition */
				//  st_insert(t->attr.name,t->lineno,location++);
				fprintf(listing,"Error:no such value. name:%s,line%d\n", t->attr.name, t->lineno);
			else
				/* already in table, so ignore location,
				   add line number of use only */
				   // st_insert(t->attr.name,t->lineno,0);
			{
				sym_insert(t->attr.name, t->lineno, 0, st_lookup_type(t->attr.name));
				t->type = st_lookup_type(t->attr.name);
		  }
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
          if (st_lookup(t->attr.name) == -1)
          /* not yet in table, so treat as new definition */
          //  st_insert(t->attr.name,t->lineno,location++);
			  fprintf(listing,"Error:no such value. name:%s,line%d\n", t->attr.name, t->lineno);
          else
          /* already in table, so ignore location, 
             add line number of use only */ 
         //   st_insert(t->attr.name,t->lineno,0);
		  // st_insert(t->attr.name,t->lineno,0);
		  {
			  sym_insert(t->attr.name, t->lineno, 0, st_lookup_type(t->attr.name));
			  t->type = st_lookup_type(t->attr.name);
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

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)   // 创建符号表
{ traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case BoolK://新增 对 BOOLK 的判定
		  if ((t->child[0]->type != Boolean) || (t->child[1]->type != Boolean)) {
			  typeError(t, "not a boolean");
		  }
		  else
		  {
			  t->type = Boolean;
		  }
		  break;
	    case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");
          if ((t->attr.op == EQ) || (t->attr.op == LT)||(t->attr.op==TK_GER)||(t->attr.op==TK_GEQ)||(t->attr.op==TK_LEQ))
            t->type = Boolean;
          else
            t->type = Integer;
          break;
        case ConstK:
			t->type = Integer;
			break;
		case StringK:
			t->type = STRing;
			break;
        case IdK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK://符合类型检查
//          if ((t->child[0]->type != Integer)&&(t->child[0]->type!=STRing)&&(t->child[0]->type != Boolean))
			if (t->child[0]->type!=st_lookup_type(t->attr.name))
            typeError(t->child[0],"not the expected type");
          break;
        case WriteK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"write of non-integer value");
          break;
        case RepeatK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
		  //新增 对 WHILE 的判定
		case WHILEK:
			if (t->child[0]->type != Boolean)
				typeError(t->child[0], "while test is not Boolean");
			break;
			//结束
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
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}

