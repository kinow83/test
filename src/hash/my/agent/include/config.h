/*
*	@ file : config.h
*	@ brief: ���� ������ �ҷ��´�.
*/

#define CONFIG_BUFSIZE		64	// �ִ� ���� ���� ����
#define LIST_CNT					6		// ���� �׸�
#define CONF_FILE "conf/config"	//���� ���� ��ġ
#define BLOCK_PAGE "html/block_page.html"	// default ���� ��������

// ���� ���� �׸�
char *keyword[]={
	"ID=",
	"DEV=",
	"MASTER=",
	"PASSWORD=",
	"URL_PATH=",
};


// ���� ���� ���
char answer[LIST_CNT][CONFIG_BUFSIZE];

// ���� ���� �ҷ�����
void get_conf();
// ���ڿ��� ������� ����
void _strim(char * p);



// ���� ���Ͽ��� ������ ������ ���� ����(answer)�� ����
void get_conf(){
	FILE *fp;
	char buf[CONFIG_BUFSIZE];
	char *p;
	int i=0;
	
	fp = fopen(CONF_FILE, "r");
	if(!fp){
		exit(1);
	}
	
	// ����!! ���� �׸���� keyword ������� �������Ͽ� ������ �־�� �Ѵ�.
	while( fgets(buf, sizeof(buf), fp) ){
		_strim(buf);
		
		if(*buf == '#'){
			continue;
		}
		
		else if( p = strstr(buf, keyword[i])){
			strcpy(answer[i], p+strlen(keyword[i]));
			answer[i][strlen(answer[i])-1] = '\0';
			i++;
		}
	}
			
	if(strlen(answer[LIST_CNT-1]) == 0)
	{
		strcpy(answer[LIST_CNT-1], BLOCK_PAGE);				
	}	
	
}

// str_trim
void _strim(char * p){
	
	char *buf, *prev;
	int i=0;
	
	buf = (char *)malloc(strlen(p)+1);
	prev = p;
	
	while(*p){
		if(*p != ' ' && *p != '\t'){
			buf[i++] = *p;			
		}		
		
		p++;
		
	}
	
	buf[i]=0;	
	strcpy(prev, buf);
	free(buf);

}


