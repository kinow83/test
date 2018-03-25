/*
* @ file : time.h
*	brief  : 실시간 시간 갱신 
*/

#include <time.h>

// Is this year  Leap year ?
// 윤년인지 판단
#define isLeapYear(x)	(\
				(\
					( (x) % 4 == 0 ) && \
					( (x) % 100 != 0 ) \
				)|| \
				( (x) % 400 == 0 ) \
			)



// 시간 구조체
typedef struct time_zone{
	
	time_t t;
	struct tm *tm;
	struct tm *pre_tm;
	
	// current time - 현재 시간
	u_short	year, mon, mday;
	u_short wday;
	u_short hour, min, sec;	
	
	// 1hour ago -  한시간 이전 시간
	u_short	pre_year, pre_mon, pre_mday;	
	u_short pre_hour;
	
}TIME_ZONE;


/*
*/
TIME_ZONE tz;
/*
*/


// 시간 갱신
void get_localtime(void);


void get_localtime(void){
	
	time(&tz.t);
	tz.tm = localtime(&tz.t);	
	
	// 현재 시간
	tz.year = tz.tm->tm_year + 1900;	// 2008, 2009....
	tz.mon = tz.tm->tm_mon;						//[0, 11]
	tz.mday = tz.tm->tm_mday;					//[1,31]	
	tz.wday = tz.tm->tm_wday;					//[0,6]	
	tz.hour = tz.tm->tm_hour;					//[0,23]
	tz.min = tz.tm->tm_min;						//[0,59]
	tz.sec = tz.tm->tm_sec;						//[0,59]
	
	// 한 시간 이전 시간
	tz.t -= 60*60;
	tz.tm = localtime(&tz.t);
	
	tz.pre_year = tz.tm->tm_year + 1900;	// 2008, 2009....
	tz.pre_mon = tz.tm->tm_mon;						//[0, 11]
	tz.pre_mday = tz.tm->tm_mday;					//[1,31]	
	tz.pre_hour = tz.tm->tm_hour;					//[0,23]	
}

