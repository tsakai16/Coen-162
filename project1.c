
/* Tate Sakai
 Code Project 1
 */


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>


/*
 steps:
 open socket
 connect to server
 recv/read & send/write
 close socket
 */


int main (int argc, char *argv[]) {

	if (argc != 3) {
		printf ("USAGE: EXECUTABLE HOSTNAME FILEPATH");
		return -1;
	}

	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd = 0;
	int bytes, sent, received, total;
	char response[4096];

	char req[4096];
	memset(req, '0', sizeof(req));
	sprintf(req, "GET %s HTTP/1.0\r\n\r\n", argv[2]);

	printf("Request constructed: \n%s\n", req);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error: Could not create socket \n");
		return 1;
	}

	if ((server = gethostbyname(argv[1])) == NULL) {
		printf("Error: no such host\n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

	if ( connect (sockfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0 )
	{
		printf ("Error : Connect Failed \n");
		return 1;
	} 

	if (send(sockfd, req, strlen(req), 0) < 0) {
		printf("Send failed...\n");
	}

	memset(response, 0, sizeof(response));
	total = sizeof(response)-1;
	received = 0;
	printf("Response: \n");
	while (1) {
		printf("%s", response);
		memset(response, 0, sizeof(response));
		bytes = recv(sockfd, response, 1024, 0);
		if (bytes < 0)
			printf("ERROR: can't read socket");
		if (bytes == 0)
			break;
		received+=bytes;
	}

	if (received == total)
		printf("ERROR: part of message lost");


	close(sockfd);
	return 0;

}
