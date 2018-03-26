/*
 * crypt.c	A thread-safe crypt wrapper
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

#include "libradius.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>


#ifdef HAVE_PTHREAD_H
#include	<pthread.h>

/*
 *  No pthreads, no mutex.
 */
static int lrad_crypt_init = 0;
static pthread_mutex_t lrad_crypt_mutex;
#endif

/*
 * performs a crypt password check in an thread-safe way.
 *
 * returns:  0 -- check succeeded
 *          -1 -- failed to crypt
 *           1 -- check failed
 */
int lrad_crypt_check(const char *key, const char *crypted)
{
	char *passwd;
	int cmp = 0;

#ifdef HAVE_PTHREAD_H
	/*
	 *	Ensure we're thread-safe, as crypt() isn't.
	 */
	if (lrad_crypt_init == 0) {
		pthread_mutex_init(&lrad_crypt_mutex, NULL);
		lrad_crypt_init = 1;
	}

	pthread_mutex_lock(&lrad_crypt_mutex);
#endif

	passwd = crypt(key, crypted);

	/*
	 *	Got something, check it within the lock.  This is
	 *	faster than copying it to a local buffer, and the
	 *	time spent within the lock is critical.
	 */
	if (passwd) {
		cmp = strcmp(crypted, passwd);
	}

#ifdef HAVE_PTHREAD_H
	pthread_mutex_unlock(&lrad_crypt_mutex);
#endif

	/*
	 *	Error.
	 */
	if (!passwd) {
		return -1;
	}

	/*
	 *	OK, return OK.
	 */
	if (cmp == 0) {
		return 0;
	}

	/*
	 *	Comparison failed.
	 */
	return 1;
}


static
int cipher_crypt(const EVP_CIPHER *evp_type, 
			   const uint8_t *in_data, int in_data_len, 
			   uint8_t **out_data, int *out_data_len, 
			   const uint8_t* key, const uint8_t* iv, int enc, int pad)
{
	int ret = 0;
	EVP_CIPHER_CTX ctx;
	uint8_t *crypt = NULL;
	int crypt_len;
	int tmplen;

	if (evp_type == NULL)
		return 0;

	EVP_CIPHER_CTX_init(&ctx);

	ret = EVP_CipherInit_ex(&ctx, evp_type, NULL, key, iv, enc);
	if (ret != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		return 0;
	}

	ret = EVP_CIPHER_CTX_set_padding(&ctx, pad);
	if (ret != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
        return 0;
	}

	if (pad == 1) {
		tmplen = EVP_CIPHER_CTX_block_size(&ctx);
		tmplen = ((in_data_len + tmplen)/tmplen) * tmplen;
	} else {
		tmplen = in_data_len;
	}

	crypt = malloc(tmplen);
	memset(crypt, 0, tmplen);
	tmplen = 0;

	ret = EVP_CipherUpdate(&ctx, crypt, &crypt_len, in_data, in_data_len);
	if (ret != 1) {
		EVP_CIPHER_CTX_cleanup(&ctx);
		free(crypt);
		*out_data_len = 0;
		*out_data = NULL;
        return 0;
	}

	ret = EVP_CipherFinal_ex(&ctx, crypt + crypt_len, &tmplen);
	if (ret != 1) {
        EVP_CIPHER_CTX_cleanup(&ctx);
        free(crypt);
        *out_data_len = 0;
        *out_data = NULL;
        return 0;
    }
	crypt_len += tmplen;

	*out_data = crypt;
	*out_data_len = crypt_len;

    EVP_CIPHER_CTX_cleanup(&ctx);

	return 1;
}

static const EVP_CIPHER * cipher_evp_type(int cipher_type)
{
	const EVP_CIPHER *evp_type = NULL;

	switch (cipher_type)
	{
		// SEED
		case CIPHER_SEED128:
			evp_type = EVP_seed_cbc();
			break;
		case CIPHER_SEED256:
			// not yet.
			break;

		// AES
		case CIPHER_AES128:
			evp_type = EVP_aes_128_cbc();
			break;
		case CIPHER_AES192:
			evp_type = EVP_aes_192_cbc();
			break;
		case CIPHER_AES256:
			evp_type = EVP_aes_256_cbc();
			break;
		case CIPHER_AES512:
			// not yet.
			break;

		// DES
		case CIPHER_DES:
			evp_type = EVP_des_cbc();
			break;
		case CIPHER_DES3:
//			evp_type = EVP_des_ede_cbc();  // 2 key triple DES, k1 = k3, k2
			evp_type = EVP_des_ede3_cbc(); // 3 key triple DES, k1 != k2 != k3
			break;
	}
	return evp_type;
}


int cipher_encrypt(int cipher_type, const char *in_data, 
				   uint8_t **out_data, int *out_data_len, 
				   const uint8_t* key, const uint8_t* iv)
{
	const EVP_CIPHER *evp_type = NULL;

	evp_type = cipher_evp_type(cipher_type);
	if (evp_type == NULL) {
		return 0;
	}

	/* padding = TRUE */
	return cipher_crypt(evp_type, (const uint8_t *)in_data, strlen(in_data), 
						out_data, out_data_len, key, iv, 1, 1);
}

int cipher_decrypt(int cipher_type, const uint8_t *in_data, int in_data_len, 
				 char **out_data, const uint8_t* key, const uint8_t* iv)
{
	int out_data_len;
	uint8_t *out1 = NULL, *out2 = NULL;
	const EVP_CIPHER *evp_type = NULL;

	evp_type = cipher_evp_type(cipher_type);
	if (evp_type == NULL) {
		return 0;
	}

	/* padding = TRUE */
	if (cipher_crypt(evp_type, in_data, in_data_len, 
				   &out1, &out_data_len, key, iv, 0, 1) == 0) {
		*out_data = NULL;
		return 0;
	}

	out2 = malloc(out_data_len+1); // NULL char
	memcpy(out2, out1, out_data_len);
	out2[out_data_len] = 0;

	free(out1);

	*out_data = out2;

	return 1;
}

char *base64_encode(unsigned char *input, int length)
{
  BIO *bmem, *b64;
  BUF_MEM *bptr;
  char *buff;

  b64 = BIO_new(BIO_f_base64());
  /* encode the data all on one line or 
   * expect the data to be all on one line.
   */
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, input, length);
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);

  buff = (char *)malloc(bptr->length+1);
  memcpy(buff, bptr->data, bptr->length+1);
  buff[bptr->length+1] = 0;

  BIO_free_all(b64);

  return buff;
}

char *base64_decode(unsigned char *input, int length, int *outlen)
{
  BIO *b64, *bmem;

  char *buffer = (char *)malloc(length);
  memset(buffer, 0, length);

  b64 = BIO_new(BIO_f_base64());
  /* encode the data all on one line or 
   * expect the data to be all on one line.
   */
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bmem = BIO_new_mem_buf(input, length);
  bmem = BIO_push(b64, bmem);

  *outlen = BIO_read(bmem, buffer, length);

  BIO_free_all(bmem);

  return buffer;
}

