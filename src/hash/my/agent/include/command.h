/*
* @ file: command.h
*	brief : 
*/

// MASTER 와 AGENT 통신 규약
#define HELLO_WITH_ID	1	// ID를 포함한 접속 요청 
#define HELLO_NO_ID		2	// ID를 포함하지 않는 접속 요청

#define ACK_WITH_ID		3	// ID를 포함한 요청 응답
#define ACK_OK				4	// ID를 포함하지 않은 접속 응답
#define ACK_INVALID		5	// 잘못된 요청

#define REQ_LIVE			6	// 실시간 로그 보기 요청
#define REQ_POLY			7	// 정책 적용 요청

#define RPL_LIVE			8	// 실시간 로그 보기 응답
#define RPL_POLY			9	// 정책 적용 응답

#define START_OK			10


#define	REQ_NET				11	// 소속 네트워크 호스트 보기 요청
#define RPL_NET				12	// 소속 네트워크 호스트 보기 응답


// WEB 과 MASTER 통신 규약
#define WEB_REQ_LIVE	"LIVE"
#define WEB_REQ_POLY	"POLY"
#define WEB_REQ_NET		"NET"

#define WEB_PRL_LIVE	"RPLY_LIVE"
#define WEB_PRL_POLY	"RPLY_POLY"
#define WEB_PRL_NET		"RPLY_NET"


// WEB과 MASTER와 AGENT 간 통신 구조체
typedef struct command{	
	unsigned char signal;	
	unsigned int data;	
}COMMAND;

//command
COMMAND	recv_cmd,	// 수신 데이터
				send_cmd;	// 발신 데이터


char WEB_COMMAND[8];	// WEB이 보내는 명령 문자열
