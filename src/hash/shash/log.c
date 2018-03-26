/*
 * log.c	Functions in the library call radlib_log() which
 *		sets a global error string "char *librad_errstr".
 *
 * Version:	$Id: log.c,v 1.1.1.1 2006/02/24 08:09:18 cvs Exp $
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

#include "autoconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "libradius.h"
#define BYTE	char
/*
 *  Try to create a thread-local-storage version of this buffer.
 */
static __thread BYTE librad_errstr[1024];

void librad_log(const char *fmt, ...)
{
	va_list ap;
	BYTE my_errstr[sizeof(librad_errstr)];

	va_start(ap, fmt);
	vsnprintf(my_errstr, sizeof(my_errstr), fmt, ap);
	snprintf(librad_errstr, sizeof(librad_errstr), "%s", my_errstr);
	va_end(ap);
}

const char *librad_strerror(void)
{
	return librad_errstr;
}

void librad_perror(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	if (strchr(fmt, ':') == NULL)
		fprintf(stderr, ": ");
	fprintf(stderr, "%s\n", librad_errstr);
}

/* Thread - unsafety */
const char * cc_int2char(int data) {
#ifdef SUPPORT_CC
	static BYTE c_data[32];

	memset(c_data, 0, sizeof(c_data));
	snprintf(c_data, sizeof(c_data), "%u", (unsigned int)data);
	return c_data;
#else
	return "";
#endif
}

void cc_func_log(const char *func_name, const char *params, const char *ret_val)
{
#ifdef SUPPORT_CC
	if (func_name == NULL || ret_val == NULL) 
		return;

	cc_log("+ ----------------------------------------- +");
	cc_log("+ Function Name : %s (%s)", func_name, params == NULL ? "" : params);
	cc_log("+ ----------------------------------------- +");
	cc_log("+ [Output Variable]");
	cc_log("+ return : %s", ret_val);
	cc_log("");
#endif
}

void cc_log(const char *fmt, ...)
{
#ifdef SUPPORT_CC
	FILE *fd = NULL;
	time_t curtime;
	struct tm *loctime;
	va_list ap;
	BYTE fname[300];
	char *progname = "radius";
	int rstat;
	struct stat logstat;
	BYTE time_stamp[20];
	DIR *dp;
	struct dirent *ep;
	char *logdir = "/tmp/";


	memset(fname, 0, sizeof(fname));

	curtime = time (NULL);
    loctime = localtime (&curtime);

	memset (time_stamp, 0, sizeof(time_stamp));
	strftime (time_stamp, sizeof(time_stamp), "%Y%m%d", loctime );

	snprintf(fname, sizeof(fname), "%scc_radius-%s.log", logdir, time_stamp );
	
	/* open file-stat */
	rstat = stat(fname, &logstat);
	if (rstat == ENOENT) {	
		// not exist file - continue;
	}	
	else if (rstat == 0) 
	{
		// over file size
		if (logstat.st_size > (long long)1024*1024*1024) 
		{
			int today_log_cnt = 1;
			BYTE buff[50];
			BYTE newlogfile[300];

			dp = opendir(logdir);

			if (dp == NULL) {
				fprintf(stderr, "%s: Couldn't open dir %s for logging: %s\n", 
					progname, logdir, strerror(errno));
				//return -1;
				exit(1);
			}

			snprintf(buff, sizeof(buff), "cc_radius-%s-", time_stamp);
			while ( (ep = readdir (dp)) != 0 ) 
			{
				if (strstr(ep->d_name, buff)!=NULL)
					today_log_cnt++;
			}
			(void) closedir (dp);

			// rename log file
			snprintf(newlogfile, sizeof(newlogfile), "%scc_radius-%s-%03d.log", logdir, time_stamp, today_log_cnt );
			if(rename (fname, newlogfile)==-1) {
				fprintf(stderr, "%s: Couldn't rename %s to %s :%s\n", 
                    	progname, fname, newlogfile, strerror(errno));
				//return -1;
				exit (1);
			}
		}
	}

	BYTE buffer[1024] = {0};

	if ((fd = fopen(fname, "a+")) == NULL) {
		fprintf(stderr, "%s: Couldn't open %s for logging: %s\n",
			progname, fname, strerror(errno));
		//return -1;
		exit(1);
	}

	int d_len = strftime( buffer, 512, "%Y-%m-%d %H:%M:%S: ", loctime );

	va_start(ap, fmt);

	vsnprintf(buffer+d_len, sizeof(buffer) - d_len, fmt, ap);	
/*
	for (p = (unsigned char *)buffer; *p != '\0'; p++) {
        if (*p == '\r' || *p == '\n')
            *p = ' ';
        else if (*p < 32 || (*p >= 128 && *p <= 160))
            *p = '?';
    }   
*/
    strcat(buffer, "\n");

	fputs(buffer, fd);
	fclose(fd);

	va_end(ap);
#endif
}
