/*
 * SSL_server.c
 *
 *  Created on: 2016. 10. 26.
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

BIO * bio_err;

#define CERT_FILE    "CA/rootcert.pem"
#define PRIVKEY_FILE "CA/rootkey.pem"

void ssl_info_cb( const SSL *s, int where, int ret)
{
	char * write_string;
	int w;

	// 현재 어떤 메시지 교환 과정인지를 나타냄
	w = where & ~SSL_ST_MASK;

	if (w & SSL_ST_CONNECT)
		write_string = "SSL_connect";
	else if (w & SSL_ST_ACCEPT)
		write_string = "SSL_accept";
	else
		write_string = "undefined";

	// 일반적인 핸드쉐이크 프로토콜 메시지일 경우
	if (where & SSL_CB_LOOP)
		BIO_printf( bio_err, "%s:%s\n", write_string, SSL_state_string_long(s) );
	// alert 프로토콜일 경우
	else if (where & SSL_CB_ALERT) {
		write_string = (where & SSL_CB_READ) ? "read" : "write";
		BIO_printf( bio_err, "SSL3 alert %s:%s:%s\n",
				write_string,
				SSL_alert_type_string_long(ret),
				SSL_alert_desc_string_long(ret) );
	}
	// 종료 과정일 경우
	else if (where & SSL_CB_EXIT) {
		// failure
		if (ret == 0)
			BIO_printf( bio_err, "%s:failed in %s\n", write_string, SSL_state_string_long(s) );
		// error
		else if (ret < 0)
			BIO_printf( bio_err, "%s:error in %s\n", write_string, SSL_state_string_long(s) );
	}
}

int main(int argc, char **argv)
{
	unsigned short port = 8443;
	char * server_address = "10.10.200.150";
	int socket_type = SOCK_STREAM;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int server_socket;
	int client_socket;

	const SSL_METHOD * method;
	SSL_CTX * ctx;
	SSL * ssl;
	int ret;


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
	// SSL 핸드쉐이크 메시지 교환 과정을 알려주는 콜백함수 셋팅
	SSL_CTX_set_info_callback( ctx, ssl_info_cb );
	// 자신의 인증서를 파일에서 로딩한다.
	if (SSL_CTX_use_certificate_file( ctx, CERT_FILE, SSL_FILETYPE_PEM ) <= 0) {
		ERR_print_errors(bio_err);
		exit(1);
	}
	// 자신의 개인키를 파일에서 로딩한다.
	if (SSL_CTX_use_PrivateKey_file( ctx, PRIVKEY_FILE, SSL_FILETYPE_PEM ) <= 0) {
		ERR_print_errors(bio_err);
		exit(1);
	}
	// 읽은 인증서와 개인키가 맞는지 확인 한다.
	if (!SSL_CTX_check_private_key( ctx )) {
		BIO_printf(bio_err, "error mismatch private key with certificate\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	//##############################################################################
	// Socket build
	//##############################################################################
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( server_address );
	server_addr.sin_port = htons( port );

	server_socket = socket(AF_INET, socket_type, 0);
	if (server_socket < 0) {
		fprintf(stderr, "error socket\n");
		exit(1);
	}
	ret = bind( server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr) );
	if (ret < 0) {
		fprintf(stderr, "error bind\n");
		exit(1);
	}
	ret = listen( server_socket, 5 );
	if (ret < 0) {
		fprintf(stderr, "error listen\n");
		exit(1);
	}

	printf("%s:%d listen...\n", server_address, port);
	int client_addlen = sizeof(client_addr);

	client_socket = accept( server_socket, (struct sockaddr*)&client_addr, &client_addlen );
	if (client_socket < 0) {
		fprintf(stderr, "error accept\n");
		exit(1);
	}

	printf("connected client %s:%d\n",
			inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));

	ssl = SSL_new( ctx );
	if (ssl == NULL) {
		BIO_printf(bio_err, "error SSL_new\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	// associate socket and SSL
	SSL_set_fd ( ssl, client_socket );

	ret = SSL_accept( ssl );
	if (ret < 0) {
		BIO_printf(bio_err, "error SSL_accept\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	const char *currnet_cipher = SSL_CIPHER_get_name( SSL_get_current_cipher( ssl ) );
	printf("SSL cipher argorithims = %s\n", currnet_cipher);

	char in_buf[1000];
	ret = SSL_read( ssl, in_buf, sizeof(in_buf)-1 );
	if (ret < 0) {
		BIO_printf(bio_err, "error SSL_read\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	in_buf[ret] = 0;
	printf("recv data(%d): %s\n", ret, in_buf);

	// server response message buffer
	char message[100] = "This is server response message.";

	ret = SSL_write( ssl, message, strlen(message) );
	if (ret < 0) {
		BIO_printf(bio_err, "error SSL_write\n");
		ERR_print_errors(bio_err);
		exit(1);
	}

	printf("close SSL\n");
	close(client_socket);
	close(server_socket);
	SSL_free( ssl );
	SSL_CTX_free( ctx );

}





