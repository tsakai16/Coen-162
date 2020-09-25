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
#include <stdbool.h>


#define INIT_RESP 10000

int createConnection(unsigned long *ip, int port);
char *retriever(int sock, int *clientSock, struct sockaddr_in *client);
char *parse(char *request);
unsigned long getIp(char *domain);
int makeConnection(unsigned long *ip, int port);
char *sender(int sock, char *message);


int main(int argc, char *argv[]) {

	char **fp, *filename, **hostP, *host, *message, *request, *response, *content;
	unsigned long ip, local_host;
	int serverSock, browserSock, clientSock, port = 8080;
	struct sockaddr_in client;

	fp = &filename;
	hostP = &host;

	if (argc > 2) {
		printf ("USAGE: EXECUTABLE <PORT>");
		return -1;
	}
	if(argc > 1)
		port = atoi(argv[1]);
	
	local_host = inet_addr("127.0.0.1");	
	browserSock = createConnection(&local_host, port);
	if(browserSock == -1) {
		printf("Error: Bind to browser failed\n");
		return 1;
	}

	listen(browserSock, 1);
	request = retriever(browserSock, &clientSock, &client);
	if(request == NULL) {
		printf("Error: Failed to get request from browser\n");
		return 1;
	}

	host = parse(request);

	while(1){
        if((ip = getIp(host) )== (unsigned long) -1) {
            printf("Error: Failed to evaluate server address\n");
            return 1;
        }	
	
        if((serverSock = makeConnection(&ip, 80)) == -1) {
            printf("Error: Could not connect to server\n");
            return 1;
        }

        response = sender(serverSock, request);
        
        if(content != NULL) {
            content += 4;
        } else {
            content = response;
        }

        send(clientSock, content, INIT_RESP, 0);
    }
	close(clientSock);
	close(serverSock);
	close(browserSock);
	return 0;
	
}



int createConnection(unsigned long *ip, int port) {
	struct sockaddr_in server;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error: Failed to create socket\n");
		return -1;
	}
	server.sin_addr.s_addr = *ip;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		printf("Error: Failed to bind to socket\n");
		return -1;
	}
	else {
		return sockfd;
	}
}


char *retriever(int sock, int *clientSock, struct sockaddr_in *client) {
	char *response;
	int n;
	int clientLength = sizeof(struct sockaddr_in);
	response = (char *) malloc(INIT_RESP);
	*clientSock = accept(sock, (struct sockaddr *) client, (socklen_t*)&clientLength);
	if(sock < 0) {
		printf("Error: Accept failed\n");
		return NULL;
	}
	n = recv(*clientSock, response, INIT_RESP, 0);
	response[n] = '\0';
	printf("%d Bytes read\nResponse:\n%s\n", n, response);

	if(strlen(response) <= 0) {
		printf("Error: Could not read from socket\n");
		return NULL;
	}
	return response;
}

char *parse(char *request) {
	
	int i, length;
	char *first, *last, *parsed;

	first = strchr(request, '\n') + 1;
	first = strchr(first, ' ') + 1;
	last = strchr(first, '\n') - 2;
	length = last - first + 1;
	parsed = malloc(length + 1);
	strncpy(parsed, first, length);
	parsed[length] = '\0';
	return parsed;
}

unsigned long getIp(char *domain) {
	
	struct hostent *ip;
	void *address;

	if((ip = gethostbyname(domain)) == NULL) {
		printf("Error: Host name could not be found\n");
		return -1;
	}
	address = malloc(ip->h_length);
	memcpy(address, ip->h_addr_list[0], ip->h_length);	

	return *((unsigned long *) address);
}

int makeConnection(unsigned long *ip, int port) {
	struct sockaddr_in server;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error: Failed to create socket\n");
		return -1;
	}
	server.sin_addr.s_addr = *ip;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if(connect(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		printf("Error: Failed to connect to socket\n");
		return -1;
	}
	else {
		return sockfd;
	}
}

char *sender(int sock, char *message) {

	int n, i = 1;
	if((n = send(sock, message, strlen(message), 0)) < 0) {
		printf("Error: could not write to socket\n");
		return NULL;
    }
    char *response;
	response = (char *) malloc(INIT_RESP);
	n = recv(sock, response, INIT_RESP, 0);
	response[n + INIT_RESP * i] = '\0';

	if(n < 0) {
		printf("Error: Could not read from\n");
		return NULL;
	}
	return response;
}

