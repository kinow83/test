#ifndef UDPFROMTO_H
#define UDPFROMTO_H
/*
 * Version:	$Id: udpfromto.h,v 1.1.1.1 2006/02/24 08:09:17 cvs Exp $
 *
 */

#include <sys/socket.h>

int udpfromto_init(int s);
int recvfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t *fromlen,
	       struct sockaddr *to, socklen_t *tolen);
int sendfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t fromlen,
	       struct sockaddr *to, socklen_t tolen);

#endif
