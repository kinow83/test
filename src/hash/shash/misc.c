/*
 * misc.c	Various miscellaneous functions.
 *
 * Version:	$Id: misc.c,v 1.2 2006/03/30 01:30:52 cvs Exp $
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Copyright 2000  The FreeRADIUS server project
 */

static const char rcsid[] =
"$Id: misc.c,v 1.2 2006/03/30 01:30:52 cvs Exp $";

#include	"autoconf.h"

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<ctype.h>
#include	<sys/file.h>
#include	<fcntl.h>
#include	<unistd.h>

#include	"libradius.h"
#include	"missing.h"

int		librad_dodns = 0;
int		librad_debug = 0;

#define BYTE	char
/*
 *	Return a printable host name (or IP address in dot notation)
 *	for the supplied IP address.
 */
char * ip_hostname(char *buf, size_t buflen, uint32_t ipaddr)
{
	struct		hostent *hp;
#ifdef GETHOSTBYADDRRSTYLE
#if (GETHOSTBYADDRRSTYLE == SYSVSTYLE) || (GETHOSTBYADDRRSTYLE == GNUSTYLE)
	BYTE buffer[2048];
	struct hostent result;
	int error;
#endif
#endif

	/*
	 *	No DNS: don't look up host names
	 */
	if (librad_dodns == 0) {
		ip_ntoa(buf, ipaddr);
		return buf;
	}

#ifdef GETHOSTBYADDRRSTYLE
#if GETHOSTBYADDRRSTYLE == SYSVSTYLE
	hp = gethostbyaddr_r((char *)&ipaddr, sizeof(struct in_addr), AF_INET, &result, buffer, sizeof(buffer), &error);
#elif GETHOSTBYADDRRSTYLE == GNUSTYLE
	if (gethostbyaddr_r((char *)&ipaddr, sizeof(struct in_addr),
			    AF_INET, &result, buffer, sizeof(buffer),
			    &hp, &error) != 0) {
		hp = NULL;
	}
#else
	hp = gethostbyaddr((char *)&ipaddr, sizeof(struct in_addr), AF_INET);
#endif
#else
	hp = gethostbyaddr((char *)&ipaddr, sizeof(struct in_addr), AF_INET);
#endif
	if ((hp == NULL) ||
	    (strlen((char *)hp->h_name) >= buflen)) {
		ip_ntoa(buf, ipaddr);
		return buf;
	}

	strNcpy(buf, (char *)hp->h_name, buflen);
	return buf;
}


/*
 *	Return an IP address from a host
 *	name or address in dot notation.
 */
uint32_t ip_getaddr(const char *host)
{
	struct hostent	*hp;
	uint32_t	 a;
#ifdef GETHOSTBYNAMERSTYLE
#if (GETHOSTBYNAMERSTYLE == SYSVSTYLE) || (GETHOSTBYNAMERSTYLE == GNUSTYLE)
	struct hostent result;
	int error;
	BYTE buffer[2048];
#endif
#endif

	if ((a = ip_addr(host)) != htonl(INADDR_NONE))
		return a;

#ifdef GETHOSTBYNAMERSTYLE
#if GETHOSTBYNAMERSTYLE == SYSVSTYLE
	hp = gethostbyname_r(host, &result, buffer, sizeof(buffer), &error);
#elif GETHOSTBYNAMERSTYLE == GNUSTYLE
	if (gethostbyname_r(host, &result, buffer, sizeof(buffer),
			    &hp, &error) != 0) {
		return htonl(INADDR_NONE);
	}
#else
	hp = gethostbyname(host);
#endif
#else
	hp = gethostbyname(host);
#endif
	if (hp == NULL) {
		return htonl(INADDR_NONE);
	}

	/*
	 *	Paranoia from a Bind vulnerability.  An attacker
	 *	can manipulate DNS entries to change the length of the
	 *	address.  If the length isn't 4, something's wrong.
	 */
	if (hp->h_length != 4) {
		return htonl(INADDR_NONE);
	}

	memcpy(&a, hp->h_addr, sizeof(uint32_t));
	return a;
}


/*
 *	Return an IP address in standard dot notation
 */
char *ip_ntoa(char *buffer, uint32_t ipaddr)
{
	ipaddr = ntohl(ipaddr);

	sprintf(buffer, "%d.%d.%d.%d",
		(ipaddr >> 24) & 0xff,
		(ipaddr >> 16) & 0xff,
		(ipaddr >>  8) & 0xff,
		(ipaddr      ) & 0xff);
	return buffer;
}


