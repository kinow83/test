/*
 * rsa_test.c
 *
 *  Created on: 2016. 10. 25.
 *      Author: root
 */




#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

static void genrsa_cb(int p, int n, void *arg);

int main(int argc, char** argv)
{
	BIO *bio_public_out = NULL;
	BIO *bio_private_out = NULL;
	BIO *bio_stdout = NULL;
	RSA *rsa = NULL;
	int key_len_input = 512;
	char public_key_file_out[50];
	char private_key_file_out[50];
	bool is_pem = true;

	printf("key length ?\n");
	scanf("%d", &key_len_input);

	printf("public key file ?\n");
	scanf("%s", public_key_file_out);

	printf("private key file ?\n");
	scanf("%s", private_key_file_out);

	bio_stdout = BIO_new( BIO_s_file() );
	if (bio_stdout) {
		BIO_set_fp( bio_stdout, stdout, BIO_NOCLOSE | BIO_FP_TEXT );
	}

	bio_public_out = BIO_new( BIO_s_file() );
	if (bio_public_out == NULL) {
		printf("error bio_public_out - %s\n", public_key_file_out);
		exit(1);
	}

	bio_private_out = BIO_new( BIO_s_file() );
	if (bio_private_out == NULL) {
		printf("error bio_private_out - %s\n", private_key_file_out);
		exit(1);
	}

	if (BIO_write_filename( bio_public_out, public_key_file_out ) <= 0) {
		printf("error BIO_write_filename - public key file out [%s]\n",
				public_key_file_out);
		exit(1);
	}

	if (BIO_write_filename( bio_private_out, private_key_file_out ) <= 0) {
		printf("error BIO_write_filename - private key file out [%s]\n",
				private_key_file_out);
		exit(1);
	}

//	RAND_screen();

	rsa = RSA_generate_key(key_len_input, RSA_F4, genrsa_cb, NULL);

	if (is_pem)
	{
		printf("PEM PUBLIC KEY -> stdout\n");
		// show public key of PEM format to stdout
		if (!PEM_write_bio_RSA_PUBKEY( bio_stdout, rsa ))	{
			printf("error PEM_write_bio_RSA_PUBKEY to stdout \n");
			exit(1);
		}
		printf("PEM PRIVATE KEY -> stdout\n");
		// show private key of PEM format to stdout
		if (!PEM_write_bio_RSAPrivateKey( bio_stdout, rsa, NULL, NULL, 0, NULL, NULL )) {
			printf("error PEM_write_bio_RSAPrivateKey to stdout \n");
			exit(1);
		}
		printf("PEM PUBLIC KEY -> %s\n", public_key_file_out);
		// show public key of PEM format to stdout
		if (!PEM_write_bio_RSA_PUBKEY( bio_public_out, rsa ))	{
			printf("error PEM_write_bio_RSA_PUBKEY to file \n");
			exit(1);
		}
		printf("PEM PRIVATE KEY -> %s\n", private_key_file_out);
		// show private key of PEM format to stdout
		if (!PEM_write_bio_RSAPrivateKey( bio_private_out, rsa, NULL, NULL, 0, NULL, NULL )) {
			printf("error PEM_write_bio_RSAPrivateKey to file \n");
			exit(1);
		}
	}
	else
	{
		printf("DER PUBLIC KEY -> stdout\n");
		//
		if (!i2d_RSA_PUBKEY_bio( bio_stdout, rsa )) {
			printf("error i2d_RSA_PUBKEY_bio to stdout \n");
			exit(1);
		}

		printf("DER PRIVATE KEY -> stdout\n");
		if (!i2d_RSAPrivateKey_bio( bio_stdout, rsa )) {
			printf("error i2d_RSAPrivateKey_bio to stdout \n");
			exit(1);
		}

		printf("DER PUBLIC KEY -> %s\n", public_key_file_out);
		if (!i2d_RSA_PUBKEY_bio( bio_public_out, rsa )) {
			printf("error i2d_RSA_PUBKEY_bio to file \n");
			exit(1);
		}

		printf("DER PRIVATE KEY -> %s\n", private_key_file_out);
		if (!i2d_RSAPrivateKey_bio( bio_private_out, rsa )) {
			printf("error i2d_RSAPrivateKey_bio to file \n");
			exit(1);
		}
	}
	if (rsa) RSA_free(rsa);
	if (bio_public_out) BIO_free_all(bio_public_out);
	if (bio_private_out) BIO_free_all(bio_private_out);
	return 0;
}

void genrsa_cb(int p, int n, void *arg)
{
	char c = '*';
	if (p == 0) c = '.';
	if (p == 1) c = '+';
	if (p == 2) c = '*';
	if (p == 3) c = '\n';
	printf("%c", c);
}
