#include <iconv.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "../include/libradius.h"

int iconv_string(char *from, char *to, const char *src, char *dst)
{
    size_t srclen;
    size_t dstlen;
    size_t inleftsize, outleftsize;
    size_t res;

    iconv_t cd;

    char *inptr;
    char *outptr;

    cd = iconv_open(to, from);
    if (cd == (iconv_t)-1) {
	return -1;
    }

    if (!strcasecmp(from, "UCS-2")) inleftsize = 2;
    else {
	srclen = (size_t)strlen(src);
	inleftsize = srclen;
    }

    outleftsize = inleftsize * 4;
    dstlen = outleftsize;
    inptr = (char *)src;
    outptr = dst;

    for (;;) {
	res = iconv(cd, &inptr, &inleftsize, &outptr, &outleftsize);
	if (res == (size_t)(-1)) {
	    if (errno == EILSEQ) {
		inptr++;
		inleftsize--;
	    } else if (errno == EINVAL) {
		if (inleftsize <= 2) {
		    *outptr = '?';
		    outleftsize--;
		    break;
		}
	    }
	    *outptr = '?';
	    outptr++;
	    outleftsize--;
	    inptr++;
	    inleftsize--;
	} else {
	    break;
	}
    }
    dst[dstlen-outleftsize] = '\0';
    iconv_close(cd);
    return (dstlen - outleftsize);
}
