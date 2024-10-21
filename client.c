/* Included libraries */
#include <stdio.h>			/* For printf() and fprintf() */
#include <sys/socket.h>		/* For socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		/* For sockaddr_in and inet_addr() */
#include <stdlib.h>			/* Supports all sorts of functionality */
#include <unistd.h>			/* For close() */
#include <string.h>			/* Support any string ops */

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

/* Functions */
void showMenu();
void list_files(int clientSock);
void diff_files(int clientSock);
void pull_files(int clientSock);
void create_file_list(const char *dir_path);
void free_file_list();

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

	while (1) {
		showMenu();
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
	printf("Enter your choice: ");
}

// 1. List Files: Ask the server to return a list of files it currently has
void list_files(int clientSock) {
	char buffer[BUFFER_SIZE];
	send(clientSock, "LIST", 4, 0);
	recv(clientSock, buffer, BUFFER_SIZE, 0); 
	printf("\nA List of Files that Server has:\n%s\n", buffer);
}

// 2. Diff (based on 1)): the client should show a "diff" of the files it has in comparison to the server);
void diff_files(int clientSock) {
	char buffer[BUFFER_SIZE];
	send(clientSock, "DIFF", 4, 0);
	memset(buffer, 0, BUFFER_SIZE); 
	recv(clientSock, buffer, BUFFER_SIZE, 0); 
	printf("\nFiles not on client:\n%s\n", buffer);
}

// 3. Pull (request all files identified in 2): from the server and store them locally)
void pull_files(int clientSock) {

}

// Create the file list
void create_file_list(const char *dir_path) {
	
	// Helper function to calculate MD5 hash of a file
	unsigned char *compute_md5(char *filename) {
		FILE *file = fopen(filename, "rb");

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

void free_file_list() {
	if (file_list) {
		for (int i = 0; i < file_list->num_files; i++) {
			free(file_list->file_names[i]);
			free(file_list->md5s[i]);
		}
		free(file_list->file_names);
		free(file_list->md5s);
		free(file_list);
	}
}
