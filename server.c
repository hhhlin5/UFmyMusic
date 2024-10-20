/*Included libraries*/

#include <stdio.h>	        /* for printf() and fprintf() */
#include <sys/socket.h>	    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	        /* supports all sorts of functionality */
#include <unistd.h>	        /* for close() */
#include <string.h>	        /* support any string ops */

/* Constants */
#define PORT 8080           
#define BUFFER_SIZE 1024	    /* The send buffer size */

/* Functions */


/* The main function */
int main(){
    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in serverAddr;		/* Local address */
    struct sockaddr_in clientAddr;		/* Client address */
    unsigned int addrLen;        /* Length of address data struct */

    /* Create new TCP Socket for incoming requests*/
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket() failed\n");
        exit(1);
    }
    
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    /* Bind to local address structure */
    if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
      printf("bind() failed\n"); 
      exit(1);
    }
  
    /* Listen for incoming connections */
    if (listen(serverSock, 5) < 0){
        printf("listen() failed\n");
        exit(1);
    }
    printf("Server is listening on port %d...\n", PORT); 

    /* Loop server forever*/
    while(1){
        /* Accept incoming connection */
        addrLen = sizeof(clientAddr); 
        if ((clientSock = accept(serverSock, (struct sockaddr * ) &clientAddr, &addrLen)) < 0){
            printf("accept() failed\n");
            exit(1);
        }
       
    }
    close(serverSock); 
    return 0; 
}