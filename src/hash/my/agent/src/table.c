/*
* @ file  : hash.c hash.h
* @ brief : hash table( create, insert, lookup) - type( url, ip, keyword )
*         : gcc table.c -c 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../include/table.h"

#define HASH_KEY 5345	//해쉬 키


/******************************************************************
*                       url hash table                            *
******************************************************************/
//
// make hash
//
u_long URL_hash
(char *str, u_long key)
{
        int c;

        while (c = *str++)
            key = ((key << 5) + key) + c; /* hash * 33 + c */

        return key ;
}


//
// url 해쉬 테이블 초기화
//
void URL_init
(URL * u)
{
	int i;
	for(i=0; i<LEN; i++)
	{
		u[i].next = NULL;
		u[i].data = NULL;
		u[i].no = ZERO_VALUE;
	}
	return;	
}


//
// url 해쉬 테이블에 url 삽입
//
int URL_insert
(char *p, u_long no, URL *table){//   p:url,   no:url 고유번호,   table:url 해쉬 테이블
	int key;
	URL * n, * cur;

	key = URL_hash(p, HASH_KEY) % LEN;	//해쉬	값
	cur = &table[key];
	
	if(cur->data == NULL){
		cur->data = (char *) malloc (strlen(p)+1);
		strcpy(cur->data, p);
		cur->no = no;
		cur->next = NULL;		
	}
	else{
		n  = (URL *) malloc ( sizeof(URL) );
		n->data = (char *) malloc ( strlen(p)+1 );
		strcpy( n->data, p);
		n->no =  no;
		
		n->next = cur->next;
		cur->next =  n;		
	}
	return 0;
}



//
// url 해쉬 테이블의 모든 노드 삭제
//
void URL_delete
(URL * u)
{
	int i;
	URL *c, *t;
	
	for(i=0; i<LEN; i++)
	{
		c = &u[i];
		c->data = NULL;
		c->no = ZERO_VALUE;
		
		c = c->next;
		while(c)
		{
			t = c->next;
			free(c);
			c = t;
		}
	}
	return;
}


//
// url 해쉬 테이블에서 url의 no(고유 번호) 찾기
//
u_long URL_serach
(char *s, URL * table)	// s: 찾을 url
{
        int key;
        URL * c;

        key = URL_hash(s, HASH_KEY) % LEN;

        c = &table[key];

        if(c->data == NULL)
                return 0;

        while(c){
                if(strcmp(c->data, s) == 0){
                        return c->no;
                }
                c = c->next;
        }
        return 0;
}


/******************************************************************
*                       ip hash table                             *
******************************************************************/

//
// ip 해쉬 테이블 초기화
//
void IP_init
(IP * p)
{
        int i;
        for(i=0; i<LEN; i++)
        {
                p[i].next = NULL;
                p[i].data = ZERO_VALUE;
                p[i].no = ZERO_VALUE;
        }
        return;
}


//
// ip 해쉬 테이블에 ip 삽입
//
IP * IP_create
(u_long no, u_long p)
{
        IP * n;
        n  = (IP *) malloc ( sizeof(IP) );
        n->data = p;
        n->no =  no;
        n->next = NULL;
        return n;
}


//
// ip 해쉬 테이블에 ip 삽입
//
int IP_insert
(u_long p, u_long no, IP *table)
{
	int key;
	IP * n, * cur;
	
	key = p % LEN;        //해쉬  값
	cur = &table[key];
	
	if(cur->data == ZERO_VALUE){
		cur->data = p;
		cur->no = no;
		cur->next = NULL;
	}
	else{
		n  = (IP *) malloc ( sizeof(IP) );
		n->data = p;
    n->no =  no;
    
    n->next = cur->next;
		cur->next =  n;
	}
	return 0;
}


//
// ip 해쉬 테이블 모든 노드 삭제
//
void IP_delete
(IP * p)
{
        int i;
        IP *c, *t;

        for(i=0; i<LEN; i++)
        {
                c = &p[i];
                c->data = ZERO_VALUE;
                c->no = ZERO_VALUE;

                c = c->next;
                while(c)
                {
                        t = c->next;
                        free(c);
                        c = t;
                }
        }
        return;
}


//
// ip 해쉬 테이블에서 ip의 no(고유 번호) 찾기
//
u_long IP_serach
(u_long p, IP * table)	// p: ip
{
        int key;
        IP * c;

        key = p % LEN; // 해쉬 값

        c = &table[key];

        if(c->data == ZERO_VALUE)
                return 0;

        while(c){
                if(c->data == p){
                        return c->no;
                }
                c = c->next;
        }
        return 0;
}


/******************************************************************
*                    keyword hash table                           *
******************************************************************/

