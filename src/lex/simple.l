%{
       /*
        * My favorite...
        */
		// int A = 10;
%}
%%
[\t]+	;
rain |
rose	{ printf("%s !! That is my favorite.\n", yytext); }
love |
story	{ printf("%s !! I like a love story.\n", yytext); }
"potato chip" |
potato |
chip	{ printf("%s ...!! now and then, I have a potato chip.\n", yytext); }
[A-Za-z]+	{ printf("%s..., What is that?\n", yytext); }
.|\n	{ ECHO; }
%%

int main()
{
	yylex();
	return 0;
}
