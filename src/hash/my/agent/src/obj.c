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
//  AVL Ʈ�� ���� �ڽ�, ������ �ڽ� �� �Լ�
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
//�� �������Ʈ  : ������Ʈ ��Ʈ��ũ�� ���� ȣ��Ʈ���� ���� ��å�� ������ ȣ��Ʈ ���� �Ǵ�
//							 : ���� ��å�� ������ ȣ��Ʈ ���� �Ǵ��ϴ� ������ ���� ��å�� �������� ���� ȣ��Ʈ�� 
//							 : ��Ʈ��ũ ������Ʈ(���� ��å)�� Ž���� �ʿ䰡 ����, �⺻ ��å ������ ������ �����̴�.
//

// �׷� ������Ʈ �ʱ�ȭ
// �迭 ũ��(OBJ_SIZE) = 256 : C CLASS ũ��
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
// ��Ʈ��ũ ������Ʈ �ʱ�ȭ
// ��Ʈ��ũ ������Ʈ�� 256���� AVL Ʈ�� �迭�� �̷�����ִ�.
// ���� ���,
// ������Ʈ �迭[10]��  ������ xxx.xxx.xxx.10 �� ȣ��Ʈ�� ��å ������ ����� ������
// ��å ����(AVL Ʈ�� ����)�� url �Ǵ� ip �� ������ȣ��� �̷���� �ִ�.
//
// ��Ŷ ĸ�� ��� source ip �� xxx.xxx.xxx.10 �� ��Ŷ�� ��û�������� �˻��Ͽ� ���� ��å�� �ɸ��� �Ǹ�
// ���� �������� xxx.xxx.xxx.10(source ip)�� ������ �ȴ�.
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
	u_char * index = (u_char *)&p;	// ip�� ������ ������ 1 ����Ʈ�� ����Ŵ
  u_char key = *(index+3);				// ������ ������ 1��Ʈ�� �ؽ� �ε��� �� - ip�� ��Ʈ��ũ ����Ʈ order��
	BiTree  *tree = &(o[key].tree);	// ��Ʈ��ũ ������Ʈ �迭�� ��ġ�� ã��
	
	
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
// ��Ʈ��ũ ������Ʈ ���Ϸ� ���� ������ �����Ͽ� 
// ��Ʈ��ũ ������Ʈ�� �����Ѵ�.
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
	
	OBJ_init(o);	// ��Ʈ��ũ ������Ʈ �迭 �ʱ�ȭ

	while( fgets(buf, sizeof(buf), fp) )
	{
		if(buf != NULL && strcmp(buf, "\r\n") && strcmp(buf, "\n"))
		{
			sscanf(buf, "%u  %s", &n, ip);
			
			// 'url' string length check
			if( strlen(ip) <= 15 )
      {
				// ip�� unsigned long Ÿ���� ���������� ��ȯ
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
// �⺻(��å) ������Ʈ �ʱ�ȭ
//
void BASE_init
(BiTree *b)
{
	bistree_destroy( b );   // tree all node delete
  bistree_init( b , compare_str, NULL);        //compare function
	return;
}


//
// �⺻(��å) ������Ʈ ���� 
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
  
  // �ʱ�ȭ
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
