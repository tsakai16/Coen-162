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

// struct cachedata{
//     char name[20];
//     char URL[200];
//     char data[20];
// }
struct cachedata {
	char *docPath;
	char *objName;
	char *URL;
	struct cachedata *next;
};
int createConnection(unsigned long *ip, int port);
char *retriever(int sock, int *clientSock, struct sockaddr_in *client);
char *parse(char *request);
unsigned long getIp(char *domain);
int makeConnection(unsigned long *ip, int port);
char *sender(int sock, char *message);

int main(int argc, char *argv[]) {

    struct sockaddr_in client;
    int serverSock, browserSock, clientSock, port = 8080, count = 0, exists = 0;
    unsigned long server, localhost;

    // struct cachedata cachearray[50];
    struct cachedata *head = NULL, *node = NULL;
    char **fpp, *name, *host, **hostP;
    FILE *fp;
    fpp = &name;
	hostP = &host;
    char *message, *request, *response, *content, *filepath;
    response = (char *) malloc(INIT_RESP);
	content = (char *) malloc(INIT_RESP * 2);

    // struct sockaddr_in client;

	char *path = NULL;
	if (argc > 3) {
		printf ("USAGE: EXECUTABLE <PORT> <FILEPATH>\n");
		printf ("If using filepath of cache folder, must specify port");
		return -1;
	}

    if(argc > 1)
        port = atoi(argv[1]);
	
	if(argc > 2){
		path = argv[2];
	}

    localhost = inet_addr("127.0.0.1");
    if ((browserSock = createConnection(&localhost, port)) == - 1){
        printf("Error: Bind to browser failed\n");
        return -1;
    }

        if ((request = retriever(browserSock, &clientSock, &client)) == NULL){
            printf("Error: Failed to read request from browser\n");
            return -1;
        }
        //host = parse(request);
    while(1){
        if ((server = getIp(host)) == (unsigned long) -1){
            printf("Error: Failed to get hostname\n");
            return -1;
        }
        if( (serverSock = makeConnection(&server, 80) ) == -1){
            printf("Error: Could not connect to server\n");
            return -1;
        }
        //if cache empty ==> Array or linked list easier?
        if(node == NULL){
            //create cache struct
            node = (struct cachedata *) malloc(sizeof(struct cachedata));
			if (path == NULL)
            	node->docPath = "./cache/document00";
			else
				node->docPath = path;
            node->objName = parse(name);
            node->URL = parse(host);
            head = node;
            count = 0;
            exists = 0;
        }
        else {
        //else - cache is non-empty
            exists = 0;
			node = head;
            //search through cache to see if file is there
            //if found send obj to broswer
            do {

                if(strcmp(node->objName, name) == 0 && strcmp(node->URL, host) == 0) {
                    //if in cache send to client socket
                
                    //need to change this to modified get
                    //Send if-modified-since GET to server
                    //if modified
                        //retrieve new version,
                        //save to cache
                        //send obj to browser

                    //else send obj from cache to browser
                    
                    exists = 1;
                    fp = fopen(node->docPath, "r");
                    while(fgets(response, INIT_RESP, fp)) {
                        strcat(content, response);
                    }
                    fclose(fp);
                    send(clientSock, content, sizeof(content), 0);

                    fclose(fp);
                    close(clientSock);
                }

                else {
                    node = node->next;
                    count ++;
                }
            } while (node->next != NULL);


            //if not in cache
            if(exists == 0) {

				//Store in cache and send

                //send get request to server
                //save in cache
                //send object to browser
				response = sender(serverSock, request);
				send(clientSock, response, sizeof(response), 0);

				node->next = (struct cachedata *) malloc(sizeof(node));
				node->objName = name;
				node->URL = host;

				if (path = NULL){
					filepath = (char *) malloc(sizeof("./cache/document00"));
					sprintf(filepath, "./cache/document%02d", count);
				}else{
					filepath = (char *) malloc(sizeof("%s/document00", path);
					sprintf(filepath, "%s/document%02d", path, count);
				}
				node->docPath = filepath;
				fp = fopen(filepath, "w+");
				fwrite(response, 1, strlen(response), fp);
				fclose(fp);
			}


        }


        close(serverSock);
    }

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