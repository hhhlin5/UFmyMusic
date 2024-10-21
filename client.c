/* Included libraries */
#include <stdio.h>			/* For printf() and fprintf() */
#include <sys/socket.h>		/* For socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		/* For sockaddr_in and inet_addr() */
#include <stdlib.h>			/* Supports all sorts of functionality */
#include <unistd.h>			/* For close() */
#include <string.h>			/* Support any string ops */

#include <dirent.h>			/* Support any file ops */
#include <openssl/evp.h>	/* Support md5 */

/* Constants */
#define PORT 8888
#define BUFFER_SIZE 1024		/* The send buffer size */

/* Globals */
typedef struct {
	int num_files;			// Number of files
	char **file_names;		// Array of file names
	unsigned char **md5s;		// Array of md5s
} file_list_t;

file_list_t* file_list;
char PATH[100] = "./music_client";

/* Functions */
void showMenu();
void list_files(int clientSock);
void diff_files(int clientSock);
void pull_files(int clientSock);
file_list_t *receive_file_list(int clientSock);
void create_file_list(const char *dir_path);
void free_file_list(file_list_t *file_list_ptr);

/* The main function */
int main() {
	int clientSock;					/* socket descriptor */
	struct sockaddr_in serv_addr;   /* The server address */

	/* Create a new TCP socket*/
	if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("socket() failed\n");
		return 1;
	}

	/* Construct the server address structure */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(PORT); 

	/* Establish connecction to the server */
	if (connect(clientSock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Connection failed\n"); 
		return 1;
	}
	printf("Connected to the server.\n");
	create_file_list(PATH);

	showMenu();
	while (1) {
		printf("\nEnter your choice: ");
		int choice;
		scanf("%d", &choice);

		switch (choice) {
			case 1:
				list_files(clientSock);
				break;
			case 2:
				diff_files(clientSock);
				break;
			case 3:
				pull_files(clientSock);
				break;
			case 4:
				send(clientSock, "LEAVE", 5, 0);
				free_file_list(file_list);
				close(clientSock); 
				return 0;
			default:
				printf("Invalid choice. Try again.\n");
		}
	}
	
	return 0;
}

void showMenu() {
	printf("--- UFmyMusic Menu ---\n");
	printf("1. List Files\n");
	printf("2. Diff Files\n");
	printf("3. Pull File\n");
	printf("4. Leave\n");
}

// 1. List Files: Ask the server to return a list of files it currently has
void list_files(int clientSock) {
	send(clientSock, "LIST", 4, 0);
	
	file_list_t* server_file_list = receive_file_list(clientSock);
	
	printf("\nA List of Files that Server has:\n");
	for (int i = 0; i < server_file_list->num_files; i++) {
		printf("%s\n", server_file_list->file_names[i]);
	}

	free_file_list(server_file_list);
}

// 2. Diff (based on 1)): the client should show a "diff" of the files it has in comparison to the server);
void diff_files(int clientSock) {
	send(clientSock, "DIFF", 4, 0);
	
	file_list_t* server_file_list = receive_file_list(clientSock);
	
	// Compare remote with local
	int flag = 0;
	for (int i = 0; i < server_file_list->num_files; i++) {
		int found = 0;
		for (int j = 0; j < file_list->num_files; j++) {
			if (strncmp((const char*)(server_file_list->md5s[i]), (const char*)(file_list->md5s[j]), 16) == 0) {
				found = 1;
				break;
			}
		}
		if(!found) { // If local file not found
			if(!flag) {
				flag = 1;
				printf("\nMissing Files Compared to Server:\n");
			}
			printf("%s\n", server_file_list->file_names[i]);
		}
	}
	if(!flag) {
		printf("\nYou are up-to-date!\n");
	}
	
	free_file_list(server_file_list);
}

