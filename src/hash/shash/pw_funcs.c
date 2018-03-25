/*
 * pw_funcs.c    Handles that are called from eap
 *
 * Copyright 2005  UNETSystem CORP.
 * written by Seung Yong Yang <joshua.yang@samsung.com>
 */

#include "autoconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <openssl/des.h>
#include <openssl/md4.h>
#include <rad_assert.h>

#include "libradius.h"
#include "sha1.h"

typedef unsigned char u8;
typedef u_int32_t u32;
typedef u_int16_t u16;

#define	S_SWAP(a,b)	do { u8 t = S[a]; S[a] = S[b]; S[b] = t; } while(0)
#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WPA_PUT_LE16(a, val)            \
    do {                    \
        (a)[1] = ((u16) (val)) >> 8;    \
        (a)[0] = ((u16) (val)) & 0xff;  \
    } while (0)

#define PWBLOCK_LEN 516


static void des_encrypt(const u8 *clear, const u8 *key, u8 *cypher)
{
    u8 pkey[8], next, tmp;
    int i;
    DES_key_schedule ks;

    /* Add parity bits to the key */
    next = 0;
    for (i = 0; i < 7; i++) {
        tmp = key[i];
        pkey[i] = (tmp >> i) | next | 1;
        next = tmp << (7 - i);
    }
    pkey[i] = next | 1;

    DES_set_key(&pkey, &ks);
    DES_ecb_encrypt((DES_cblock *) clear, (DES_cblock *) cypher, &ks,
            DES_ENCRYPT);
}


/**
 * rc4 - XOR RC4 stream to given data with skip-stream-start
 * @key: RC4 key
 * @keylen: RC4 key length
 * @skip: number of bytes to skip from the beginning of the RC4 stream
 * @data: data to be XOR'ed with RC4 stream
 * @data_len: buf length
 *
 * Generate RC4 pseudo random stream for the given key, skip beginning of the
 * stream, and XOR the end result with the data buffer to perform RC4
 * encryption/decryption.
 */
static void rc4_skip(const u8 *key, size_t keylen, size_t skip,
          u8 *data, size_t data_len)
{
    u32 i, j, k;
    u8 S[256], *pos;
    size_t kpos;

    /* Setup RC4 state */
    for (i = 0; i < 256; i++)
        S[i] = i;
    j = 0;
    kpos = 0;
    for (i = 0; i < 256; i++) {
        j = (j + S[i] + key[kpos]) & 0xff;
        kpos++;
        if (kpos >= keylen)
            kpos = 0;
        S_SWAP(i, j);
    }

    /* Skip the start of the stream */
    i = j = 0;
    for (k = 0; k < skip; k++) {
        i = (i + 1) & 0xff;
        j = (j + S[i]) & 0xff;
        S_SWAP(i, j);
    }

    /* Apply RC4 to data */
    pos = data;
    for (k = 0; k < data_len; k++) {
        i = (i + 1) & 0xff;
        j = (j + S[i]) & 0xff;
        S_SWAP(i, j);
        *pos++ ^= S[(S[i] + S[j]) & 0xff];
    }
}

/**
 * rc4 - XOR RC4 stream to given data
 * @buf: data to be XOR'ed with RC4 stream
 * @len: buf length
 * @key: RC4 key
 * @key_len: RC4 key length
 *
 * Generate RC4 pseudo random stream for the given key and XOR this with the
 * data buffer to perform RC4 encryption/decryption.
 */
static void rc4(u8 *buf, size_t len, const u8 *key, size_t key_len)
{
    rc4_skip(key, key_len, 0, buf, len);
}

/**
 * nt_password_hash_encrypted_with_block - NtPasswordHashEncryptedWithBlock() - RFC 2759, Sect 8.13
 * @password_hash: 16-octer PasswordHash (IN)
 * @block: 16-octet Block (IN)
 * @cypher: 16-octer Cypher (OUT)
 */
static void nt_password_hash_encrypted_with_block(const u8 *password_hash,
                          const u8 *block,
                          u8 *cypher)
{
    des_encrypt(password_hash, block, cypher);
    des_encrypt(password_hash + 8, block + 7, cypher + 8);
}

/**
 * encrypt_pw_block_with_password_hash - EncryptPwBlobkWithPasswordHash() - RFC 2759, Sect. 8.10
 * @password: 0-to-256-unicode-char Password (IN)
 * @password_len: Length of password
 * @password_hash: 16-octet PasswordHash (IN)
 * @pw_block: 516-byte PwBlock (OUT)
 */
