/*
* @ file  : obj.h
* @ brief :
*/
#include "../include/bistree.h"

#define OBJ_SIZE	256

////////////////////////////////////////////////
// ��Ʈ��ũ ��ü �ؽ� ���̺�
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


int compare_str(const void *str1, const void *str2);	// AVL ���Խ� �����ڽ�, ������ �ڽ� �Ǵ� �Լ�
void OBJ_group_init(void);										// �׷� ������Ʈ �ʱ�ȭ
void OBJ_init(OBJ * o);												// ��Ʈ��ũ ������Ʈ �ʱ�ȭ
int OBJ_insert(u_long p, u_long no, OBJ *o);	// ��Ʈ��ũ ������Ʈ �迭�� ��Ʈ��ũ ������Ʈ ����
int OBJ_create(OBJ *o, char *filepath);				// ��Ʈ��ũ ������Ʈ �����


void BASE_init(BiTree *b);										// �⺻(��å) ������Ʈ �ʱ�ȭ
int BASE_create(BiTree *b, char *filepath);		// �⺻(��å) ������Ʈ �����



