/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/


#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include<stdbool.h>

#include"SYMTAB.H"

static TokenType token; /* holds current token */

//新增符号表储存声明

//static Sym sym[100];//最多存储100个变量
//int symTail = 0;//用于跟踪sym当前节点

static int location = 0;//地址值
//结束

/* function prototypes for recursive calls */
static TreeNode * stmt_sequence(void); //获取语句表
static TreeNode * statement(void);
static TreeNode * if_stmt(void);
static TreeNode * repeat_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * read_stmt(void);
static TreeNode * write_stmt(void);
static TreeNode * exp(void);
static TreeNode * simple_exp(void);
static TreeNode * term(void);
static TreeNode * factor(void);

//新增 TINY+ WHILE循环	声明分析函数  and&OR判断函数
static TreeNode * while_stmt(void);
void declarations();
static TreeNode * A_O_exp(void);

//新增Sym符号表操作相关函数
//Sym* insertSym(TokenType tokType, char *tokenString);//插入函数
//Sym* findSym(char *name);
//结束


static void syntaxError(char * message)
{ fprintf(listing,"\n>>> ");
  fprintf(listing,"Syntax error at line %d: %s",lineno,message);
  Error = TRUE;
}

//新增match函数
static bool b_match(TokenType expected)
{
	if (token==expected)
	{
		token = getToken();
		return true;
	}
	else
	{
		return false;
	}
}
//结束

static void match(TokenType expected)
{ if (token == expected) token = getToken();
  else {
    syntaxError("unexpected token -> ");
    printToken(token,tokenString);
    fprintf(listing,"      ");
  }
}