/*
 *	Return an IP address from
 *	one supplied in standard dot notation.
 */
uint32_t ip_addr(const char *ip_str)
{
	struct in_addr	in;

	if (inet_aton(ip_str, &in) == 0)
		return htonl(INADDR_NONE);
	return in.s_addr;
}


/*
 *	Like strncpy, but always adds \0
 */
char *strNcpy(char *dest, const char *src, int n)
{
	char *p = dest;

	while ((n > 1) && (*src)) {
		*(p++) = *(src++);

		n--;
	}
	*p = '\0';

	return dest;
}

/*
 * Lowercase a string
 */
void rad_lowercase(char *str) {
	char *s;

	for (s=str; *s; s++)
		if (isupper((int) *s)) *s = tolower((int) *s);
}

/*
 * Remove spaces from a string
 */
void rad_rmspace(char *str) {
	char *s = str;
	char *ptr = str;

  while(ptr && *ptr!='\0') {
    while(isspace((int) *ptr))
      ptr++;
    *s = *ptr;
    ptr++;
    s++;
  }
  *s = '\0';
}

/*
 *	Internal wrapper for locking, to minimize the number of ifdef's
 *
 *	Lock an fd, prefer lockf() over flock()
 */
int rad_lockfd(int fd, int lock_len)
{
#if defined(F_LOCK) && !defined(BSD)
	return lockf(fd, F_LOCK, lock_len);
#elif defined(LOCK_EX)
	return flock(fd, LOCK_EX);
#else
	struct flock fl;
	fl.l_start = 0;
	fl.l_len = lock_len;
	fl.l_pid = getpid();
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_CUR;
	return fcntl(fd, F_SETLKW, (void *)&fl);
#endif
}

/*
 *	Internal wrapper for locking, to minimize the number of ifdef's
 *
 *	Lock an fd, prefer lockf() over flock()
 *	Nonblocking version.
 */
int rad_lockfd_nonblock(int fd, int lock_len)
{
#if defined(F_LOCK) && !defined(BSD)
	return lockf(fd, F_TLOCK, lock_len);
#elif defined(LOCK_EX)
	return flock(fd, LOCK_EX | LOCK_NB);
#else
	struct flock fl;
	fl.l_start = 0;
	fl.l_len = lock_len;
	fl.l_pid = getpid();
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_CUR;
	return fcntl(fd, F_SETLK, (void *)&fl);
#endif
}

/*
 *	Internal wrapper for unlocking, to minimize the number of ifdef's
 *	in the source.
 *
 *	Unlock an fd, prefer lockf() over flock()
 */
int rad_unlockfd(int fd, int lock_len)
{
#if defined(F_LOCK) && !defined(BSD)
	return lockf(fd, F_ULOCK, lock_len);
#elif defined(LOCK_EX)
	return flock(fd, LOCK_UN);
#else
	struct flock fl;
	fl.l_start = 0;
	fl.l_len = lock_len;
	fl.l_pid = getpid();
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_CUR;
	return fcntl(fd, F_UNLCK, (void *)&fl);
#endif
}

/*
 *	Return an interface-id in standard colon notation
 */
char *ifid_ntoa(char *buffer, size_t size, uint8_t *ifid)
{
	snprintf(buffer, size, "%x:%x:%x:%x",
		 (ifid[0] << 8) + ifid[1], (ifid[2] << 8) + ifid[3],
		 (ifid[4] << 8) + ifid[5], (ifid[6] << 8) + ifid[7]);
	return buffer;
}


/*
 *	Return an interface-id from
 *	one supplied in standard colon notation.
 */
uint8_t *ifid_aton(const char *ifid_str, uint8_t *ifid)
{
	static const char xdigits[] = "0123456789abcdef";
	const char *p, *pch;
	int num_id = 0, val = 0, idx = 0;

	for (p = ifid_str; ; ++p) {
		if (*p == ':' || *p == '\0') {
			if (num_id <= 0)
				return NULL;

			/*
			 *	Drop 'val' into the array.
			 */
			ifid[idx] = (val >> 8) & 0xff;
			ifid[idx + 1] = val & 0xff;
			if (*p == '\0') {
				/*
				 *	Must have all entries before
				 *	end of the string.
				 */
				if (idx != 6)
					return NULL;
				break;
			}
			val = 0;
			num_id = 0;
			if ((idx += 2) > 6)
				return NULL;
		} else if ((pch = strchr(xdigits, tolower(*p))) != NULL) {
			if (++num_id > 4)
				return NULL;
			/*
			 *	Dumb version of 'scanf'
			 */
			val <<= 4;
			val |= (pch - xdigits);
		} else
			return NULL;
	}
	return ifid;
}
/*
 *	Return an IPv6 address in standard colon notation
 */
