/*
* @file: table.h
*/

#define LEN		500000
#define HASH		5432
#define NON_VALUE	-1
#define ZERO_VALUE	0
#define MAXKEYWORD 	5000
#define MAXWORD 	25


////////////////////////////////////////////////
// ���� ��å �ؽ� ���̺�
////////////////////////////////////////////////

typedef struct _url{
	char * data;
	u_long no;
	struct _url *next;
}URL;


typedef struct _ip{
	u_long data;
	u_long no;
	struct _ip *next;
}IP;


typedef struct _keyword{
	char data[MAXWORD];
	u_long no;
}KEY;


URL	url_node[LEN];					// url �ؽ� ���̺�
KEY	key_node[MAXKEYWORD];		// Ű���� ���̺�
IP	ip_node[LEN];						// ip �ؽ� ���̺�

// make hash key
u_long URL_hash(char *str, u_long key);

// search inputted url, keyword, ip in table
u_long URL_serach(char *s, URL * table);	//url �ؽ� ���̺��� url�� no(������ȣ) ã��
u_long KEY_serach(char *s, KEY * table);	//keyword ���̺��� ��ġ�ϴ� keyword�� �ִ��� Ȯ
u_long IP_serach(u_long p, IP * table);		//ip �ؽ� ���̺��� ip�� no(������ȣ) ã��


// insert node into table
int URL_insert(char *p, u_long no, URL *table);					// url�� �ؽ� ���̺� ����
int KEY_insert(char * p, u_long no, KEY *table, int n);	// keyword�� ���̺� ����
int IP_insert(u_long p, u_long no, IP *table);					// ip�� �ؽ� ���̺� ����

// initialize
void URL_init(URL * u);	// url �ؽ� ���̺� �ʱ�ȭ
void KEY_init(KEY * k); // keyword ���̺� �ʱ�ȭ
void IP_init(IP * p);		// ip �ؽ� ���̺� �ʱ�ȭ

// table remove all nodes
void IP_delete(IP * p);		// ip �ؽ� ���̺��� ��� ��� ����
void URL_delete(URL * u);	// url �ؽ� ���̺��� ��� ��� ����

int URLToTable(char *filepath);	// url �ؽ� ���̺� �ϼ��ϱ�
int IPToTable(char *filepath);	// ip �ؽ� ���̺� �ϼ��ϱ�
int KEYToTable(char *filepath);	// keyword ���̺� �ϼ��ϱ�

char * _strnstr(char *big, char *little, int len);	// ���ڿ� ���� �����Ͽ� ���� ���ڿ��� �� ���ڿ��� ���ԵǴ��� Ȯ��

