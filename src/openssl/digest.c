/*
 * digest.c
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


int main(int argc, char **argv)
{
	EVP_MD_CTX ctx;
	const EVP_MD *md;
	const char *md_name = argv[1] ? argv[1] : "sha256";

	char message1[] = "this is test message.\n";
	char message2[] = "hi kinow~~~";
	unsigned char hash_value[EVP_MAX_MD_SIZE];
	unsigned int hash_len, i;

	OpenSSL_add_all_digests();

	md = EVP_get_digestbyname(md_name);
	if (!md) {
		printf("unkowun digest algorithm - %s\n", md_name);
		exit(1);
	}

	EVP_MD_CTX_init( &ctx );
	EVP_DigestInit_ex( &ctx, md, NULL );
	EVP_DigestUpdate( &ctx, message1, (unsigned int)strlen(message1) );
	EVP_DigestUpdate( &ctx, message2, (unsigned int)strlen(message2) );
	EVP_DigestFinal_ex( &ctx, hash_value, &hash_len);
	EVP_MD_CTX_cleanup( &ctx );

	printf("hash length = %d\n", hash_len);
	for (i=0; i<hash_len; i++) {
		printf("%02x", hash_value[i]);
	}
	printf("\n");
	return 0;
}