const char *ipv6_ntoa(char *buffer, size_t size, void *ip6addr)
{
#if 0//defined(HAVE_INET_NTOP) && defined(AF_INET6)
	return inet_ntop(AF_INET6, (struct in6_addr *) ip6addr, buffer, size);
#else
	/*
	 *	Do it really stupidly.
	 *  The canonical form is represented as: x:x:x:x:x:x:x:x, each ¡°x¡± being a 16 Bit hex value. 
	 */
	snprintf(buffer, size, "%x:%x:%x:%x:%x:%x:%x:%x",
		 (((uint8_t *) ip6addr)[0] << 8) | ((uint8_t *) ip6addr)[1],
		 (((uint8_t *) ip6addr)[2] << 8) | ((uint8_t *) ip6addr)[3],
		 (((uint8_t *) ip6addr)[4] << 8) | ((uint8_t *) ip6addr)[5],
		 (((uint8_t *) ip6addr)[6] << 8) | ((uint8_t *) ip6addr)[7],
		 (((uint8_t *) ip6addr)[8] << 8) | ((uint8_t *) ip6addr)[9],
		 (((uint8_t *) ip6addr)[10] << 8) | ((uint8_t *) ip6addr)[11],
		 (((uint8_t *) ip6addr)[12] << 8) | ((uint8_t *) ip6addr)[13],
		 (((uint8_t *) ip6addr)[14] << 8) | ((uint8_t *) ip6addr)[15]);
	return buffer;
#endif
}


/*
 *	Return an IPv6 address from
 *	one supplied in standard colon notation.
 */
int ipv6_addr(const char *ip6_str, void *ip6addr)
{
#if 0	//defined(HAVE_INET_PTON) && defined(AF_INET6)
	if (inet_pton(AF_INET6, ip6_str, (struct in6_addr *) ip6addr) != 1)
		return -1;
#else
	/*
	 *	Copied from the 'ifid' code above, with minor edits.
	 */
	static const BYTE xdigits[] = "0123456789abcdef";
	const char *p, *pch;
	int num_id = 0, val = 0, idx = 0;
	uint8_t *addr = ip6addr;

	for (p = ip6_str; ; ++p) {
		if (*p == ':' || *p == '\0') {
			if (num_id <= 0)
				return -1;

			/*
			 *	Drop 'val' into the array.
			 */
			addr[idx] = (val >> 8) & 0xff;
			addr[idx + 1] = val & 0xff;
			if (*p == '\0') {
				/*
				 *	Must have all entries before
				 *	end of the string.
				 */
				if (idx != 14)
					return -1;
				break;
			}
			val = 0;
			num_id = 0;
			if ((idx += 2) > 14)
				return -1;
		} else if ((pch = strchr(xdigits, tolower(*p))) != NULL) {
			if (++num_id > 8) /* no more than 8 16-bit numbers */
				return -1;
			/*
			 *	Dumb version of 'scanf'
			 */
			val <<= 4;
			val |= (pch - xdigits);
		} else
			return -1;
	}
#endif
	return 0;
}

/**
 *	Return normalized MAC string of form "11:22:33:44:55:66"
 *	added by syyang
 */
int normalize_mac(const char *mac, char *normal)
{
	int ch;
	unsigned int i, j = 0;
	BYTE tmp[32];

	memset (tmp, 0, sizeof(tmp));
	for (i = 0; i < strlen (mac); i++) {
		ch = mac[i];
        if ((ch>='0'&&ch<='9') || (ch>='A'&&ch<='Z') || (ch>='a'&&ch<='z'))
            tmp[j++]=ch;
		if (i > 20)
			return -1;
	}

	/* upper case tmp */
	for (i = 0; i < strlen (tmp); i++) {
		tmp[i] = toupper(tmp[i]);
	}

	sprintf(normal, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
		tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],
		tmp[8],tmp[9],tmp[10],tmp[11]);
	return 0;
}

