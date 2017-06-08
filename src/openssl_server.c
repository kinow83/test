/*
 * openssl_server.c
 *
 *  Created on: 2017. 6. 7.
 *      Author: root
 */
/*
 * ref openssl program
 * http://pchero21.com/?p=603
 *
 * ref make certifications using openssl
 * https://www.lesstif.com/pages/viewpage.action?pageId=6979614
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define REQUIRE_CLIENT_CERT
#define VERIFY_USE_CA_PATH
#define PRIVATE_KEY_ENCRYPTED

#define DEBUG(fmt, ...) printf("[kinow]"#fmt"\n", ##__VA_ARGS__)

#define likely(x) __glibc_likely(x)
#define unlikely(x) __glibc_unlikely(x)

#define CHECK_NULL(x) {if (unlikely(x==NULL)) abort();}
#define CHECK_ERR(e,s) {if (unlikely(e!=1)) {perror(s); abort();}}
#define CHECK_SSL(e) {if (unlikely(e!=1)) {ERR_print_errors_fp(stderr); abort();}}


static int my_verify_cert(X509_STORE_CTX* ctx, void *arg)
{
	int rc = X509_verify_cert(ctx);
	DEBUG(\t\tSSL_CTX_set_cert_verify_callback >>>>>>>>>>>>);
	DEBUG(X509_verify_cert = %d, rc);
	DEBUG(\t\tSSL_CTX_set_cert_verify_callback <<<<<<<<<<<<);
	return rc;
}

static int verify_cb(int ok, X509_STORE_CTX *ctx)
{
	DEBUG(call verify_cb: ok=%d, ok);
	X509_NAME *nm;
	X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
	int depth = X509_STORE_CTX_get_error_depth(ctx);

	DEBUG(\t\tSSL_CTX_set_verify >>>>>>>>>>>>>>>);
	DEBUG(\t\tissuer =%s depth=%d,
			X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0), depth);
	DEBUG(\t\tsubject=%s depth=%d,
			X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0), depth);
	DEBUG(\t\tSSL_CTX_set_verify <<<<<<<<<<<<<<<);
	return ok;
}

static int password_cb(char *buf, int size, int rwflag, void *userdata) {
	DEBUG(call password_cb userdata=%s size=%d rwflag=%d, (char*)userdata, size, rwflag);
	strncpy(buf, (char *)(userdata), size);
	buf[size - 1] = '\0';
	return (strlen((char *)buf));

}

int main(int argc, char **argv)
{
	int opt;
	int err;
	int ssock, csock;
	struct sockaddr_in saddr, caddr;
	socklen_t clilen;

	SSL_CTX *ctx;
	SSL *ssl;
	X509 *client_cert;
	const SSL_METHOD *method;

	const char *ca_file          =
#if 1
			"pem/rootca.crt";
#else
			"pem/rootca_server.crt";
#endif
	const char *ca_path = "pem";
	const char *server_cert_file = "pem/server.crt";
	const char *server_key_file  =
#ifdef PRIVATE_KEY_ENCRYPTED
			"pem/server.key";
#else
			"pem/server.key.without_pass";
#endif

	// init SSL
	{
		// mandatory and missing from some examples
		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
		OpenSSL_add_all_ciphers();
		OpenSSL_add_all_digests();

		method = SSLv23_server_method();
		ctx = SSL_CTX_new(method);
		CHECK_NULL(ctx);
	}
	// private key password callback (in case, private key encrypted by password)
#ifdef PRIVATE_KEY_ENCRYPTED
	{
		SSL_CTX_set_default_passwd_cb_userdata(ctx, "sqwe123");
		SSL_CTX_set_default_passwd_cb(ctx, password_cb);
	}
#endif
	// load cert file and key file
	{
		err = SSL_CTX_use_certificate_file(ctx, server_cert_file, SSL_FILETYPE_PEM);
		CHECK_SSL(err);
		DEBUG(SSL_CTX_use_certificate_file ---- OK);

		err = SSL_CTX_use_PrivateKey_file(ctx, server_key_file, SSL_FILETYPE_PEM);
		CHECK_SSL(err);
		DEBUG(SSL_CTX_use_PrivateKey_file ---- OK);
	}
	// verify key file
	{
		DEBUG(check private key);
		err = SSL_CTX_check_private_key(ctx);
		CHECK_SSL(err);
		DEBUG(SSL_CTX_check_private_key ---- OK);
	}
	// verify callback
	{

		int verify_mode  = 0;
#ifdef REQUIRE_CLIENT_CERT
		verify_mode = SSL_VERIFY_PEER
				| SSL_VERIFY_FAIL_IF_NO_PEER_CERT
				| SSL_VERIFY_CLIENT_ONCE;
#endif
		SSL_CTX_set_verify(ctx, verify_mode, verify_cb);
		SSL_CTX_set_cert_verify_callback(ctx, my_verify_cert, NULL);
	}
	// load ca file
	{
#ifdef VERIFY_USE_CA_FILE
		// using ca file
		SSL_CTX_load_verify_locations(ctx, ca_file, NULL);
#endif

#ifdef VERIFY_USE_CA_PATH
		// using ca path
		/**
		 * case 1. c_rehash command is hash naming file for certification
		 * ]# c_rehash pem/
		 * ]# ls pem/
lrwxrwxrwx 1 root root   10  6월  7 21:14 1df1f48b.0 -> server.crt
lrwxrwxrwx 1 root root   10  6월  7 21:14 9307c19f.0 -> client.crt
		 *
		 *
		 * case 2. direct make hash naming file
		 * ]# openssl x509 -in rootca.crt -noout -subject_hash
		 * ]# ln -s rootca.crt ca4eec93.0
		 */
		if (!SSL_CTX_load_verify_locations(ctx, NULL, ca_path) ||
				!SSL_CTX_set_default_verify_paths(ctx)) {
			ERR_print_errors_fp(stderr);
			abort();
		}
