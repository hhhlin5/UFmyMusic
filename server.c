/*Included libraries*/

#include <stdio.h>			/* For printf() and fprintf() */
#include <sys/socket.h>		/* For socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		/* For sockaddr_in and inet_addr() */
#include <stdlib.h>			/* Supports all sorts of functionality */
#include <unistd.h>			/* For close() */
#include <string.h>			/* Support any string ops */

#include <pthread.h>		/* For thread() */
#include <dirent.h>			/* Support any file ops */

/* Constants */
#define PORT 8080		   
#define BUFFER_SIZE 1024		/* The send buffer size */

/* Globals */
typedef struct {
    int sock;
    char client_ip[INET_ADDRSTRLEN]; // For storing client IP
    int client_port;                 // For storing client port
} client_info_t;

pthread_mutex_t lock;

/* Functions */
void *handle_client(void *socket_desc);
void list_files(int client_sock);
void send_diff(int client_sock, char *client_files);
void send_files(int client_sock, char *requested_files);
void log_client_activity(client_info_t *client_info, const char *message);


/* The main function */
int main(){
	int serverSock;						/* Server Socket */
	int clientSock;						/* Client Socket */
	struct sockaddr_in serverAddr;		/* Local address */
	struct sockaddr_in clientAddr;		/* Client address */
	unsigned int addrLen;				/* Length of address data struct */
	
	// Initialize mutex
    pthread_mutex_init(&lock, NULL);

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
	addrLen = sizeof(struct sockaddr_in);

	// Handle multiple clients using threads
	while ((clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &addrLen))) {
		pthread_t client_thread;

		// Allocate memory for client info
		client_info_t *client_info = malloc(sizeof(client_info_t));
		client_info->sock = clientSock;
		
		// Get client IP and port
		inet_ntop(AF_INET, &(clientAddr.sin_addr), client_info->client_ip, INET_ADDRSTRLEN);
		client_info->client_port = ntohs(clientAddr.sin_port);

		// Log client connection
		log_client_activity(client_info, "Client connected");

		if (pthread_create(&client_thread, NULL, handle_client, (void*)client_info) < 0) {
			printf("Could not create thread/n");
			free(client_info);
			return 1;
		}
	}

	if (clientSock < 0) {
		printf("Accept failed\n");
		return 1;
	}
	
	// Cleanup
	pthread_mutex_destroy(&lock);
	close(serverSock); 
	return 0; 
}

// Handle client communication
void *handle_client(void *client_info_ptr) {
    client_info_t client_info;
    memcpy(&client_info, client_info_ptr, sizeof(client_info_t));
    free(client_info_ptr);
    char client_message[BUFFER_SIZE];

    while (recv(client_info.sock, client_message, sizeof(client_message), 0) > 0) {
        // Handle messages from the client
        if (strncmp(client_message, "LIST", 4) == 0) {
            list_files(client_info.sock);
        } else if (strncmp(client_message, "DIFF", 4) == 0) {
            char *client_files = client_message + 5; // Assuming file list follows 'DIFF '
            send_diff(client_info.sock, client_files);
        } else if (strncmp(client_message, "PULL", 4) == 0) {
            char *requested_files = client_message + 5; // Assuming file list follows 'PULL '
            send_files(client_info.sock, requested_files);
        } else if (strncmp(client_message, "LEAVE", 5) == 0) {
            log_client_activity(&client_info, "Client disconnected");
            break; // End session
        }
    }

    close(client_info.sock);
    pthread_exit(NULL);
}

// List files available on the server
void list_files(int client_sock) {
    DIR *dir;
    struct dirent *ent;
    char file_list[BUFFER_SIZE] = {0};

    if ((dir = opendir(".")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // Only regular files
                strcat(file_list, ent->d_name);
                strcat(file_list, "\n");
            }
        }
        closedir(dir);
    } else {
        printf("Could not open directory/n");
        return;
    }

    send(client_sock, file_list, strlen(file_list), 0);
}

// Send diff of files between server and client
void send_diff(int client_sock, char *client_files) {
    // Here you should compare the server's files with the client's files
    // and generate a diff (simplified for the example)
    char diff_message[BUFFER_SIZE] = "Diff: No differences\n";
    send(client_sock, diff_message, strlen(diff_message), 0);
}

// Send files requested by the client
void send_files(int client_sock, char *requested_files) {
    // Parse the requested file list and send the requested files
    char file_message[BUFFER_SIZE] = "Sending requested files\n";
    send(client_sock, file_message, strlen(file_message), 0);
}

// Log client activity
void log_client_activity(client_info_t *client_info, const char *message) {
    pthread_mutex_lock(&lock);

    FILE *log_file = fopen("client_log.txt", "a");
    if (log_file != NULL) {
        fprintf(log_file, "Client (%s:%d): %s\n", client_info->client_ip, client_info->client_port, message);
        fclose(log_file);
    } else {
        perror("Could not open log file");
    }

    pthread_mutex_unlock(&lock);
}
