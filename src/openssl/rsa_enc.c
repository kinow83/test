/*
 * rsa_enc.c
 *
 *  Created on: 2016. 10. 25.
 *      Author: root
 */




#include <stdio.h>
#include <strings.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/rand.h>
#include <openssl/pem.h>



int main(int argc, char** argv)
{
	BIO *bio_key = NULL;
	BIO *bio_err = NULL;
	RSA *rsa = NULL;

	char key_file[100];
	char key_format[10];
	char enc_type[10];
	char key_type[10];
	char in_file[100];
	char out_file[100];
	BIO *bio_in  = NULL;
	BIO *bio_out = NULL;

	bio_err = BIO_new( BIO_s_file() );
	if (bio_err) {
		BIO_set_fp( bio_err, stderr, BIO_NOCLOSE|BIO_FP_TEXT );
	}

	printf("encryt? decrypt? (e,d)\n");
	scanf("%s", enc_type);

	printf("public key? private key? (pri, pub)\n");
	scanf("%s", key_type);

	printf("key file?\n");
	scanf("%s", key_file);

	printf("key format? (pem, der)\n");
	scanf("%s", key_format);

	printf("input file?\n");
	scanf("%s", in_file);

	printf("output file?\n");
	scanf("%s", out_file);

	bio_key = BIO_new( BIO_s_file() );
	if (bio_key == NULL) {
		ERR_print_errors(bio_err);
		exit(1);
	}
	// read key file
	if (BIO_read_filename( bio_key, key_file ) <= 0) {
		BIO_printf(bio_err, "error open key file [%s]\n", key_file);
		ERR_print_errors(bio_err);
		exit(1);
	}
	if (strcasecmp(key_format, "der") == 0) {
		if (strcasecmp(key_type, "pub") == 0) {
			// DER/PUB
			rsa = d2i_RSAPublicKey_bio( bio_key, NULL );
		} else {
			// DER/PRI
			rsa = d2i_RSAPrivateKey_bio( bio_key, NULL );
		}
	}
	else if (strcasecmp(key_format, "pem") == 0) {
		if (strcasecmp(key_type, "pub") == 0) {
			// PEM/PUB
			rsa = PEM_read_bio_RSA_PUBKEY( bio_key, NULL, NULL, NULL );
		} else {
			// PEM/PRI
			rsa = PEM_read_bio_RSAPrivateKey( bio_key, NULL, NULL, NULL );
		}
	}

	if (rsa == NULL) {
		BIO_printf(bio_err, "unable road key\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	bio_in  = BIO_new_file( in_file,  "rb" );
	bio_out = BIO_new_file( out_file, "wb" );
	int key_size = RSA_size(rsa);

	// input  buffer = key size * 2
	// output buffer = key size
	unsigned char *in_buffer  = (unsigned char*)malloc(key_size * 2);
	unsigned char *out_buffer = (unsigned char*)malloc(key_size);
	unsigned char pad = RSA_PKCS1_PADDING;

	// road content from input file
	int in_length = BIO_read( bio_in, in_buffer, key_size*2 );
	int out_length = 0;

	if ( (strcasecmp(enc_type, "e") == 0) && (strcasecmp(key_type, "pub") == 0) ) {
		out_length = RSA_public_encrypt( in_length, in_buffer, out_buffer, rsa, pad);
	} else
	if ( (strcasecmp(enc_type, "e") == 0) && (strcasecmp(key_type, "pri") == 0) ) {
		out_length = RSA_private_encrypt( in_length, in_buffer, out_buffer, rsa, pad);
	} else
	if ( (strcasecmp(enc_type, "d") == 0) && (strcasecmp(key_type, "pub") == 0) ) {
		out_length = RSA_public_decrypt( in_length, in_buffer, out_buffer, rsa, pad);
	} else
	if ( (strcasecmp(enc_type, "d") == 0) && (strcasecmp(key_type, "pri") == 0) ) {
		out_length = RSA_private_decrypt( in_length, in_buffer, out_buffer, rsa, pad);
	}

	if (out_length < 0) {
		BIO_printf(bio_err, "error RSA encrypt(decrypt)\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	// save ok for out file
	BIO_write( bio_out, out_buffer, out_length );
	BIO_printf(bio_err, "completed OK\n");

	if (bio_in) BIO_free(bio_in);
	if (bio_out) BIO_free(bio_out);
	if (bio_err) BIO_free(bio_err);
	if (bio_key) BIO_free(bio_key);
	if (rsa) RSA_free(rsa);

	free(in_buffer);
	free(out_buffer);

	return 0;
}


















