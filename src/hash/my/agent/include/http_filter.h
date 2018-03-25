/*
*	@ file : http_filter.h
*/

#define IS_GET	1	// GET 메소드
#define IS_POST	2	// POST 메소드

// GET 메소드 인지, POST 메소드 인지 판단
int type_request(char * hp){
	
	if( *(hp) == 'G' && *(hp+1) == 'E' && *(hp+2) == 'T' )
	{
		hp += 3;
		return IS_GET;
	}
	else if( *(hp) == 'P' && *(hp+1) == 'O' && *(hp+2) == 'S' && *(hp+3) == 'T'){
		hp += 4;
		return IS_POST;
	}
	return -1;	
}

