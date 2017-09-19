
#ifndef __location__

	#define __D_STRING_LINE1__(s) #s
	#define __D_STRING_LINE2__(s) __D_STRING_LINE1__(s)
	#define __D_STRING_LINE3__    __D_STRING_LINE2__(__LINE__)
	#define __location__ __FILE__ ":" __D_STRING_LINE3__

#endif
