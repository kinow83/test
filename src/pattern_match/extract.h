
#ifndef __EXTRACT__
#define __EXTRACT__


#define 20MB (1024*1024*20)

size_t readfile(const char *file, char *buf, size_t buflen);
char *rseek(char *buf, size_t pos, size_t len);

#endif
