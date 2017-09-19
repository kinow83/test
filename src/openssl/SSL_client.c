/*
 * SSL_client.c
 *
 *  Created on: 2016. 10. 27.
 *      Author: root
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>

// gcc SSL_client.c -o ssl_client -lssl -lcrypto
int main(int argc, char **argv)
{
	BIO *bio_err = NULL;
	const char *server_name = "10.10.200.150";
	unsigned short port = 8443;
	struct sockaddr_in server_addr;
	struct hostent *host;
	int socket_type = SOCK_STREAM;
	int ret;
	int server_socket;

	const SSL_METHOD *method = NULL;
	SSL_CTX *ctx = NULL;
	SSL* ssl = NULL;
	X509 * server_cert = NULL;

	//##############################################################################
	// SSL build
	//##############################################################################

	// 화면 출력 BIO 생성
	bio_err = BIO_new( BIO_s_file() );
	if (bio_err) {
		BIO_set_fp(bio_err, stderr, BIO_NOCLOSE | BIO_FP_TEXT);
	}

	SSL_library_init();
	// 모든 에러 스트링 로드
	SSL_load_error_strings();
	// 모든 알고리즘 로드
	SSLeay_add_ssl_algorithms();
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();

	// SSL version 3
	method = SSLv23_method();

	// create SSL context
	ctx = SSL_CTX_new( method );
	if (ctx == NULL) {
		BIO_printf(bio_err, "error SSL_CTX_new\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	if (isalpha(server_name[0])) {
		printf("server name is hostname - %s\n", server_name);
		host = gethostbyname(server_name);
	} else {
		printf("server name is ip - %s\n", server_name);
		struct sockaddr_in addr;
		addr.sin_addr.s_addr = inet_addr(server_name);
		host = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
		if (host == NULL) {
			fprintf(stderr, "%s\n", hstrerror(h_errno));
		}
		printf("%p\n", host);
	}
	if (host == NULL) {
		fprintf(stderr, "unknown host - %s\n", server_name);
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
//	h_addr_list
//	- An array of pointers to network addresses for the host (in network byte order),
//	  terminated by a null pointer.
	memcpy( &server_addr.sin_addr, host->h_addr_list[0], host->h_length );
//	h_addrtype
//	- The type of address; always AF_INET or AF_INET6 at present.
	server_addr.sin_family = host->h_addrtype;
	server_addr.sin_port = htons( port );

	server_socket = socket(AF_INET, socket_type, 0);
	if (server_socket < 0) {
		fprintf(stderr, "error socket()\n");
		exit(1);
	}

	printf("[%s] connecting to server...\n", server_name);
	if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		fprintf(stderr, "error connect()\n");
		exit(1);
	}

//	RAND_screen();

	ssl = SSL_new( ctx );
	if (ssl == NULL) {
		BIO_printf(bio_err, "error SSL_new\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	SSL_set_fd( ssl, server_socket );
	ret = SSL_connect( ssl );
	if (ret < 0) {
		BIO_printf(bio_err, "error SSL_connect\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	const char *current_cipher = SSL_CIPHER_get_name ( SSL_get_current_cipher(ssl) );
	printf("SSL current algorithm - %s\n", current_cipher);

	server_cert = SSL_get_peer_certificate( ssl );
	if (server_cert == NULL) {
		BIO_printf(bio_err, "error SSL_get_peer_certificate\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	printf("server certificate:\n");

	char * desc = NULL;

	// get subject DN
	desc = X509_NAME_oneline( X509_get_subject_name(server_cert), NULL, 0 );
	if (desc == NULL) {
		BIO_printf(bio_err, "error get subject DN of server certificate\n");
		ERR_print_errors(bio_err);
		exit(1);
	}
	printf("\tsubject: %s\n", desc);
	free(desc);

	// get issuer DN
	desc = X509_NAME_oneline( X509_get_issuer_name(server_cert), NULL, 0 );
	if (desc == NULL) {
		BIO_printf(bio_err, "error get issuer DN of server certificate\n");
		ERR_print_errors(bio_err);
		exit(1);
	}
	printf("\tissuer: %s\n", desc);
	free(desc);

	X509_free( server_cert );

	char buff[1000];
	char message[100] = "This is client message.";
	ret = SSL_write( ssl, message, strlen(message) );
	if (ret == -1) {
		BIO_printf(bio_err, "error SSL_write\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	ret = SSL_read( ssl, buff, sizeof(buff)-1 );
	if (ret == -1) {
		BIO_printf(bio_err, "error SSL_read\n");
		ERR_print_errors(bio_err);
		exit(1);
	}
	buff[ret] = 0;
	printf("message from server(%d): %s\n", ret, buff);

	SSL_shutdown( ssl );

	close(server_socket);
	SSL_free( ssl );
	SSL_CTX_free( ctx );

	return 0;

}

