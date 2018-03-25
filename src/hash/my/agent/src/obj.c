/*
* @file: obj.c
* @brief: network object table( create, insert, lookup )
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../include/obj.h"


//
//  AVL 트리 왼쪽 자식, 오른쪽 자식 비교 함수
//
int compare_str
(const void *str1, const void *str2) 
{
	int	retval;

	//  Compare two u_long.
	if ((retval = *(const unsigned long *)str1 - *(const unsigned long *)str2) > 0)
   	return 1;
  else if (retval < 0)
  	return -1;
  else
   	return 0;
}


//
//그 룹오브젝트  : 에이전트 네트워크에 속한 호스트들이 개별 정책이 설정된 호스트 인지 판단
//							 : 개별 정책이 설정된 호스트 인지 판단하는 이유는 개별 정책이 설정되지 않은 호스트는 
//							 : 네트워크 오브젝트(개별 정책)를 탐색할 필요가 없이, 기본 정책 설정을 따르기 때문이다.
//

// 그룹 오브젝트 초기화
// 배열 크기(OBJ_SIZE) = 256 : C CLASS 크기
void OBJ_group_init(void)
{
	int i;
        for(i=0; i<OBJ_SIZE; i++)
	{
		url_obj_group[i] = 0;   // Valid policy network url check
		ip_obj_group[i] = 0;   	// Valid policy network ip check
		key_obj_group[i] = 0;   // Valid policy network keyword check
	}
}


//
// 네트워크 오브젝트 초기화
// 네트워크 오브젝트는 256개의 AVL 트리 배열로 이루어져있다.
// 예를 들면,
// 오브제트 배열[10]은  아이피 xxx.xxx.xxx.10 인 호스트의 정책 내용이 담겨져 있으며
// 정책 내용(AVL 트리 내용)은 url 또는 ip 의 고유번호들로 이루어져 있다.
//
// 패킷 캡쳐 결과 source ip 가 xxx.xxx.xxx.10 인 패킷의 요청페이지를 검사하여 차단 정책에 걸리게 되면
// 차단 페이지를 xxx.xxx.xxx.10(source ip)로 날리게 된다.
//
void OBJ_init
(OBJ * o)
{
  int i;
	for(i=0; i<OBJ_SIZE; i++)
  {
		bistree_destroy( &(o[i].tree) );									// tree all node delete		
		bistree_init( &(o[i].tree) , compare_str, NULL);	//compare function
  }
  return;
}


//
// insert tree node of obj table
//
int OBJ_insert
(u_long p, u_long no, OBJ *o)
{
	char ty[5];
	u_char * index = (u_char *)&p;	// ip의 마지막 옥텟의 1 바이트를 가리킴
  u_char key = *(index+3);				// 마지막 옥텟의 1바트는 해쉬 인덱스 값 - ip는 네트워크 바이트 order임
	BiTree  *tree = &(o[key].tree);	// 네트워크 오브젝트 배열의 위치를 찾음
	
	
	// malloc data value is no
	u_long * num = (u_long *)malloc(sizeof(u_long));
	memcpy(num, &no, sizeof(u_long));
	
	// AVL Tree insert 'NO' value
	if ( bistree_insert(tree, num) != 0 )
	{
		printf("%s_OBJ_insert ERROR, INDEX:%d, NUM:%u\n",ty,key,*num);
		return -1;
	}
}


//
// 네트워크 오브젝트 파일로 부터 내용을 추출하여 
// 네트워크 오브젝트를 구성한다.
//
int OBJ_create
(OBJ *o, char *filepath)
{
	FILE *fp;
	char buf[350];
	char ip[20];
	u_long n, addr;

	// open url file
	if ( (fp = fopen(filepath, "r")) == NULL )
  {
  	printf("%s FILE OPEN ERROR",filepath);
  	return -1;
  }
	
	OBJ_init(o);	// 네트워크 오브젝트 배열 초기화

	while( fgets(buf, sizeof(buf), fp) )
	{
		if(buf != NULL && strcmp(buf, "\r\n") && strcmp(buf, "\n"))
		{
			sscanf(buf, "%u  %s", &n, ip);
			
			// 'url' string length check
			if( strlen(ip) <= 15 )
      {
				// ip를 unsigned long 타입의 정수형으로 변환
        addr = inet_addr(ip);        
        OBJ_insert(addr, n, o);
      }
			else
			{
				puts("function ERROR:URL_OBJ_insert, overstring");
				return -1;
			}
		}
	}
}


//
// 기본(정책) 오브젝트 초기화
//
void BASE_init
(BiTree *b)
{
	bistree_destroy( b );   // tree all node delete
  bistree_init( b , compare_str, NULL);        //compare function
	return;
}


//
// 기본(정책) 오브젝트 구성 
//
int BASE_create
(BiTree *b, char *filepath)
{
	FILE *fp;
	char buf[40];
  u_long n, *num;

  // open url file
  if ( (fp = fopen(filepath, "r")) == NULL )
  {
  	printf("%s FILE OPEN ERROR",filepath);
    return -1;
  }
  
  // 초기화
	BASE_init(b);

	while( fgets(buf, sizeof(buf), fp) )
	{
		if(buf != NULL && strcmp(buf, "\r\n") && strcmp(buf, "\n"))
    {    	
			// 'no' string length check
			if( strlen(buf) <= 32 )
      { 
      	sscanf(buf, "%u", &n);
								
				// malloc data value is no
				num = (u_long *) malloc ( sizeof(u_long) );
			  memcpy( num, &n, sizeof(u_long) );			  
			  
				// num value insert into AVL Tress
				if (bistree_insert(b, num) != 0)
				{
					printf("BASE_ ERROR, num::%u\n",*num);
					return -1;
				}
			}
			else
			{
				puts("function ERROR:BASE_create, overstring no");
				return -1;
			}
		}
	}
}