#endif
//		SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(ca_file));
	}



	// create server socket
	{
		ssock = socket(AF_INET, SOCK_STREAM, 0);
		if (ssock < 0) {
			CHECK_ERR(-1, "socket()");
		}

		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(18888);
		saddr.sin_addr.s_addr = INADDR_ANY;

		setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

		err = bind(ssock, (struct sockaddr*)&saddr, sizeof(saddr));
		if (err < 0) {
			CHECK_ERR(err, "bind()");
		}

		err = listen(ssock, 5);
		if (err < 0) {
			CHECK_ERR(err, "listen()");
		}
	}
	// accept from client
	{
		clilen = sizeof(caddr);
		DEBUG(wait for SSL client);
		csock = accept(ssock, (struct sockaddr*)&caddr, &clilen);
		if (csock < 0) {
			CHECK_ERR(-1, "accept()");
		}
	}
	DEBUG(accept OK);

	// TCP connection is ready. Do server side SSL
	{
		// get SSL session from SSL Context
		ssl = SSL_new(ctx);
		CHECK_NULL(ssl);

		SSL_set_fd(ssl, csock);

		// wait for connect SSL client
		err = SSL_accept(ssl);
		CHECK_NULL(ssl);
	}

	DEBUG(SSL connection using: %s, SSL_get_cipher(ssl));

	client_cert = SSL_get_peer_certificate(ssl);
	if (client_cert) {
		DEBUG(client cert info);
		X509_NAME *subject = X509_get_subject_name(client_cert);
		X509_NAME *issuer = X509_get_issuer_name(client_cert);
		char *str;

		str = X509_NAME_oneline(subject, 0, 0);
		CHECK_NULL(str);
		DEBUG(subject: %s, str);
		OPENSSL_free(str);

		str = X509_NAME_oneline(issuer, 0, 0);
		CHECK_NULL(str);
		DEBUG(issuer: %s, str);
		OPENSSL_free(str);

		X509_free(client_cert);
	} else {
		DEBUG(client does not have certification);
	}

	DEBUG(==============================================);
	{
		char buf[1024];
		err = SSL_read(ssl, buf, sizeof(buf)-1);
		if (err <= 0) {
			CHECK_SSL(err);
		}
		buf[err] = 0;
		DEBUG(Get %d chars: %s, err, buf);

		const char *repsmsg = "hello I'm Kang";
		err = SSL_write(ssl, repsmsg, strlen(repsmsg));
		if (err <= 0) {
			CHECK_SSL(err);
		}
	}

	close(csock);
	close(ssock);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();
//	SSL_CONF_CTX_free(cctx);

	return 0;
}
