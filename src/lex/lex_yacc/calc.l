%{
	#include  "y.tab.h"
	extern int  yylval;
%}
%%
[0-9]+ { yylval = atoi(yytext); return NUMBER;  }
[\t ]+ ;
\n return 0;    /* 논리적인 입력처리열의 끝 */
. return  yytext[0];
%%