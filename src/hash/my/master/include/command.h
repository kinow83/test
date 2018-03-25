
/*
* @ file : command.c
*	brief : 
*/

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

#define	REQ_REV				13	// 이전 백업 로그 보기 요청
#define RPL_REV				14	// 이전 백업 로그 보기 응답

#define REQ_REVTIME				15	// 이전 백업 로그 보기 소요 시간 요청
#define RPL_REVTIME				16	// 이전 백업 로그 보기 소요 시간 응답

// 웹에서 MASTER로 보내는 시그널(문자형)
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


// 마스터와 에이전트 간 통신을 위한 명령어 구조체
typedef struct command{
	unsigned char signal;	
	unsigned int data;
	char dump[10];
}COMMAND;

// 웹과 마스터 간 통신을 위한 변수(문자형)
char WEB_COMMAND[8];

