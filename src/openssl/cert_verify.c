/*
 * cert_verify.c
 *
 *  Created on: 2016. 10. 26.
 *      Author: root
 */

#include <stdio.h>
#include <openssl/bio.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

#define     CRL_FILE "crl.pem"
static char CA_CERT_FILE[50] = "CA/rootcert.pem";
static char CERT_FILE[50] = "CA/certs/00.pem";

int verify_cb_func(int ok, X509_STORE_CTX *store)
{
	X509 * cert = NULL;
	X509_NAME *subject_nm = NULL;
	X509_NAME *issuer_nm = NULL;
	int depth;
	char *subject = NULL;
	char *issuer = NULL;
	int subject_cn_index;
	X509_NAME_ENTRY *subject_cn_entry;

	cert = X509_STORE_CTX_get_current_cert(store);

	subject_nm = X509_get_subject_name(cert);
	subject = X509_NAME_oneline(subject_nm, NULL, 256);

	issuer_nm = X509_get_issuer_name(cert);
	issuer = X509_NAME_oneline(issuer_nm, NULL, 256);

	printf("subject = %s\n", subject);
	printf("issuer  = %s\n", issuer);

	subject_cn_index = X509_NAME_get_index_by_NID(subject_nm, NID_commonName, -1);
	printf("subject = %s -> cn index = %d\n", subject, subject_cn_index);

	if (subject_cn_index >= 0) {
		subject_cn_entry = X509_NAME_get_entry(subject_nm, subject_cn_index);
	}


	if (!ok)
	{
		depth = X509_STORE_CTX_get_error_depth(store);
		printf("error: %s\n", X509_verify_cert_error_string(store->error));
		printf("depth = %d\n", depth);
	}
	else
	{

	}

	return ok;
}

int main(int argc, char** argv)
{
	int ret;
	char *ret_string;
	BIO * bio_err = NULL;
	X509 * cert = NULL;
	X509_STORE * store = NULL;
	X509_LOOKUP *lookup = NULL;
	X509_STORE_CTX *store_ctx = NULL;
	BIO *bio_cert = NULL;

	/*
	printf("CA cert file?\n");
	scanf("%s", CA_CERT_FILE);

	printf("cert file?\n");
	scanf("%s", CERT_FILE);
	*/

	OpenSSL_add_all_algorithms();

	bio_err = BIO_new( BIO_s_file() );
	if (bio_err) {
		BIO_set_fp(bio_err, stderr, BIO_NOCLOSE|BIO_FP_TEXT);
	}

	// 인증할 인증서 읽기 위한 BIO 생성
	bio_cert = BIO_new( BIO_s_file() );
	if (BIO_read_filename( bio_cert, CERT_FILE ) <= 0) {
		BIO_printf(bio_err, "can not read cert file - %s\n", CERT_FILE);
		ERR_print_errors(bio_err);
		exit(1);
	}

	// 인증할 인증서를 파일로 부터 읽어 X509 구조체로 변환
	cert = PEM_read_bio_X509( bio_cert, NULL, NULL, NULL );
	if (!cert) {
		BIO_printf(bio_err, "error road x509 cert - %s\n", CERT_FILE);
		ERR_print_errors(bio_err);
		exit(1);
	}

	// 인증서를 저장할 STORE 구조체 생성
	store = X509_STORE_new();
	if (store == NULL) {
		BIO_printf(bio_err, "error X509_STORE_new\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	// 콜백함수 설정
	X509_STORE_set_verify_cb_func( store, verify_cb_func );


	// 파일로 부터 CA 인증서 읽음
	if (!X509_STORE_load_locations( store, CA_CERT_FILE, NULL )) {
		BIO_printf(bio_err, "error unable road CA cert\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	// STORE에 CA 인증서 추가
	lookup = X509_STORE_add_lookup( store, X509_LOOKUP_file() );
	if (lookup == NULL) {
		BIO_printf(bio_err, "error X509_STORE_add_lookup\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	// road CRL file
	if (!X509_load_crl_file( lookup, CRL_FILE, X509_FILETYPE_PEM)) {
		BIO_printf(bio_err, "error unable road CRL\n");
		/*
		ERR_print_errors(bio_err);
		exit(1);
		*/
	} else {
		// CA 인증서, CRL 인증 모두 지원
		X509_STORE_set_flags( store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
	}

	// STORE 컨텍스트 생성
	store_ctx = X509_STORE_CTX_new();
	if (store_ctx == NULL) {
		BIO_printf(bio_err, "error X509_STORE_CTX_new\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	// X509_STORE_CTX 구조체에 지금까지 읽은 모든 인증서와 CRL을 저장
	if (!X509_STORE_CTX_init( store_ctx, store, cert, NULL )) {
		BIO_printf(bio_err, "error X509_STORE_CTX_init\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	ret = X509_verify_cert( store_ctx );
	if (ret == 1) {
		BIO_printf(bio_err, "Verify OK\n");
	}else
	{
		BIO_printf(bio_err, "Unable verify\n");
		ERR_print_errors( bio_err );
	}
}