static const char *hextab = "0123456789abcdef";
void lrad_bin2hex(const uint8_t *bin, char *hex, size_t len)
{
	size_t	i;

	for(i = 0; i < len; i++)
	{
		hex[0] = hextab[((*bin) >> 4) & 0x0f];
		hex[1] = hextab[*bin & 0x0f];
		hex += 2;
		bin++;
	}

	*hex = '\0';

	return;
}

/*
 * check ipv4 address validation
 * added by kinow
 */
int valid_ipv4address(const char * ip)
{
#if 0	// Delete it, Thread-Unsafe
        int tok_cnt = 0;
        BYTE check_ip[20]={0};
        char *ipch;

        if (!ip) return 0;

		snprintf(check_ip, sizeof(check_ip), "%s", ip);

        ipch = strtok(check_ip, ".");
        while (ipch != NULL) {
                tok_cnt++;
                ipch = strtok(NULL, ".");
        }
        if (tok_cnt == 4) // 4 octec
			return 1;
        return 0;
#endif
	struct sockaddr_in sa;
	int ret = inet_pton(AF_INET, ip, &(sa.sin_addr));
	return (ret != 0);
	
}

/*
 * Check IP Address validation, thread safe
 * af : IP Address Family
 */
int valid_ipaddress(int af, const char * ip)
{
	BYTE check_ip[256] = {0};
	char *p;
	int prefix;

	if (af != AF_INET && af != AF_INET6) return 0;

	memcpy(check_ip, ip, strlen(ip));

	/* Remove Prefix */
	p = strchr(check_ip, '/');
	if (p) {
		*p++ = '\0';
		prefix = atoi(p);

		if (prefix < 0)	
			return 0;
		else if (af == AF_INET && prefix > 32)
			return 0;
		else if(af == AF_INET6 && prefix > 128)
			return 0;
	}
	
	if (af == AF_INET) {
        struct sockaddr_in sa;
        return inet_pton(af, check_ip, &(sa.sin_addr));
    } else {
        struct sockaddr_in6 sa;
        return inet_pton(af, check_ip, &(sa.sin6_addr));
    }
	/* return (ret != 0); */
}

int valid_ipv6address(const char * ip)
{
#if 0	// thread unsafe code
        int tok_cnt = 0;
        BYTE check_ip[128]={0};
        char *ipch;

        if (!ip) return 0;

		snprintf(check_ip, sizeof(check_ip), "%s", ip);

        ipch = strtok(check_ip, ":");
        while (ipch != NULL) {
                tok_cnt++;
                ipch = strtok(NULL, ":");
        }
        // 0:0:0:0:0:0:0:0 or ::1
        if (tok_cnt > 1 && tok_cnt < 8)
            return 1;
        return 0;
#endif
	struct sockaddr_in6 sa;
	int ret = inet_pton(AF_INET6, ip, &(sa.sin6_addr));
	return (ret != 0);
}

/*
 * check mac address validation
 * added by kinow
 */
int valid_macaddress(const char *mac)
{
        BYTE tok_cnt = 0;
        BYTE check_mac[18]={0};
        char *macch;

        if(!mac || strlen(mac) != 17) return 0;
		snprintf(check_mac, sizeof(check_mac), "%s", mac);

        macch = strtok(check_mac, ":");
        while (macch != NULL) {
                tok_cnt++;
                macch = strtok(NULL, ":");
        }
        if (tok_cnt == 6) return 1;
        return 0;
}


/*
 *  Wrappers for IPv4/IPv6 host to IP address lookup.
 *  This API returns only one IP address, of the specified
 *  address family, or the first address (of whatever family),
 *  if AF_UNSPEC is used.
 */
int ip_hton(const char *src, int af, fr_ipaddr_t *dst)
{
    int rcode;
    struct addrinfo hints, *ai = NULL, *res = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;

    if ((rcode = getaddrinfo(src, NULL, &hints, &res)) != 0) {
        return -1;
    }

    for (ai = res; ai; ai = ai->ai_next) {
        if ((af == ai->ai_family) || (af == AF_UNSPEC))
            break;
    }

    if (!ai) {
        /* ip_hton failed to find requested information for host */
        freeaddrinfo(ai);
        return -1;
    }

    rcode = fr_sockaddr2ipaddr((struct sockaddr_storage *)ai->ai_addr,
                   ai->ai_addrlen, dst, NULL);
    freeaddrinfo(ai);
    if (!rcode) return -1;

    return 0;
}

