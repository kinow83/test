#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char *server = NULL;

void netstat(void)
{
	char buf[1024];
	//if (server) sprintf(buf, "netstat -atunp |grep \"%s\"", server);
	if (server) sprintf(buf, "netstat -atunp |grep TIME_WAIT");
	system(buf);
}

void error(const char *msg)
{
	perror(msg);
	netstat();
	printf("ERROR!!!\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd;
	socklen_t serv_len, cli_len;
	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 3) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	server = argv[2];

	// net.ipv4.ip_local_port_range = 32768    60999
	system("sysctl -w net.ipv4.ip_local_port_range=\"60000	60001\" > /dev/null");

	serv_len = sizeof(serv_addr);
	cli_len = sizeof(cli_addr);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
	serv_addr.sin_port = htons(atoi(argv[3]));

	bzero((char *) &cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = inet_addr(argv[1]);
	cli_addr.sin_port = 0;

	int on = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0)
		error("ERROR on SO_REUSEADDR");
	on = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		error("ERROR on SO_REUSEADDR");

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) 
		error("ERROR on binding");

	if (getsockname(sockfd, (struct sockaddr *)&cli_addr, &cli_len) < 0)
		error("ERROR on getsockname");

	printf("local port: %d\n", ntohs(cli_addr.sin_port));

	if (connect(sockfd, (struct sockaddr *)&serv_addr, serv_len) < 0)
		error("ERROR on connect");

	netstat();
	close(sockfd);

	printf("SCCESS!!!\n");
	return 0; 
}
