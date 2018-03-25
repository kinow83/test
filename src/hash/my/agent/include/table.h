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
// 차단 정책 해쉬 테이블
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


URL	url_node[LEN];					// url 해쉬 테이블
KEY	key_node[MAXKEYWORD];		// 키워드 테이블
IP	ip_node[LEN];						// ip 해쉬 테이블

// make hash key
u_long URL_hash(char *str, u_long key);

// search inputted url, keyword, ip in table
u_long URL_serach(char *s, URL * table);	//url 해쉬 테이블에서 url의 no(고유번호) 찾기
u_long KEY_serach(char *s, KEY * table);	//keyword 테이블에서 일치하는 keyword가 있는지 확
u_long IP_serach(u_long p, IP * table);		//ip 해쉬 테이블에서 ip의 no(고유번호) 찾기


// insert node into table
int URL_insert(char *p, u_long no, URL *table);					// url을 해쉬 테이블에 삽입
int KEY_insert(char * p, u_long no, KEY *table, int n);	// keyword를 테이블에 삽입
int IP_insert(u_long p, u_long no, IP *table);					// ip를 해쉬 테이블에 삽입

// initialize
void URL_init(URL * u);	// url 해쉬 테이블 초기화
void KEY_init(KEY * k); // keyword 테이블 초기화
void IP_init(IP * p);		// ip 해쉬 테이블 초기화

// table remove all nodes
void IP_delete(IP * p);		// ip 해쉬 테이블의 모든 노드 삭제
void URL_delete(URL * u);	// url 해쉬 테이블의 모든 노드 삭제

int URLToTable(char *filepath);	// url 해쉬 테이블 완성하기
int IPToTable(char *filepath);	// ip 해쉬 테이블 완성하기
int KEYToTable(char *filepath);	// keyword 테이블 완성하기

char * _strnstr(char *big, char *little, int len);	// 문자열 길이 지정하여 원본 문자열에 비교 문자열이 포함되는지 확인

