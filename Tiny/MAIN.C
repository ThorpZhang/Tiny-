/****************************************************/
/* File: main.c                                     */
/* Main program for TINY compiler                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

int Error = FALSE;

int main( int argc, char * argv[] )
{ TreeNode * syntaxTree;     //定义语法树
  char pgm[120]; /* source code file nam */
  //if (argc != 2)
   // { fprintf(stderr,"usage: %s <filename>\n",argv[0]);
     // exit(1);
    //}
  //strcpy(pgm,argv[1]) ;
  scanf("%s", &pgm);
  if (strchr (pgm, '.') == NULL)
     strcat(pgm,".tny");
  source = fopen(pgm,"r");  //打开源代码文件
  if (source==NULL)
  { fprintf(stderr,"File %s not found\n",pgm);
    exit(1);
  }
  listing = stdout; /* send listing to screen */
  fprintf(listing,"\nTINY COMPILATION: %s\n",pgm);
#if NO_PARSE                    //解析 ， 初始化为FALSE
  while (getToken()!=ENDFILE); // getToken 在scan.c文件79行
#else
  syntaxTree = parse();       // 语法分析生成语法树
  if (TraceParse) {           // 判断语法树是否生成
    fprintf(listing,"\nSyntax tree:\n");
    printTree(syntaxTree);
  }
#if !NO_ANALYZE             // 语义分析
  if (! Error)
  { if (TraceAnalyze) fprintf(listing,"\nBuilding Symbol Table...\n");
    buildSymtab(syntaxTree);
    if (TraceAnalyze) fprintf(listing,"\nChecking Types...\n");
    typeCheck(syntaxTree);
    if (TraceAnalyze) fprintf(listing,"\nType Checking Finished\n");
  }
#if !NO_CODE               // 中间代码生成
  if (! Error)
  { char * codefile;
    int fnlen = strcspn(pgm,".");   // 返回'.'在pgm 中的下标值， 即文件名长度
    codefile = (char *) calloc(fnlen+4, sizeof(char));
    strncpy(codefile,pgm,fnlen);    // 从pgm中复制fnlen个字符给codefile
    strcat(codefile,".tm");         // 字符串相接
    code = fopen(codefile,"w");
    if (code == NULL)
    { printf("Unable to open %s\n",codefile);
      exit(1);
    }
	fprintf(listing, "\n\nGenerate Three - address Intermediate Code\n\n");
    codeGen(syntaxTree,codefile);   // 中间代码生成
    fclose(code);
  }
#endif
#endif
#endif
  fclose(source);
  system("pause");
  return 0;
}

