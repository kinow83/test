/*
 * openssl_client.c
 *
 *  Created on: 2017. 6. 7.
 *      Author: root
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
#include <pthread.h>
#include <netdb.h>

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define DEBUG(fmt, ...) printf("[kinow]"#fmt"\n", ##__VA_ARGS__)

#define likely(x) __glibc_likely(x)
#define unlikely(x) __glibc_unlikely(x)

#define CHECK_NULL(x) {if (unlikely(x==NULL)) abort();}
#define CHECK_ERR(e,s) {if (unlikely(e!=1)) {perror(s); abort();}}
#define CHECK_SSL(e) {if (unlikely(e!=1)) {ERR_print_errors_fp(stderr); abort();}}

int _gethostaddr(const char* hostname, struct in_addr *addr)
{
	struct hostent *h;
	struct in_addr **addrs;
	int i;

	h = gethostbyname(hostname);
	if (!h) return -1;
	addrs = (struct in_addr **)h->h_addr_list;

	for (i=0; addrs[i]; i++) ;
	printf("%s ip resovled count =%d\n", hostname, i);

	for (i=0; addrs[i]; i++) {
		memcpy(addr, addrs[i], sizeof(*addr));
		break;
	}
	return 0;
}

int main(int argc, char **argv)
{
	SSL_CONF_CTX *cctx = NULL;
	SSL_CTX *ctx = NULL;
	SSL *ssl = NULL;
	X509 *server_cert;
	const SSL_METHOD *method;
	const char *ca_file          = "pem/rootca.crt";
	const char *client_cert_file = "pem/client.crt";
	const char *client_key_file  = "pem/client.key.without_pass";
	int err;
	int sock;
	struct sockaddr_in saddr;
	char *server_host = argv[1];
	char *server_port = argv[2];

	// init SSL
	{
		// mandatory and missing from some examples
		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
		OpenSSL_add_all_ciphers();
		OpenSSL_add_all_digests();
		method = SSLv23_client_method();
		ctx = SSL_CTX_new(method);
		CHECK_NULL(ctx);
	}
	// load cert file and key file
	if (0) {
#if 0
		err = SSL_CTX_use_certificate_file(ctx, ca_file, SSL_FILETYPE_PEM);
		CHECK_SSL(err);
#endif
		err = SSL_CTX_use_certificate_file(ctx, client_cert_file, SSL_FILETYPE_PEM);
		CHECK_SSL(err);
		err = SSL_CTX_use_PrivateKey_file(ctx, client_key_file, SSL_FILETYPE_PEM);
		CHECK_SSL(err);
	}
	// verify key file
	if (0) {
		err = SSL_CTX_check_private_key(ctx);
		CHECK_SSL(err);
	}
	// SSL conf ctx
	if (0) {
		cctx = SSL_CONF_CTX_new();
		CHECK_NULL(cctx);

		SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_FILE
		                                    | SSL_CONF_FLAG_CLIENT
		                                    | SSL_CONF_FLAG_SERVER);

		SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
		{
			err = SSL_CONF_cmd(cctx, "CipherString", "AES128-SHA256");
			if (err != 2) {
				DEBUG(unrecognised command);
				abort();
			}
		}
		SSL_CONF_CTX_finish(cctx);
	}
	// load ca file
	{
		SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(ca_file));

	}

	// create client socket
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) {
			CHECK_ERR(-1, "socket()");
		}

		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(atoi(server_port));
		_gethostaddr(server_host, &saddr.sin_addr);
		//saddr.sin_addr.s_addr = inet_addr(server_host);

		printf("connect to %s:%s\n", server_host, server_port);

		err = connect(sock, (struct sockaddr*)&saddr, sizeof(saddr));
		if (err < 0) {
			perror("connect()");
			CHECK_ERR(err, "connect()");
		}
	}
	DEBUG(connect OK);

	// TCP connection is ready. Do client side SSL
	{
		ssl = SSL_new(ctx);
		CHECK_NULL(ssl);

		// connect to server using SSL
		SSL_set_fd(ssl, sock);

		err = SSL_connect(ssl);
		CHECK_SSL(err);
	}

	DEBUG(SSL connection using: %s, SSL_get_cipher(ssl));

	int ssl_app_data1_idx = -1;
	int ssl_app_data2_idx = -1;
	// store app ex data
	{
		ssl_app_data1_idx = SSL_get_ex_new_index(0, "kinow app", NULL, NULL, NULL);
		DEBUG(ssl_app_data1_idx = %d, ssl_app_data1_idx);
		SSL_set_ex_data(ssl, ssl_app_data1_idx, "unetsystem1");

		ssl_app_data2_idx = SSL_get_ex_new_index(0, "kinow app", NULL, NULL, NULL);
		DEBUG(ssl_app_data2_idx = %d, ssl_app_data2_idx);
		SSL_set_ex_data(ssl, ssl_app_data2_idx, "unetsystem2");
	}

	{
		char * str;

		server_cert = SSL_get_peer_certificate(ssl);
		CHECK_NULL(server_cert);
		DEBUG(server cert info);

		X509_NAME *subject = X509_get_subject_name(server_cert);
		X509_NAME *issuer = X509_get_issuer_name(server_cert);

		str = X509_NAME_oneline(subject, 0, 0);
		CHECK_NULL(str);
		DEBUG(subject: %s, str);
		OPENSSL_free(str);

		str = X509_NAME_oneline(issuer, 0, 0);
		CHECK_NULL(str);
		DEBUG(issuer: %s, str);
		OPENSSL_free(str);

		X509_free(server_cert);
	}
	{
		STACK_OF(X509) *psk = sk_X509_new_null();
		STACK_OF(X509) *sk = SSL_get_peer_cert_chain(ssl);
		int n = sk_X509_num(sk);
		int i;
		printf("n = %d\n", n);
		for (i=0; i<n; i++) {
			X509 *xs = sk_X509_value(sk, i);
			X509_NAME *sj, *is;
			sj = X509_get_subject_name(xs);
			is = X509_get_issuer_name(xs);
			if (!sj || !is) {
				printf("null %d\n", i);
			} else {
				printf("%d: %s: %s\n", i, X509_NAME_oneline(sj, 0, 0), X509_NAME_oneline(is, 0, 0));
			}

			if (i > 0) {
				sk_X509_push(psk, sk_X509_dup(xs));
			}

			BIO *out;
			char outfile[256];
			snprintf(outfile, sizeof(outfile), "cert_%d.pem", i);
			out = BIO_new(BIO_s_file());
			BIO_write_filename(out, outfile);

			PEM_write_bio_X509(out, xs);
			BIO_free_all(out);

		}

		printf("psk n = %d==========================================\n", sk_X509_num(psk));
		for (i=0; i<sk_X509_num(psk); i++) {
			X509 *xs = sk_X509_value(sk, i);
			X509_NAME *sj, *is;
			sj = X509_get_subject_name(xs);
			is = X509_get_issuer_name(xs);
			if (!sj || !is) {
				printf("null %d\n", i);
			} else {
				printf("%d: %s: %s\n", i, X509_NAME_oneline(sj, 0, 0), X509_NAME_oneline(is, 0, 0));
			}
		}
		sk_X509_free(psk);
		printf("========================================================\n");
	}

	DEBUG(===============================================);
	if (0) {
		char buf[1024];
		const char *msg = "I'm client hello~ server";
		err = SSL_write(ssl, msg, strlen(msg));
		if (err <= 0) {
			CHECK_SSL(err);
		}

		err = SSL_read(ssl, buf, sizeof(buf)-1);
		if (err <= 0) {
			CHECK_SSL(err);
		}
		buf[err] = 0;
		DEBUG(Get %d chars: %s, err, buf);

		// close SSL connection
	}
	SSL_shutdown(ssl);

	if (0) {
		void *ssl_app_data1 = SSL_get_ex_data(ssl, ssl_app_data1_idx);
		void *ssl_app_data2 = SSL_get_ex_data(ssl, ssl_app_data2_idx);


		DEBUG(ssl_app_data1 = %s (%p), (char*)ssl_app_data1, ssl_app_data1);
		DEBUG(ssl_app_data2 = %s (%p), (char*)ssl_app_data2, ssl_app_data2);
	}

	close(sock);
	if (ssl) SSL_free(ssl);
	if (ctx) SSL_CTX_free(ctx);
	if (cctx) SSL_CONF_CTX_free(cctx);
	ERR_free_strings();

	return 0;
}
