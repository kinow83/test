#ifndef _FR_MISSING_H
#define _FR_MISSING_H

/*
 * missing.h	Replacements for functions that are or can be
 *		missing on some platforms.
 *
 * Version:	$Id: missing.h,v 1.1.1.1 2006/02/24 08:09:17 cvs Exp $
 *
 */
#ifndef HAVE_STRNCASECMP
extern int strncasecmp(char *s1, char *s2, int n);
#endif

#ifndef HAVE_STRCASECMP
extern int strcasecmp(char *s1, char *s2);
#endif

#ifndef HAVE_STRSEP
extern char *strsep(char **stringp, const char *delim);
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *localtime_r(const time_t *l_clock, struct tm *result);
#endif

#ifndef HAVE_CTIME_R
char *ctime_r(const time_t *l_clock, char *l_buf);
#endif

#ifdef NEED_DECLARATION_CRYPT
char *crypt(char *key, char *salt);
#endif

#ifdef NEED_DECLARATION_STRNCASECMP
int strncasecmp(char *s1, char *s2, int n);
#endif

#ifdef NEED_DECLARATION_STRCASECMP
int strcasecmp(char *s1, char *s2);
#endif

#ifdef NEED_DECLARATION_INET_ATON
struct in_addr;
int inet_aton(char *cp, struct in_addr *inp);
#endif

#ifdef NEED_DECLARATION_GETHOSTNAME
int gethostname(char *name, int len);
#endif

#ifndef HAVE_SETLINEBUF
#ifdef HAVE_SETVBUF
#define setlinebuf(x) setvbuf(x, NULL, _IOLBF, 0)
#else
#define setlinebuf(x)     0
#endif
#endif

#ifdef NEED_DECLARATION_SETLINEBUF
#define setlinebuf(x)     0
#endif

#ifdef NEED_DECLARATION_GETUSERSHELL
char *getusershell(void);
#endif

#ifdef NEED_DECLARATION_ENDUSERSHELL
void endusershell(void);
#endif

#ifndef INADDR_ANY
#define INADDR_ANY      ((uint32_t) 0x00000000)
#endif

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK ((uint32_t) 0x7f000001) /* Inet 127.0.0.1 */
#endif

#ifndef INADDR_NONE
#define INADDR_NONE     ((uint32_t) 0xffffffff)
#endif

#ifndef INADDRSZ
#define INADDRSZ 4
#endif 

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif 

#ifndef AF_UNSPEC
#define AF_UNSPEC 0
#endif 

#ifndef AF_INET6
#define AF_INET6 10
#endif 

#ifndef IN6ADDRSZ
#define IN6ADDRSZ 16
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif 

#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT    {{{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }}}
#endif

#ifndef IPV6_LOOPBACK_LONG_STRING
#define IPV6_LOOPBACK_LONG_STRING	"0:0:0:0:0:0:0:1"
#endif

#ifndef IPV6_LOOPBACK_SHORT_STRING
#define IPV6_LOOPBACK_SHORT_STRING	"::1"
#endif

#ifndef IN6ADDR_LOOPBACK_INIT
#define IN6ADDR_LOOPBACK_INIT   {{{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }}}
#endif

#ifndef IN6ADDR_NONE_INIT
#define IN6ADDR_NONE_INIT   {{{ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff }}}
#endif

#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a) \
    (((__const uint32_t *) (a))[0] == 0                   \
     && ((__const uint32_t *) (a))[1] == 0                    \
     && ((__const uint32_t *) (a))[2] == 0                    \
     && ((__const uint32_t *) (a))[3] == 0)
#endif

#ifndef IN6_IS_ADDR_LOOPBACK
#define IN6_IS_ADDR_LOOPBACK(a) \
    (((__const uint32_t *) (a))[0] == 0                   \
     && ((__const uint32_t *) (a))[1] == 0                    \
     && ((__const uint32_t *) (a))[2] == 0                    \
     && ((__const uint32_t *) (a))[3] == htonl (1))
#endif

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)
#endif

#ifndef IN6_IS_ADDR_LINKLOCAL
#define IN6_IS_ADDR_LINKLOCAL(a) \
    ((((__const uint32_t *) (a))[0] & htonl (0xffc00000))             \
     == htonl (0xfe800000))
#endif

#ifndef IN6_IS_ADDR_SITELOCAL
#define IN6_IS_ADDR_SITELOCAL(a) \
    ((((__const uint32_t *) (a))[0] & htonl (0xffc00000))             \
     == htonl (0xfec00000))
#endif

#ifndef IN6_IS_ADDR_V4MAPPED
#define IN6_IS_ADDR_V4MAPPED(a) \
    ((((__const uint32_t *) (a))[0] == 0)                     \
     && (((__const uint32_t *) (a))[1] == 0)                  \
     && (((__const uint32_t *) (a))[2] == htonl (0xffff)))
#endif

#ifndef IN6_IS_ADDR_V4COMPAT
#define IN6_IS_ADDR_V4COMPAT(a) \
    ((((__const uint32_t *) (a))[0] == 0)                     \
     && (((__const uint32_t *) (a))[1] == 0)                  \
     && (((__const uint32_t *) (a))[2] == 0)                  \
     && (ntohl (((__const uint32_t *) (a))[3]) > 1))
#endif

#ifndef IN6_IS_ADDR_NONE
#define IN6_IS_ADDR_NONE(a) \
    ((((__const uint32_t *) (a))[0] == 0xffffffff)                     \
     && (((__const uint32_t *) (a))[1] == 0xffffffff)                  \
     && (((__const uint32_t *) (a))[2] == 0xffffffff)                  \
     && (((__const uint32_t *) (a))[3] == 0xffffffff))
#endif

#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL(a,b) \
    ((((__const uint32_t *) (a))[0] == ((__const uint32_t *) (b))[0])     \
     && (((__const uint32_t *) (a))[1] == ((__const uint32_t *) (b))[1])  \
     && (((__const uint32_t *) (a))[2] == ((__const uint32_t *) (b))[2])  \
     && (((__const uint32_t *) (a))[3] == ((__const uint32_t *) (b))[3]))
#endif

#endif /* _FR_MISSING_H */
