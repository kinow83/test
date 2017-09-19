/*
 * test_PRNG.c
 *
 *  Created on: 2016. 10. 20.
 *      Author: root
 */

#include <stdio.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/evp.h>

int getRand()
{
	int ret = 0;
	int length = 64;

	// seed with PRNG
	//RAND_screen();

	unsigned char *buffer = (unsigned char*)
			malloc(sizeof(unsigned char) * length);

	ret = RAND_bytes(buffer, length);
	if (ret <= 0) {
		printf("error build rand number\n");
		return 0;
	}

	printf("rand number = ");
	for (int i=0; i<length; i++) {
		printf("%c", buffer[i]);
	}
	return ret;
}

int test_evp_cipher_ctx()
{
	EVP_CIPHER_CTX *ctx = NULL;

	EVP_CIPHER_CTX_init(ctx);

	int EVP_EncryptFinal_ex()
}

int make_key_and_iv()
{
	unsigned char salt[8];
	const EVP_CIPHER *cipher = NULL;
	char *password = "test_password";
	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH];
	int ret = 0;

	printf("EVP_MAX_KEY_LENGTH = %d\n", EVP_MAX_KEY_LENGTH);
	printf("EVP_MAX_IV_LENGTH  = %d\n", EVP_MAX_IV_LENGTH);

	// make salt with PRNG
	ret = RAND_pseudo_bytes(salt, sizeof(salt));
	if (ret < 0) {
		printf("failed to create random salt\n");
		return -1;
	}

	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();

	// DES - ECB
//	cipher = EVP_des_ecb();
	cipher = EVP_aes_128_ecb();
	cipher = EVP_get_cipherbyname("AES-128-CBC");
	if (cipher == NULL) {
		printf("unkowun cipher\n");
		exit(1);
	}

	// create key and iv
	EVP_BytesToKey(
			cipher,
			EVP_sha256(),
			salt,
			(unsigned char*)password, strlen(password),
			1, // count
			key,
			iv);

	// print salt
	printf("salt(%ld): ", sizeof(salt));
	for (int i=0; i<sizeof salt; i++) {
		printf("%02x", salt[i]);
	}
	printf("\n");

	// print key
	printf("key(%d): ", cipher->key_len);
	if (cipher->key_len > 0) {
		for (int i=0; i<cipher->key_len; i++) {
			printf("%02x", key[i]);
		}
	}
	printf("\n");

	// print iv
	printf("iv(%d): ", cipher->iv_len);
	if (cipher->iv_len > 0) {
		for (int i=0; i<cipher->iv_len; i++) {
			printf("%02x", iv[i]);
		}
	}
	printf("\n");

	return 0;
}

int main(int argc, char* argv[])
{
//	getRand();
	make_key_and_iv();
}



