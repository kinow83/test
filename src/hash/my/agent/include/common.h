/*
* @ file : common.h
*	brief : 
*/

#define MAX_AGENT	30	// ������Ʈ �ִ� ��
#define moduler 31		// trans_time ���� moduler ��
#define LOGFILE	"LOG_FILE"
#define REFRESH_TIME 12	// ������Ʈ�� �����Ϳ� ������ ���������� REFRESH_TIME �̻� �̷������
												// �����Ϳ� ������ ���������� �ǹ� �Ѵ�.


int alive_value=1;			// alive_value�� 1, -1, 1, -1 ... ������ ���鼭 ������Ʈ�� ��������� �ǽð����� ��� ����Ѵ�.
int alive_interval = 1; // alive �ֱ����� ���͹� �ð�
int log_send_sec = 10;	// LOG ������ ������ �� �ð�

enum {START, STOP} STATUS;	// ������ ����
enum {F_START, F_STOP} FAKE;// ���� ���μ��� ����

int trans_time;		//���� �� �ð�(sec) : ���� = agent ��ȣ % moduler
int agent_num=0;	//������Ʈ ID

int command_sock;
