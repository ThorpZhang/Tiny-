/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"



/* states in scanner DFA */
typedef enum
   { START,INASSIGN,INCOMMENT,INNUM,INID,DONE, INGEQ,INLEQ, INSTRING }//新增GEQ，LEQ,INSTRING
   StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{ if (!(linepos < bufsize))   // 初始化 linepos = 0， bufsize = 0
  { lineno++;
    if (fgets(lineBuf,BUFLEN-1,source)) // 从源文件中提取255个字节
    { if (EchoSource)        // EchoSource 初始化为FALSE
		fprintf(listing,"%4d: %s",lineno,lineBuf);
      bufsize = strlen(lineBuf); //新的块的大小
      linepos = 0;           //新读取的代码块的起始位置
      return lineBuf[linepos++];
    }
    else
    { EOF_flag = TRUE;
      return EOF;
    }
  }
  else return lineBuf[linepos++]; // 不需要读取下一个块时
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{ if (!EOF_flag) linepos-- ;}

/* lookup table of reserved words */ //******************新增TINY+的保留字
static struct
    { char* str;
      TokenType tok;
    } reservedWords[MAXRESERVED]
   = {{"if",IF},{"then",THEN},{"else",ELSE},{"end",END},
	  {"repeat",REPEAT},{"until",UNTIL},{"read",READ}, {"int", TK_INT }, {"bool", TK_BOOL}, {"string", TK_STRING},{"while", TK_WHILE}, {"do", TK_DO},
	  {"write",WRITE},{ "true",TK_TURE }, { "false", TK_FALSE },{ "or", TK_OR },{ "and", TK_AND }, { "not" , TK_NOT}  };

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{ int i;
  for (i=0;i<MAXRESERVED;i++)
    if (!strcmp(s,reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the 
 * next token in source file
 */
TokenType getToken(void) // TokenType 类型定义在globals.h中，为enum类型
{  /* index for string into tokenString */
   int tokenStringIndex = 0;
   /* holds current token to be returned */
   TokenType currentToken;
   /* current state - always begins at START */
   StateType state = START;
   /* flag to indicate save to tokenString */
   int save;
   while (state != DONE)
   { int c = getNextChar();
     save = TRUE;
     switch (state)
     { case START:
		 if (isdigit(c)) //是否为整型
			 state = INNUM;
		 else if (isalpha(c)) // 是否为字母
			 state = INID;
		 else if (c == ':')
			 state = INASSIGN;
		 //新增状态识别
		 else if (c == '>')
			 state = INGEQ;
		 else if (c == '<')
			 state = INLEQ;
		 else if (c == '\'')
		 {
			 state = INSTRING;
			 save = FALSE;
		 }
		 //结束
         else if ((c == ' ') || (c == '\t') || (c == '\n'))  //空白字符
           save = FALSE;
         else if (c == '{')
         { save = FALSE;
           state = INCOMMENT;
         }
         else
         { state = DONE;  
           switch (c)
           { case EOF:
               save = FALSE;
               currentToken = ENDFILE;
               break;
             case '=':
               currentToken = EQ;
               break;
             case '<':
               currentToken = LT;
               break;
             case '+':
               currentToken = PLUS;
               break;
             case '-':
               currentToken = MINUS;
               break;
             case '*':
               currentToken = TIMES;
               break;
             case '/':
               currentToken = OVER;
               break;
             case '(':
               currentToken = LPAREN;
               break;
             case ')':
               currentToken = RPAREN;
               break;
             case ';':
               currentToken = SEMI;
               break;
			   //新增的符号识别
			 case '>':
				 currentToken = TK_GER;
				 break;
			 case ',':
				 currentToken = TK_COMMA;
				 break;
			 case '\'':
				 currentToken = TK_QUEOT;
				 break;
				 //结束
             default:
               currentToken = ERROR;
               break;
           }
         }
         break;
       case INCOMMENT:
         save = FALSE;
         if (c == EOF)
         { state = DONE;
           currentToken = ENDFILE;
         }
         else if (c == '}')
			 state = START;
         break;
       case INASSIGN:
         state = DONE;
         if (c == '=')
           currentToken = ASSIGN;
         else
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           currentToken = ERROR;
         }
         break;
		 //新增符号状态识别 >= 和 <=
	   case INGEQ:
		   state = DONE;
		   if (c == '=')
			   currentToken = TK_GEQ;
		   else
		   { /* backup in the input */
			   ungetNextChar();
			  // save = FALSE;
			   currentToken = TK_GER;
		   }
		   break;
	   case INLEQ:
		   state = DONE;
		   if (c == '=')
			   currentToken = TK_LEQ;
		   else
		   { /* backup in the input */
			   ungetNextChar();
			  // save = FALSE;
			   currentToken = LT;
		   }
		   break;
	   case INSTRING:
		   if (c == '\n' || c == EOF)
		   {
			   save = FALSE;
			   state = DONE;
			   currentToken = ERROR;
			   if (c == EOF)
			   {
				   ungetNextChar();
			   }
		   }
		   else if (c == '\'')
		   {
			   save = FALSE;
			   state = DONE;
			   currentToken = STRING;
		   }
		   break;
		   //结束
       case INNUM:
         if (!isdigit(c))
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = NUM;
         }
         break;
       case INID:
         if (!isalpha(c))
         { /* backup in the input */
           ungetNextChar();
           save = FALSE;
           state = DONE;
           currentToken = ID;
         }
         break;
       case DONE:
       default: /* should never happen */
         fprintf(listing,"Scanner Bug: state= %d\n",state);
         state = DONE;
         currentToken = ERROR;
         break;
     }
     if ((save) && (tokenStringIndex <= MAXTOKENLEN))
       tokenString[tokenStringIndex++] = (char) c;
     if (state == DONE)
     { tokenString[tokenStringIndex] = '\0';
       if (currentToken == ID)
         currentToken = reservedLookup(tokenString);
     }
   }
   if (TraceScan) {
     fprintf(listing,"\t%d: ",lineno);
     printToken(currentToken,tokenString);
   }
   return currentToken;
} /* end getToken */

