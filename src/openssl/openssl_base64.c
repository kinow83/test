/*
 * openssl_base64.c
 *
 *  Created on: 2017. 8. 25.
 *      Author: root
 */

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

int base64_encode(const char *msg, char **buf)
{
	BIO *bio, *b64;
	FILE *stream;
	int encode_size = 4 * ceil(((double)strlen(msg))/3);

	*buf = (char *)malloc(encode_size + 1);
	stream = fmemopen(*buf, encode_size + 1, "w");

	b64 = BIO_new( BIO_f_base64() );
	bio = BIO_new_fp(stream, BIO_NOCLOSE);

	// bio append to base64
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	// ignore newline. - write everything in one line

	BIO_write(bio, msg, strlen(msg));
	BIO_flush(bio);
	BIO_free_all(bio);
	fclose(stream);

	return 0;
}

int base64_decode_length(const char *b64_input)
{
	int len;
	int padding = 0;

	len = strlen(b64_input);

	if (b64_input[len-1] == '=' && b64_input[len-2] == '=') {
		padding = 2;
	}
	else if (b64_input[len-1] == '=') {
		padding = 1;
	}
	return ((int)len * 0.75) - padding;
}

int base64_decode(char *b64msg, char **buf)
{
	BIO *bio, *b64;
	int decodelen;
	FILE *stream;
	int len;

	decodelen = base64_decode_length(b64msg);
	*buf = (char *)malloc(decodelen + 1);

	stream = fmemopen(b64msg, strlen(b64msg), "r");

	b64 = BIO_new( BIO_f_base64() );
	bio = BIO_new_fp(stream, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	// ignore newline. - write everything in one line

#if 0
	len = BIO_read(bio, *buf, strlen(b64msg));
#else
	len = BIO_read(bio, *buf, decodelen);
#endif
	(*buf)[len] = '\0';

	printf("base64 msg len: %ld, base64 decode len:%d\n",
			strlen(b64msg), decodelen);

	BIO_free_all(bio);
	fclose(stream);

	return 0;
}

int main()
{
	static char msg[] = "kinow kaka kangsukju 1234567890";
	char *base64_enc;
	char *base64_dec;

	base64_encode(msg, &base64_enc);
	printf("base64 enc: %s\n", base64_enc);

	base64_decode(base64_enc, &base64_dec);
	printf("base64 dec: %s\n", base64_dec);

	free(base64_enc);
	free(base64_dec);
}

/*
 *  gcc openssl_base64.c -o openssl_base64 -lm -lcrypto
 */


