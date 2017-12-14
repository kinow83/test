#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

main(int argc, const char **argv)
{
    struct hostent ret;
    struct hostent *hinfo = NULL;
    char **pptr = NULL;
    const char *ptr = argv[1];
    char tmpbuf[1024]={0,};
    int herr;
    memset(&ret , 0x00 , sizeof(struct hostent));

    printf("query %s\n", ptr);
    if (gethostbyname_r(ptr, &ret, tmpbuf, sizeof(tmpbuf), &hinfo, &herr)){
        printf("error - gethostbyname_r\n");
        perror("gethostbyname_r");
        return -1;
    }
    if (ret.h_addr_list == NULL){
        printf("ret.h_addr_list == NULL\n");
        return -1;
    }
    printf("ret.h_addr_list != NULL\n");

    for (pptr = ret.h_addr_list ; *pptr != NULL ; pptr++){
        printf("resovled: %s\n", ret.h_name);
    }

    printf("finished\n");
    return 0;
}