static void encrypt_pw_block_with_password_hash(
		const unsigned char *password, size_t password_len,
		const unsigned char *password_hash, unsigned char *pw_block)
{
	size_t i, offset;
	unsigned char *pos;

	if (password_len > 256)
		return;

	memset(pw_block, 0, PWBLOCK_LEN);
	offset = (256 - password_len) * 2;
	for (i = 0; i < password_len; i++)
		pw_block[offset + i * 2] = password[i];
	pos = &pw_block[2 * 256];
	WPA_PUT_LE16(pos, password_len * 2);
	rc4(pw_block, PWBLOCK_LEN, password_hash, 16);
}

/**
 * new_password_encrypted_with_old_nt_password_hash - 
 * NewPasswordEncryptedWithOldNtPasswordHash() - RFC 2759, Sect. 8.9
 * @new_password: 0-to-256-unicode-char NewPassword (IN)
 * @new_password_len: Length of new_password
 * @old_password: 0-to-256-unicode-char OldPassword (IN)
 * @old_password_len: Length of old_password
 * @encrypted_pw_block: 516-octet EncryptedPwBlock (OUT)
 */
void new_password_encrypted_with_old_nt_password_hash(
		const unsigned char *new_password,
		size_t new_password_len,
		const unsigned char *old_password,
		size_t old_password_len,
		unsigned char *encrypted_pw_block)
{
	unsigned char password_hash[16];

	nt_password_hash(old_password, old_password_len, password_hash);
	encrypt_pw_block_with_password_hash(
			new_password, new_password_len,
			password_hash, encrypted_pw_block);
}


/**
 * decrypt_new_oassword_with_old_nt_password_hash 
 * @encrypted_pw_block: encrypted password block in ChangePassword packet (IN)
 * @old_password: 0-to-256-unicode-char OldPassword (IN)
 * @old_password_len: Length of old_password (IN)
 * @new_password: plaintext new password (OUT)
 * @new_password_len: plaintext new password len (OUT)
 */
int decrypt_new_oassword_with_old_nt_password_hash(
	unsigned char *encrypted_pw_block, 
	const char *old_password, 
	size_t old_password_len,
	char *new_password, 
	size_t *new_password_len)
{
	unsigned char password_hash[16];
	unsigned char pw_block[PWBLOCK_LEN];
	unsigned char *pos;
	unsigned short len;
	size_t i, offset;

	/* generate RC4 key */
	nt_password_hash(old_password, old_password_len, password_hash);

	/* decrypt password block */
	memcpy (pw_block, encrypted_pw_block, PWBLOCK_LEN);
	rc4(pw_block, PWBLOCK_LEN, password_hash, 16);

	pos = &pw_block[2 * 256];
	len = WPA_GET_LE16(pos);
	*new_password_len = len / 2;

	if ((*new_password_len > 256) || (*new_password_len <= 0)) {
		*new_password_len = 0; /* logging is done in caller */
		return -1;
	}

	offset = (256 - *new_password_len) * 2;

	for (i = 0; i < *new_password_len; i++)
		new_password[i] = pw_block[offset + i * 2];

	return 0;
}

/**
 * old_nt_password_hash_encrypted_with_new_nt_password_hash - OldNtPasswordHashEncryptedWithNewNtPasswordHash() - RFC 2759, Sect. 8.12
 * @new_password: 0-to-256-unicode-char NewPassword (IN)
 * @new_password_len: Length of new_password
 * @old_password: 0-to-256-unicode-char OldPassword (IN)
 * @old_password_len: Length of old_password
 * @encrypted_password_ash: 16-octet EncryptedPasswordHash (OUT)
 */
void old_nt_password_hash_encrypted_with_new_nt_password_hash(
    const u8 *new_password, size_t new_password_len,
    const u8 *old_password, size_t old_password_len,
    u8 *encrypted_password_hash)
{
    u8 old_password_hash[16], new_password_hash[16];

    nt_password_hash(old_password, old_password_len, old_password_hash);
    nt_password_hash(new_password, new_password_len, new_password_hash);
    nt_password_hash_encrypted_with_block(old_password_hash,
                          new_password_hash,
                          encrypted_password_hash);
}

static const char *letters = "0123456789ABCDEF";


/*
 *  bin2hex creates hexadecimal presentation
 *  of binary data
 */
