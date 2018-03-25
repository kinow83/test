/*
* @file: db.h
*/
#include <mysql.h>


/*
* @ information database
*/
#define AGENT_INFORMATION "information"
#define AGENT_INFORMATION_MASTER	AGENT_INFORMATION".member"
#define AGENT_INFORMATION_STATUS	AGENT_INFORMATION".status"
#define CREATE_INFORMATION "create database "AGENT_INFORMATION

/*
* @ statics database
*/
#define STATICS "statics"
#define STATICS_AGENT STATICS".agent"
#define DB_STATICS "create database "STATICS
#define POLICY	"policy"
#define POLICY_url			POLICY".url"
#define POLICY_ip				POLICY".ip"
#define POLICY_keyword	POLICY".keyword"
#define POLICY_agent		POLICY".agent"

/*
* @ agent database
*/
#define AGENT "agent"
#define LIVE "live"
	
int query_invalied(MYSQL con, int query_stat);

int query_invalied(MYSQL con, int query_stat)
{

	if (query_stat != 0){
		printf("%s\n", mysql_error(&con));
		return -1;
	}
	return 0;
}
