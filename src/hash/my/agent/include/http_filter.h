/*
*	@ file : http_filter.h
*/

#define IS_GET	1	// GET �޼ҵ�
#define IS_POST	2	// POST �޼ҵ�

// GET �޼ҵ� ����, POST �޼ҵ� ���� �Ǵ�
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

