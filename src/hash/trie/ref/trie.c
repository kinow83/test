#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node{
	struct node* child[26];
	int word;
}node;

node* newNode(){ //init
	node* new = (node *)malloc(sizeof(node));
	new->word=0;
	for (int i=0; i<26; i++) new->child[i]=0;
	return new;
}

void insert(node* root, const char* str){
	int len = strlen(str);
	node* now = root;

	for (int i=0; i<len;i++){
		if (!now->child[str[i]-'a']) now->child[str[i]-'a']= newNode();
		now=now->child[str[i]-'a'];
	}
	now->word=1;
}

int search(node* root, const char* str){
	int len = strlen(str);
	node* now = root;

	for (int i=0; i<len; i++){ 
		if (!now->child[str[i]-'a']) return 0;
		now=now->child[str[i]-'a'];
	}
	return now->word;
}

void showtree(node* now, char* str, int depth){
	if (now->word) printf("%s\n", str);

	for (int i=0; i<26; i++){
		if(now->child[i]) {
			str[depth]=i+'a';
			str[depth+1]=0;
			showtree(now->child[i], str, depth+1);
		}
	}
}

int delete(node* now, const char* str, int i){
	if (i == strlen(str)){
		int chk=0;
		for (int i=0; i<26; i++){
			if (now->child[i]) chk=1;
		}
		if (chk) return 0;
		return 1;
	}

	if ( now->child[str[i]-'a']){
		if (delete(now->child[str[i]-'a'], str, i+1)){
			free(now->child[str[i]-'a']);
			now->child[str[i]-'a']=0;

			int chk=0; 
			for (int i=0; i<26; i++){
				if (now->child[i]) chk=1;
			}

			if (chk) return 0;
			return 1;
		}
	}
	return 0;
}	

void recommend(node* root, char* str){
	int len=strlen(str);

	node* now = root;
	for (int i=0; i<len; i++) {
		if (!now->child[str[i]-'a']) return;
		now=now->child[str[i]-'a'];
	}

	showtree(now, str, len);
}

void printhead(const char* str){
	printf("\n==============%s=================\n\n", str);
}

int main(){

	char my[][8]={
		"below", "blue",
		"ban", "bzak",
		"bzu",  "a",
		"im", "i"
	};

	node* root = newNode();

	for (int i=0; i<8; i++) insert(root, my[i]);

	char tmp[20];
	printhead("show all word");
	showtree(root, tmp, 0);

	printhead("do i have?... ");
	printf("%s : %s\n", "bz", search(root, "bz")?"YES":"NO");
	printf("%s : %s\n", "blue", search(root, "blue")?"YES":"NO");
	printf("%s : %s\n", "i", search(root, "i")?"YES":"NO");

	strcpy(tmp, "blue");
	printf("\n\nafter deleting %s word\n\n", tmp);
	delete(root, tmp, 0);

	printhead("show all word");
	showtree(root, tmp, 0);
	printhead("do i have?... ");

	printf("%s : %s\n", "bz", search(root, "bz")?"YES":"NO");
	printf("%s : %s\n", "blue", search(root, "blue")?"YES":"NO");
	printf("%s : %s\n", "i", search(root, "i")?"YES":"NO");

	char help[20]="bz";
	printf("\n\ndo i have any word start with %s?... \n\n", help);
	recommend(root, help);
}
