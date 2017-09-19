/*
 * test_cipher_dec.c
 *
 *  Created on: 2016. 10. 24.
 *      Author: root
 */




#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#include "string_util.h"

#define IN_FILE  "encrypt.bin"
#define OUT_FILE "plain2.txt"

unsigned char *readFile(char *file, int *readLen)
{
	unsigned char * retBuffer = NULL;
	unsigned char * buffer = (unsigned char*)malloc(1001);
	int length = 0;

	BIO *fileBIO = BIO_new_file( file, "rb" );
	if (!fileBIO) {
		printf("error! file open - %s\n", file);
		exit(1);
	}
	*readLen = 0;

	printf("readFile - %s\n", file);

	while (1)
	{
		length = BIO_read( fileBIO, buffer, 1000 );
		printf("read [%d/%d]\n", length, *readLen);
		buffer[length] = 0;
		retBuffer = append_string( retBuffer, *readLen, buffer, length );
		*readLen += length;

		if (length == 1000) {
//			BIO_seek( fileBIO, 1000 );
			BIO_tell (fileBIO);
		}
		else
			break;
	}
	BIO_free(fileBIO);
	free(buffer);

	return retBuffer;
}

unsigned char *readFileBio(BIO *fileBIO, int *readLen)
{
	unsigned char * retBuffer = NULL;
	unsigned char * buffer = (unsigned char*)malloc(1001);
	int length = 0;

	*readLen = 0;

	while (1)
	{
		length = BIO_read( fileBIO, buffer, 1000 );
		buffer[length] = 0;
		retBuffer = append_string( retBuffer, *readLen, buffer, length );
		*readLen += length;

		if (length == 1000)
//			BIO_seek( fileBIO, 1000 );
			BIO_tell (fileBIO);
		else
			break;
	}
	free(buffer);

	return retBuffer;
}

// gcc test_cipher_dec.c string_util.c -o test_cipher_dec -lssl -lcrypto

int main(int argc, char* argv[])
{
	unsigned char key [] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	unsigned char iv  [] = {1,2,3,4,5,6,7,8};
	BIO *errBIO = NULL;
	BIO *outBIO = NULL;

	ERR_load_crypto_strings();

	if ((errBIO = BIO_new( BIO_s_file() )) != NULL) {
		BIO_set_fp(errBIO, stderr, BIO_NOCLOSE | BIO_FP_TEXT);
	}

	outBIO = BIO_new_file( OUT_FILE, "wb" );
	if (!outBIO) {
		BIO_printf(errBIO, "Error! failed to create [%s] file\n", OUT_FILE);
		ERR_print_errors(errBIO);
		exit(1);
	}

	BIO_printf(errBIO, "err bio create - OK\n");

	int len;
	unsigned char *readBuffer = readFile( IN_FILE, &len );

	BIO_printf(errBIO, "read file (len:%d) - OK\n", len);

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);

	BIO_printf(errBIO, "cipher ctx init - OK\n");

	// Blowfish decryption algorithm
	EVP_DecryptInit_ex( &ctx, EVP_bf_cbc(), NULL, key, iv );

	BIO_printf(errBIO, "blowfish decryption evp - OK\n");

	// add block padding
	unsigned char * outbuf
		= (unsigned char*)malloc( sizeof(unsigned char) * len );
	int outlen, tmplen;

	if (!EVP_DecryptUpdate( &ctx, outbuf, &outlen, readBuffer, len) ) {
		BIO_printf(errBIO, "failed to decrypt - EVP_DecryptUpdate\n");
		return 0;
	}
	if (!EVP_DecryptFinal_ex( &ctx, outbuf + outlen, &tmplen) ) {
		BIO_printf(errBIO, "failed to decrypt - EVP_DecryptFinal_ex\n");
		return 0;
	}
	outlen += tmplen;
	EVP_CIPHER_CTX_cleanup( &ctx );

	BIO_printf(errBIO, "%s\n", OUT_FILE);

	BIO_write( outBIO, outbuf, outlen );
	BIO_free( outBIO );
	BIO_free( errBIO );
}
