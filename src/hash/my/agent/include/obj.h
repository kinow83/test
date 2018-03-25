/*
* @ file  : obj.h
* @ brief :
*/
#include "../include/bistree.h"

#define OBJ_SIZE	256

////////////////////////////////////////////////
// 네트워크 객체 해쉬 테이블
////////////////////////////////////////////////
typedef struct _obj{
	BiTree	tree;
	struct _obj *next;
}OBJ;


OBJ	url_obj[OBJ_SIZE];	// url network object
OBJ	ip_obj[OBJ_SIZE];	// ip network object
OBJ	key_obj[OBJ_SIZE];	// keyword network object

BiTree  url_base;       // base("0.0.0.0") url network AVL TREE
BiTree  ip_base;        // base("0.0.0.0") ip network AVL TREE
BiTree  key_base;       // base("0.0.0.0") keyword network AVL TREE


int url_obj_group[OBJ_SIZE];	// Applied url network's STATE
int ip_obj_group[OBJ_SIZE];		// Applied ip network's STATE
int key_obj_group[OBJ_SIZE];	// Applied keyword network's STATE


int compare_str(const void *str1, const void *str2);	// AVL 삽입시 왼쪽자식, 오른쪽 자식 판단 함수
void OBJ_group_init(void);										// 그룹 오브젝트 초기화
void OBJ_init(OBJ * o);												// 네트워크 오브젝트 초기화
int OBJ_insert(u_long p, u_long no, OBJ *o);	// 네트워크 오브젝트 배열에 네트워크 오브젝트 삽입
int OBJ_create(OBJ *o, char *filepath);				// 네트워크 오브젝트 만들기


void BASE_init(BiTree *b);										// 기본(정책) 오브젝트 초기화
int BASE_create(BiTree *b, char *filepath);		// 기본(정책) 오브젝트 만들기