TreeNode * stmt_sequence(void)
{ TreeNode * t = statement();
  TreeNode * p = t;
  while ((token!=ENDFILE) && (token!=END) &&
         (token!=ELSE) && (token!=UNTIL))
  { TreeNode * q;
 //   match(SEMI);
    q = statement();
    if (q!= NULL) {
      if (t==NULL) t = p = q;
      else /* now p cannot be NULL either */
      { p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}

TreeNode * statement(void)
{ TreeNode * t = NULL;
  switch (token) {
    case IF : t = if_stmt(); break;
    case REPEAT : t = repeat_stmt(); break;
    case ID : t = assign_stmt(); break;
    case READ : t = read_stmt(); break;
    case WRITE : t = write_stmt(); break;
		//新增TINY+ WHILE循环
	case TK_WHILE: t = while_stmt(); break;
    default : syntaxError("unexpected token -> ");
              printToken(token,tokenString);
              token = getToken();
              break;
  } /* end case */
  return t;
}

//新增TINY+ WHILE循环
TreeNode * while_stmt(void)
{
	TreeNode *t = newStmtNode(WHILEK);
	match(TK_WHILE);
	if (t != NULL) t->child[0] = A_O_exp();
	match(TK_DO);
	if (t != NULL) t->child[1] = stmt_sequence();
	return t;
}
//结束

TreeNode * if_stmt(void)
{ TreeNode * t = newStmtNode(IfK);
  match(IF);
  if (t!=NULL) t->child[0] = A_O_exp();//新增 and or 判定
  match(THEN);
  if (t!=NULL) t->child[1] = stmt_sequence();
  if (token==ELSE) {
    match(ELSE);
    if (t!=NULL) t->child[2] = stmt_sequence();
  }
  match(END);
  return t;
}

TreeNode * repeat_stmt(void)
{ TreeNode * t = newStmtNode(RepeatK);
  match(REPEAT);
  if (t!=NULL) t->child[0] = stmt_sequence();
  match(UNTIL);
  if (t!=NULL) t->child[1] = A_O_exp();//新增 and or 判定
  return t;
}

TreeNode * assign_stmt(void)
{ TreeNode * t = newStmtNode(AssignK);
  if ((t!=NULL) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  match(ASSIGN);
  if (t!=NULL) t->child[0] = exp();
  return t;
}

TreeNode * read_stmt(void)
{ TreeNode * t = newStmtNode(ReadK);
  match(READ);
  if ((t!=NULL) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  return t;
}

TreeNode * write_stmt(void)
{ TreeNode * t = newStmtNode(WriteK);
  match(WRITE);
  if (t!=NULL) t->child[0] = exp();
  return t;
}
/*
TreeNode * exp(void)
{ TreeNode * t = simple_exp();
  if ((token==LT)||(token==EQ)) {
    TreeNode * p = newExpNode(OpK);
    if (p!=NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
    }
    match(token);
    if (t!=NULL)
      t->child[1] = simple_exp();
  }
  return t;
}
*/

//TINY+ exp function  A_O_exp
TreeNode * A_O_exp(void)
{
	TreeNode * t = exp();
	if ((token==TK_AND)||(token==TK_OR))
	{
		TreeNode * p = newExpNode(BoolK);//And OR 判定节点为 bool 型
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);
		if (t != NULL) {
			t->child[1] = exp();
		}
	}
	return t;
}

TreeNode * exp(void)
{
	TreeNode * t = simple_exp();
	if ((token == LT) || (token == EQ) || (token==TK_GER) || (token== TK_GEQ) || (token== TK_LEQ)) {
		TreeNode * p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);
		if (t != NULL)
			t->child[1] = simple_exp();
	}
	return t;
}
//结束

TreeNode * simple_exp(void)
{ TreeNode * t = term();
  while ((token==PLUS)||(token==MINUS))
  { TreeNode * p = newExpNode(OpK);
    if (p!=NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      t->child[1] = term();
    }
  }
  return t;
}

TreeNode * term(void)
{ TreeNode * t = factor();
  while ((token==TIMES)||(token==OVER))
  { TreeNode * p = newExpNode(OpK);
    if (p!=NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      p->child[1] = factor();
    }
  }
  return t;
}

TreeNode * factor(void)
{ TreeNode * t = NULL;
  switch (token) {
    case NUM :
      t = newExpNode(ConstK);
      if ((t!=NULL) && (token==NUM))
        t->attr.val = atoi(tokenString);
      match(NUM);
      break;
    case ID :
      t = newExpNode(IdK);
      if ((t!=NULL) && (token==ID))
        t->attr.name = copyString(tokenString);
      match(ID);
      break; 
    case LPAREN :
      match(LPAREN);
      t = exp();
      match(RPAREN);
      break;
	  //新增终结符识别
	case TK_TURE:
		t = newExpNode(BoolK);
		if ((t != NULL) && (token == TK_TURE))
			t->attr.name = copyString(tokenString);
		match(TK_TURE);
		break;
	case TK_FALSE:
		t = newExpNode(BoolK);
		if ((t != NULL) && (token == TK_FALSE))
			t->attr.name = copyString(tokenString);
		match(TK_FALSE);
		break;
	case STRING:
		t = newExpNode(StringK);
		if ((t != NULL) && (token == STRING)) 
			t->attr.name = copyString(tokenString);
		match(STRING);
		break;
		//结束
    default:
      syntaxError("unexpected token -> ");
      printToken(token,tokenString);
      token = getToken();
      break;
    }
  return t;
}

//声明分析函数
void declarations()
{
	while ((token==TK_INT)||(token==TK_STRING)||(token==TK_BOOL))
	{
		switch (token)
		{
		case TK_INT:
			token = getToken();
			
				do {
					if (st_lookup(copyString(tokenString)) == -1)
						sym_insert(copyString(tokenString), lineno, location, Integer);
					else
						fprintf(listing, "      ERROR:%s重定义\n", copyString(tokenString));
					match(ID);
				} while (b_match(TK_COMMA));
				match(SEMI);
			break;
		case TK_STRING:
			token = getToken();
			
				do {
					if (st_lookup(copyString(tokenString)) == -1)
						sym_insert(copyString(tokenString), lineno, location, STRing);
					else
						fprintf(listing, "      ERROR:%s重定义\n", copyString(tokenString));
					match(ID);
				} while (b_match(TK_COMMA));
				match(SEMI);
			
			break;
		case TK_BOOL:
			token = getToken();
			
				do {
					if (st_lookup(copyString(tokenString)) == -1)
						sym_insert(copyString(tokenString), lineno, location, Boolean);
					else
						fprintf(listing, "      ERROR:%s重定义\n", copyString(tokenString));
					match(ID);
				} while (b_match(TK_COMMA));
				match(SEMI);
			
			break;
		default:
			break;
		}

	}
}


/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly 
 * constructed syntax tree
 */
TreeNode * parse(void)
{ TreeNode * t;
  token = getToken();       // 获取token
  declarations();           //声明分析
  t = stmt_sequence();      // 获取语句表
  match(END);
  if (token!=ENDFILE)
    syntaxError("Code ends before file\n");
  return t;
}
