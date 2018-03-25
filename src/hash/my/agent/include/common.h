/*
* @ file : common.h
*	brief : 
*/

#define MAX_AGENT	30	// 에이전트 최대 수
#define moduler 31		// trans_time 계산시 moduler 값
#define LOGFILE	"LOG_FILE"
#define REFRESH_TIME 12	// 에이전트가 마스터에 접속이 연속적으로 REFRESH_TIME 이상 이루어지면
												// 마스터와 연결이 끊어졌음을 의미 한다.


int alive_value=1;			// alive_value가 1, -1, 1, -1 ... 번갈아 가면서 에이전트가 살아있음을 실시간으로 디비에 기록한다.
int alive_interval = 1; // alive 주기적인 인터벌 시간
int log_send_sec = 10;	// LOG 파일을 보내는 초 시간

enum {START, STOP} STATUS;	// 쓰레드 상태
enum {F_START, F_STOP} FAKE;// 차단 프로세스 상태

int trans_time;		//전송 할 시간(sec) : 계산법 = agent 번호 % moduler
int agent_num=0;	//에이전트 ID

int command_sock;