void bin2hex (const unsigned char *szBin, char *szHex, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        szHex[i<<1] = letters[szBin[i] >> 4];
        szHex[(i<<1) + 1] = letters[szBin[i] & 0x0F];
    }
}

/*
 *  hex2bin converts hexadecimal strings into binary
 */
int hex2bin (const char *szHex, unsigned char* szBin, int len)
{
	char * c1, * c2;
	int i;

	for (i = 0; i < len; i++) {
		if( !(c1 = memchr(letters, toupper((int) szHex[i << 1]), 16)) ||
			!(c2 = memchr(letters, toupper((int) szHex[(i << 1) + 1]), 16)))
             break;
		szBin[i] = ((c1-letters)<<4) + (c2-letters);
	}
	return i;
}


/**
 * nt_password_hash - NtPasswordHash() - RFC 2759, Sect. 8.3
 * @password: 0-to-256-unicode-char Password (IN)
 * @password_len: Length of password
 * @password_hash: 16-octet PasswordHash (OUT)
 */
void nt_password_hash(const unsigned char *password, size_t password_len,
              unsigned char *password_hash)
{
    unsigned char buf[256];
    size_t i, len;

    if (password_len > 127)
        return;

    memset(buf, 0, password_len * 2);
    for (i = 0; i < password_len; i++)
        buf[2 * i] = password[i];

    len = password_len * 2;
    MD4(buf, len, password_hash);
}


/**
 * hash_nt_password_hash - HashNtPasswordHash() - RFC 2759, Sect. 8.4
 * @password_hash: 16-octet PasswordHash (IN)
 * @password_hash_hash: 16-octet PaswordHashHash (OUT)
 */
static void hash_nt_password_hash(const unsigned char *password_hash,
                            unsigned char *password_hash_hash)
{
    size_t len = 16;
    MD4(password_hash, len, password_hash_hash);
}


/*
 *  challenge_hash() is used by mschap2() and auth_response()
 *  implements RFC2759 ChallengeHash()
 *  generates 64 bit challenge
 */
void challenge_hash( const char *peer_challenge,
                const char *auth_challenge,
                const char *user_name, char *challenge )
{
    SHA1_CTX Context;
    unsigned char hash[20];

    SHA1Init(&Context);
    SHA1Update(&Context, peer_challenge, 16);
    SHA1Update(&Context, auth_challenge, 16);
    SHA1Update(&Context, user_name, strlen(user_name));
    SHA1Final(hash, &Context);
    memcpy(challenge, hash, 8);
}


/*
 *  generate_authenticator_response() generates MS-CHAP v2 SUCCESS response
 *  according to RFC 2759 GenerateAuthenticatorResponse()
 *  returns 42-octet response string
 */
void generate_authenticator_response(const char *username,
        	const char *password, unsigned char *nthashhash,
        	unsigned char *ntresponse,
        	char *peer_challenge, 
			char *auth_challenge,
        	char *response)
{
    SHA1_CTX Context;
    const unsigned char magic1[39] =
    {0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
     0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
     0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
     0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74};

    const unsigned char magic2[41] =
    {0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
     0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
     0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
     0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
     0x6E};

    unsigned char password_hash[16], password_hash_hash[16];
    char challenge[8];
    unsigned char digest[20];

	if (nthashhash != NULL) {
		memcpy(password_hash_hash, nthashhash, 16);
	}
	else {
    	nt_password_hash(password, strlen(password), password_hash);
    	hash_nt_password_hash(password_hash, password_hash_hash);
	}

    SHA1Init(&Context);
    SHA1Update(&Context, password_hash_hash, 16);
    SHA1Update(&Context, ntresponse, 24);
    SHA1Update(&Context, magic1, 39);
    SHA1Final(digest, &Context);
    challenge_hash(peer_challenge, auth_challenge, username, challenge);
    SHA1Init(&Context);
    SHA1Update(&Context, digest, 20);
    SHA1Update(&Context, challenge, 8);
    SHA1Update(&Context, magic2, 41);
    SHA1Final(digest, &Context);

    /*
     *  Encode the value of 'Digest' as "S=" followed by
     *  40 ASCII hexadecimal digits and return it in
     *  AuthenticatorResponse.
     *  For example,
     *  "S=0123456789ABCDEF0123456789ABCDEF01234567"
     */
    response[0] = 'S';
    response[1] = '=';
    bin2hex(digest, response + 2, 20);
}
