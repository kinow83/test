
/*
* @ file : command.c
*	brief : 
*/

#define HELLO_WITH_ID	1	// ID�� ������ ���� ��û 
#define HELLO_NO_ID		2	// ID�� �������� �ʴ� ���� ��û

#define ACK_WITH_ID		3	// ID�� ������ ��û ����
#define ACK_OK				4	// ID�� �������� ���� ���� ����
#define ACK_INVALID		5	// �߸��� ��û

#define REQ_LIVE			6	// �ǽð� �α� ���� ��û
#define REQ_POLY			7	// ��å ���� ��û

#define RPL_LIVE			8	// �ǽð� �α� ���� ����
#define RPL_POLY			9	// ��å ���� ����

#define START_OK			10

#define	REQ_NET				11	// �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ��û
#define RPL_NET				12	// �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ����

#define	REQ_REV				13	// ���� ��� �α� ���� ��û
#define RPL_REV				14	// ���� ��� �α� ���� ����

#define REQ_REVTIME				15	// ���� ��� �α� ���� �ҿ� �ð� ��û
#define RPL_REVTIME				16	// ���� ��� �α� ���� �ҿ� �ð� ����

// ������ MASTER�� ������ �ñ׳�(������)
#define WEB_REQ_LIVE	"LIVE"
#define WEB_REQ_POLY	"POLY"
#define WEB_REQ_NET		"NET"
#define WEB_REQ_REV		"REV"
#define WEB_REQ_REVTIME		"TIME"

#define WEB_PRL_LIVE			"RPLY_LIVE"
#define WEB_PRL_POLY			"RPLY_POLY"
#define WEB_PRL_NET				"RPLY_NET"
#define WEB_PRL_REV				"RPLY_REV"
#define WEB_PRL_REVTIME		"RPLY_TIME"


// �����Ϳ� ������Ʈ �� ����� ���� ��ɾ� ����ü
typedef struct command{
	unsigned char signal;	
	unsigned int data;
	char dump[10];
}COMMAND;

// ���� ������ �� ����� ���� ����(������)
char WEB_COMMAND[8];