/*
 *  Look IP addreses up, and print names (depending on DNS config)
 */
const char *ip_ntoh(const fr_ipaddr_t *src, char *dst, size_t cnt)
{
#if 0
    struct sockaddr ss;
    int error;
    socklen_t salen;

    /*
     *  No DNS lookups
     */
    if (!librad_dodns) {
        return inet_ntop(src->af, &(src->ipaddr), dst, cnt);
    }

    if (!fr_ipaddr2sockaddr(src, 0, &ss, &salen)) {
        return NULL;
    }

    if ((error = getnameinfo((struct sockaddr_storage *)&ss, salen, dst, cnt, NULL, 0,
                 NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
        return NULL;
    }
    return dst;
#endif

	/* this ip address must be canonical */
	switch (src->af) {
	case AF_INET:
		return ip_ntoa(dst, src->fr_ip4addr.s_addr);
		break;
	case AF_INET6:
		return ipv6_ntoa(dst, cnt, (void *)src->fr_ip6addr.s6_addr);
		break;
	}
	return dst;
}

int fr_ipaddr_cmp(const fr_ipaddr_t *a, const fr_ipaddr_t *b)
{
#if 0
	BYTE c[128] = {0};
	printf("fr_debug: fr_ipaddr_cmp> [a] ip:%s,\tscope:%d,\taf:%d\n",
			ip_ntoh(a, c, 128), a->scope, a->af);
	printf("fr_debug: fr_ipaddr_cmp> [b] ip:%s,\tscope:%d,\taf:%d\n",
			ip_ntoh(b, c, 128), b->scope, b->af);
#endif

    if (a->af < b->af) return -1;
    if (a->af > b->af) return +1;

    switch (a->af) {
    case AF_INET:
        return memcmp(&a->ipaddr.ip4addr,
                  &b->ipaddr.ip4addr,
                  sizeof(a->ipaddr.ip4addr));
        break;

    case AF_INET6:
        if (a->scope < b->scope) return -1;
        if (a->scope > b->scope) return +1;

        return memcmp(&a->ipaddr.ip6addr,
                  &b->ipaddr.ip6addr,
                  sizeof(a->ipaddr.ip6addr));
        break;

    default:
        break;
    }

    return -1;
}

int fr_ipaddr2sockaddr(const fr_ipaddr_t *ipaddr, int port,
               struct sockaddr_storage *sa, socklen_t *salen)
{
    if (ipaddr->af == AF_INET) {
        struct sockaddr_in s4;

        *salen = sizeof(s4);

        memset(&s4, 0, sizeof(s4));
        s4.sin_family = AF_INET;
        s4.sin_addr = ipaddr->ipaddr.ip4addr;
        s4.sin_port = htons(port);
        memset(sa, 0, sizeof(*sa));
        memcpy(sa, &s4, sizeof(s4));

    } else if (ipaddr->af == AF_INET6) {
        struct sockaddr_in6 s6;

        *salen = sizeof(s6);

        memset(&s6, 0, sizeof(s6));
        s6.sin6_family = AF_INET6;
        s6.sin6_addr = ipaddr->ipaddr.ip6addr;
        s6.sin6_port = htons(port);
        s6.sin6_scope_id = ipaddr->scope;
        memset(sa, 0, sizeof(*sa));
        memcpy(sa, &s6, sizeof(s6));
    } else {
        return 0;
    }

    return 1;
}

int fr_sockaddr2ipaddr(const struct sockaddr_storage *sa, socklen_t salen,
               fr_ipaddr_t *ipaddr, int *port)
{
    if (sa->ss_family == AF_INET) {
        struct sockaddr_in  s4;

        if (salen < sizeof(s4)) {
            /* IPv4 address is too small */
            return 0;
        }

        memcpy(&s4, sa, sizeof(s4));
        ipaddr->af = AF_INET;
        ipaddr->ipaddr.ip4addr = s4.sin_addr;
        if (port) *port = ntohs(s4.sin_port);

    } else if (sa->ss_family == AF_INET6) {
        struct sockaddr_in6 s6;

        if (salen < sizeof(s6)) {
            /* IPv6 address is too small */
            return 0;
        }

        memcpy(&s6, sa, sizeof(s6));
        ipaddr->af = AF_INET6;
        ipaddr->ipaddr.ip6addr = s6.sin6_addr;
        if (port) *port = ntohs(s6.sin6_port);
        ipaddr->scope = s6.sin6_scope_id;

    } else {
        /* Unsupported address famility */
        return 0;
    }

    return 1;
}


int is_fr_ipaddr_any(const fr_ipaddr_t *ipaddr)
{
	switch (ipaddr->af) {
	case AF_INET:
		return (ipaddr->fr_ip4addr.s_addr == htonl(INADDR_ANY));
		break;
	case AF_INET6:
		return IN6_IS_ADDR_UNSPECIFIED(ipaddr->fr_ip6addr.s6_addr32);
		break;
	}
	return 0;
}


int is_fr_ipaddr_none(const fr_ipaddr_t *ipaddr)
{
	switch (ipaddr->af) {
	case AF_INET:
		return (ipaddr->fr_ip4addr.s_addr == htonl(INADDR_NONE));
		break;
	case AF_INET6:
		return (IN6_IS_ADDR_NONE(ipaddr->fr_ip6addr.s6_addr32) ||
			    IN6_IS_ADDR_MULTICAST(ipaddr->fr_ip6addr.s6_addr32)); /* or Muticasr address */
		break;
	}
	return 0;

}

int is_fr_ipaddr_loopback(const fr_ipaddr_t *ipaddr)
{
	switch (ipaddr->af) {
	case AF_INET:
		return (ipaddr->fr_ip4addr.s_addr == htonl(INADDR_LOOPBACK));
		break;
	case AF_INET6:
		return IN6_IS_ADDR_LOOPBACK(ipaddr->fr_ip6addr.s6_addr32);
		break;
	}
	return 0;
}

/*
 * ip_str is required standard address format
 * if 	IP is Loop-Back, return 0
 * else return !0
 */
int is_ipstr_loopback(int af, const char *ip_str)
{
	static const char *_loop = "127.0.0.1";
	fr_ipaddr_t fr_ip;

	switch (af) {
	case AF_INET:
		return strcmp(ip_str, _loop);
		break;
	case AF_INET6:
		if (ip_hton(ip_str, af, &fr_ip)) {
			return -1;
		}
		
		return !IN6_IS_ADDR_LOOPBACK(fr_ip.fr_ip6addr.s6_addr32);
	
		break;
	}
	return -1;
}

int set_fr_ipaddr_any(fr_ipaddr_t *ipaddr)
{
	if (!ipaddr) return -1;

	switch (ipaddr->af) {
    case AF_INET:
		ipaddr->fr_ip4addr.s_addr = htonl(INADDR_ANY);
        break;
    case AF_INET6:
		memset(ipaddr->fr_ip6addr.s6_addr, 0x00, IN6ADDRSZ);
        break;
    }
    return 0;
}

int set_fr_ipaddr_none(fr_ipaddr_t *ipaddr)
{
	if (!ipaddr) return -1;

	switch (ipaddr->af) {
    case AF_INET:
		ipaddr->fr_ip4addr.s_addr = htonl(INADDR_NONE);
        break;
    case AF_INET6:
		/* MUST not exist SUCH AS this ip address */
		memset(ipaddr->fr_ip6addr.s6_addr, 0xff, IN6ADDRSZ);
        break;
    }
    return 0;
}

int set_fr_ipaddr_loopback(fr_ipaddr_t *ipaddr)
{
	if (!ipaddr) return -1;

	switch (ipaddr->af) {
    case AF_INET:
		ipaddr->fr_ip4addr.s_addr = htonl(INADDR_LOOPBACK);
        break;
    case AF_INET6:
		memset(ipaddr->fr_ip6addr.s6_addr, 0x00, IN6ADDRSZ);
		((uint32_t *)(ipaddr->fr_ip6addr.s6_addr))[3] = htonl(1);
        break;
    }
    return 0;
}

int fr_ipaddr_debug(
const char *method, const char *desc, 
const fr_ipaddr_t * ipaddr, char *interface)
{
	BYTE b[128];
	printf("fr_debug: [%s] <%s> AF:%d,\tIP:%s,\tscope:%d,\tinterface:%s\n",
			method, 
			desc,
			ipaddr->af, 
			ip_ntoh(ipaddr, b, 128), 
			ipaddr->scope, 
			interface);
	return 0;
}

int is_power_of_2(int input)
{
    int i = 1;
    if (input < 0) return 0;
    
    for (i = 1; i <31; i++) {
        if (input == (1 << i))
            return 1;
    }
    return 0;
}