//
// keyword 테이블 초기화
//
void KEY_init
(KEY * k)
{
        int i;
        for(i=0; i<MAXKEYWORD; i++)
        {
                memset( k[i].data, '\0', MAXWORD );
                k[i].no = ZERO_VALUE;
        }
        return;
}


//
// keyword 테이블에 keyword 삽입
//
int KEY_insert
(char * p, u_long no, KEY *table, int n)
{
        int key;
	if( strlen(p)+1 <= MAXWORD && n < MAXKEYWORD )
	{
		strcpy( table[n].data, p );
		table[n].no = no;
	  return 0;
	}
	return -1;
}



//
//keyword 테이블에서 일치하는 keyword가 있는지 확인
//
u_long KEY_serach
(char *s, KEY * table)
{
	int i;
	char *p = (char *)s+10;
	
	for(i=0; i<MAXKEYWORD; i++)
	{
		// MAXKEYWORD : 5000
		if (_strnstr(p, table[i].data , 200) )
		{		
			return table[i].no;			
		}
	}

	return 0;
}






/******************************************************************
*                    create  hash table                           *
******************************************************************/

//
// URL 해쉬 테이블 완성 하기
//
int URLToTable
(char *filepath)
{
	FILE * fp;
	char buf[350];
	char url[300];
	u_long 	n;
	
	// open url file
	if ( (fp = fopen(filepath, "r")) == NULL )
	{
		printf("%s FILE OPEN ERROR",filepath);
		return -1;
	}

	// url 테이블 초기화
	URL_delete(url_node);
	

	
	while( fgets(buf, sizeof(buf), fp) )
	{
		if(buf != NULL && strcmp(buf, "\r\n") && strcmp(buf, "\n"))
		{
			sscanf(buf, "%u	%s", &n, url);
			
			// 'url' string length check
			if( strlen(url) > 256)
			{
				puts("function ERROR:URLToTable, overstring url");
				return -1;
			}
			
			// insert url node table
			if( URL_insert(url, n, url_node) != 0)
			{
				printf("URL_insert ERROR, URL:%s\n",url);
				return -1;				
			}
		}
	}
	fclose(fp);
	return 0;
}


//
// IP 해쉬 테이블 완성 하기
//
int IPToTable
(char *filepath)
{
        FILE * fp;
        char buf[70];
        char ip[20];
        u_long  n, addr;

        // open ip file
        if ( (fp = fopen(filepath, "r")) == NULL )
        {
                printf("%s FILE OPEN ERROR",filepath);
                return -1;
        }

        // ip 해쉬 테이블 초기화
        IP_delete(ip_node);
        
        
        while( fgets(buf, sizeof(buf), fp) )
        {
                if(buf != NULL && strcmp(buf, "\r\n") && strcmp(buf, "\n"))
                {
                        sscanf(buf, "%u  %s", &n, ip);
                        
                        // 'ip' string length check
                        if( strlen(ip) > 15)
                        {
                                puts("function ERROR:IPToTable, overstring ip");
                                return -1;
                        }
                        
                        // ip를 unsigned  long 타입의 정수로 변환
                        addr = inet_addr(ip);

                        // insert ip node table
                        if( IP_insert(addr, n, ip_node) != 0)
                        {
                                printf("IP_insert ERROR, IP:%s\n",ip);
                        }
                }
        }

        fclose(fp);
				return 0;
}


//
// keyword 테이블 완성하기
//
int KEYToTable
(char *filepath)
{
        FILE * fp;
        char buf[80];
        char key[MAXWORD];
        u_long  n, addr;
				int i=0;

        // open url file
        if ( (fp = fopen(filepath, "r")) == NULL )
        {
                printf("%s FILE OPEN ERROR",filepath);
                return -1;
        }

        // keyword 테이블 초기화
				KEY_init(key_node);

        
        while( fgets(buf, sizeof(buf), fp) )
        {
                if(buf != NULL && strcmp(buf, "\r\n") && strcmp(buf, "\n"))
                {
                        sscanf(buf, "%u  %s", &n, key);
                        
                        // 'key' string length check
                        if( strlen(key) > MAXWORD)
                        {
                                puts("function ERROR:KEYToTable, overstring key");
                                return -1;
                        }

                        // insert key node table
                        if( KEY_insert(key, n, key_node, i) != 0)
                        {
                                printf("KEY_insert ERROR, KEY:%s\n",key);
                        }
                }
								i++;
        }
        fclose(fp);
        return 0;
}



/*  strstr 함수에 검색 길이 조건 추가  */
char * _strnstr(char *big, char *little, int len)
{
        char *p1, *p2;
        while(len)
        {
                while(len > 0 && *big++ != *little) len--;
                if(len <= 0) break;
                p1 = big-1;
                p2 = little;                
                while(*p2 && *p1 == *p2) { p1++; p2++;}
                if(!*p2) return big-1;
                len--;
        }
        return NULL;
}