// 3. Pull (request all files identified in 2): from the server and store them locally)
void pull_files(int clientSock) {
	send(clientSock, "PULL", 4, 0);
	
	file_list_t* server_file_list = receive_file_list(clientSock);
	
	// Compare remote with local
	int flag = 0;
	for (int i = 0; i < server_file_list->num_files; i++) {
		int found = 0;
		for (int j = 0; j < file_list->num_files; j++) {
			if (strncmp((const char*)(server_file_list->md5s[i]), (const char*)(file_list->md5s[j]), 16) == 0) {
				found = 1;
				break;
			}
		}
		if(!found) { // If local file not found
			if(!flag) {
				flag = 1;
				printf("\nFetching Missing Files:\n");
			}
			printf("%s\n", server_file_list->file_names[i]);
			send(clientSock, &i, sizeof(i), 0);
			
			// Receive file size
			long file_size;
			recv(clientSock, &file_size, sizeof(file_size), 0);

			char buffer[BUFFER_SIZE];
			FILE *file;
			size_t bytes_received;
			long total_received = 0;
			
			// Open the file in write mode
			snprintf(buffer, sizeof(buffer), "%s/%s", PATH, server_file_list->file_names[i]);
			file = fopen(buffer, "wb");

			// Receive the file in chunks
			while (total_received < file_size && (bytes_received = recv(clientSock, buffer, sizeof(buffer), 0)) > 0) {
				fwrite(buffer, 1, bytes_received, file);
				total_received += bytes_received;
			}

			fclose(file);
		}
	}
	if(!flag) {
		printf("\nYou are up-to-date!\n");
	}
	else {
		printf("\nDone fetching!\n");
	}
	int i = -1;
	send(clientSock, &i, sizeof(i), 0);
	
	free_file_list(server_file_list);
	free_file_list(file_list);
	create_file_list("./music_client");
}

// Receive the file list from the server
file_list_t *receive_file_list(int clientSock) {
	file_list_t *server_file_list = malloc(sizeof(file_list_t));
	
	// Receive the number of files
	int num;
	recv(clientSock, &num, sizeof(num), 0);
	server_file_list->num_files = num;
	
	// Allocate memory for the file names and md5s
	server_file_list->file_names = malloc(num * sizeof(char *));
	server_file_list->md5s = malloc(num * sizeof(unsigned char *));
	
	// Receive each file info
	for (int i = 0; i < num; i++) {
		int len;
		recv(clientSock, &len, sizeof(len), 0);
		server_file_list->file_names[i] = malloc(len);
		recv(clientSock, server_file_list->file_names[i], len, MSG_WAITALL); // Receive the file name
		server_file_list->md5s[i] = malloc(16);
		recv(clientSock, server_file_list->md5s[i], 16, MSG_WAITALL); // Receive the file md5
	}
	
	return server_file_list;
}

// Create the file list
void create_file_list(const char *dir_path) {
	
	// Helper function to calculate MD5 hash of a file
	unsigned char *compute_md5(char *file_name) {
		FILE *file = fopen(file_name, "rb");

		EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
		EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

		unsigned char buffer[1024];
		int bytes;
		while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
			EVP_DigestUpdate(mdctx, buffer, bytes);
		}

		unsigned char *md5_hash = malloc(EVP_MD_size(EVP_md5()));
		EVP_DigestFinal_ex(mdctx, md5_hash, NULL);

		fclose(file);
		EVP_MD_CTX_free(mdctx);
		return md5_hash;
	}
	
	DIR *dir = opendir(dir_path);
	file_list = malloc(sizeof(file_list_t));
	
	int count = 0;
	struct dirent *entry;
	
	// Count files in the directory first
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) { // Only regular files
			count++;
		}
	}
	
	file_list->num_files = count;
	file_list->file_names = malloc(count * sizeof(char *));
	file_list->md5s = malloc(count * sizeof(char *));
	
	// Reset directory stream and fill in file names
	rewinddir(dir);
	count = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) { // Only regular files
			file_list->file_names[count] = strdup(entry->d_name); // Names
			
			char* filepath = malloc(strlen(dir_path) + strlen(entry->d_name) + 2);
			strcpy(filepath, dir_path);
			filepath[strlen(dir_path)] = '/';
			filepath[strlen(dir_path) + 1] = 0;
			strcat(filepath, entry->d_name);
			file_list->md5s[count] = compute_md5(filepath); // md5
			
			count++;
		}
	}
	
	closedir(dir);
}

void free_file_list(file_list_t *file_list_ptr) {
	if (file_list_ptr) {
		for (int i = 0; i < file_list_ptr->num_files; i++) {
			free(file_list_ptr->file_names[i]);
			free(file_list_ptr->md5s[i]);
		}
		free(file_list_ptr->file_names);
		free(file_list_ptr->md5s);
		free(file_list_ptr);
	}
}
