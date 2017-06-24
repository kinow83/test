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


int main(int argc, char **argv)
{
	SSL_CONF_CTX *cctx;
	SSL_CTX *ctx;
	SSL *ssl;
	X509 *server_cert;
	const SSL_METHOD *method;
	const char *ca_file          = "pem/rootca.crt";
	const char *client_cert_file = "pem/client.crt";
	const char *client_key_file  = "pem/client.key.without_pass";
	int err;
	int sock;
	struct sockaddr_in saddr;
	char *server_host = argv[1];

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
	{
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
	{
		err = SSL_CTX_check_private_key(ctx);
		CHECK_SSL(err);
	}
	// SSL conf ctx
	{
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
		saddr.sin_port = htons(18888);
		saddr.sin_addr.s_addr = inet_addr(server_host);

		err = connect(sock, (struct sockaddr*)&saddr, sizeof(saddr));
		if (err < 0) {
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

	DEBUG(===============================================);
	{
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
		SSL_shutdown(ssl);
	}

	{
		void *ssl_app_data1 = SSL_get_ex_data(ssl, ssl_app_data1_idx);
		void *ssl_app_data2 = SSL_get_ex_data(ssl, ssl_app_data2_idx);


		DEBUG(ssl_app_data1 = %s (%p), (char*)ssl_app_data1, ssl_app_data1);
		DEBUG(ssl_app_data2 = %s (%p), (char*)ssl_app_data2, ssl_app_data2);
	}

	close(sock);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	SSL_CONF_CTX_free(cctx);
	ERR_free_strings();

	return 0;
}
