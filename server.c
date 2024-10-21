/*Included libraries*/
#include <stdio.h>		/* For printf() and fprintf() */
#include <sys/socket.h> /* For socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	/* For sockaddr_in and inet_addr() */
#include <stdlib.h>		/* Supports all sorts of functionality */
#include <unistd.h>		/* For close() */
#include <string.h>		/* Support any string ops */

#include <pthread.h>	 /* For thread() */
#include <dirent.h>		 /* Support any file ops */
#include <openssl/evp.h> /* Support md5 */

/* Constants */
#define PORT 8888
#define BUFFER_SIZE 1024 /* The send buffer size */

/* Globals */
typedef struct
{
	int sock;
	char client_ip[INET_ADDRSTRLEN]; // For storing client IP
	int client_port;				 // For storing client port
} client_info_t;

typedef struct
{
	int num_files;		  // Number of files
	char **file_names;	  // Array of file names
	unsigned char **md5s; // Array of md5s
} file_list_t;

pthread_mutex_t lock;
file_list_t *file_list;
char PATH[100] = "./music_server";

/* Functions */
void *handle_client(void *client_info_ptr);
void list_files(int clientSock);
void get_diff(int clientSock);
void send_files(client_info_t *client_info);
void log_client_activity(client_info_t *client_info, const char *message);
void send_file_list(int client_sock);
void create_file_list(const char *dir_path);
void free_file_list(file_list_t *file_list_ptr);
unsigned char *compute_md5(char *file_name);

/* The main function */
int main()
{
	int serverSock;				   /* Server Socket */
	int clientSock;				   /* Client Socket */
	struct sockaddr_in serverAddr; /* Local address */
	struct sockaddr_in clientAddr; /* Client address */
	unsigned int addrLen;		   /* Length of address data struct */

	// Initialize mutex
	pthread_mutex_init(&lock, NULL);

	/* Create new TCP Socket for incoming requests*/
	if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("socket() failed\n");
		return 1;
	}

	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	/* Bind to local address structure */
	if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		printf("bind() failed\n");
		return 1;
	}

	/* Listen for incoming connections */
	if (listen(serverSock, 5) < 0)
	{
		printf("listen() failed\n");
		return 1;
	}

	addrLen = sizeof(struct sockaddr_in);
	create_file_list(PATH);
	printf("Server is listening on port %d...\n", PORT);

	// Handle multiple clients using threads
	while ((clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &addrLen)))
	{
		pthread_t client_thread;

		// Allocate memory for client info
		client_info_t *client_info = malloc(sizeof(client_info_t));
		client_info->sock = clientSock;

		// Get client IP and port
		inet_ntop(AF_INET, &(clientAddr.sin_addr), client_info->client_ip, INET_ADDRSTRLEN);
		client_info->client_port = ntohs(clientAddr.sin_port);

		// Log client connection
		log_client_activity(client_info, "Client connected");

		if (pthread_create(&client_thread, NULL, handle_client, (void *)client_info) < 0)
		{
			printf("Could not create thread\n");
			free(client_info);
			break;
		}
	}

	// Cleanup
	pthread_mutex_destroy(&lock);
	free_file_list(file_list);
	close(serverSock);
	return 0;
}

// Handle client communication
void *handle_client(void *client_info_ptr)
{
	client_info_t client_info;
	memcpy(&client_info, client_info_ptr, sizeof(client_info_t));
	free(client_info_ptr);
	char client_message[BUFFER_SIZE];

	while (recv(client_info.sock, client_message, sizeof(client_message), 0) > 0)
	{
		log_client_activity(&client_info, client_message);
		// Handle messages from the client
		if (strncmp(client_message, "LIST", 4) == 0)
		{
			list_files(client_info.sock);
		}
		else if (strncmp(client_message, "DIFF", 4) == 0)
		{
			get_diff(client_info.sock);
		}
		else if (strncmp(client_message, "PULL", 4) == 0)
		{
			send_files(&client_info);
		}
		else if (strncmp(client_message, "LEAVE", 5) == 0)
		{
			log_client_activity(&client_info, "Client disconnected");
			break; // End session
		}
	}

	close(client_info.sock);
	pthread_exit(NULL);
}

// List files available on the server
void list_files(int clientSock)
{
	send_file_list(clientSock);
}

// Send diff of files between server and client
void get_diff(int clientSock)
{
	send_file_list(clientSock);
}

// Send files requested by the client
void send_files(client_info_t *client_info)
{
	send_file_list(client_info->sock);

	int index;
	while (recv(client_info->sock, &index, sizeof(index), 0) > 0)
	{
		if (index < 0)
		{
			break;
		}

		char buffer[BUFFER_SIZE];
		FILE *file;
		size_t bytes_read;

		// Open the requested file in read mode
		snprintf(buffer, sizeof(buffer), "%s/%s", PATH, file_list->file_names[index]);
		file = fopen(buffer, "rb");

		// Get file size
		fseek(file, 0L, SEEK_END);
		long file_size = ftell(file);
		send(client_info->sock, &file_size, sizeof(file_size), 0);
		rewind(file);

		// Send the file in chunks
		while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
		{
			if (send(client_info->sock, buffer, bytes_read, 0) < 0)
			{
				printf("Failed to send file\n");
				break;
			}
		}

		fclose(file);

		snprintf(buffer, sizeof(buffer), "Sent file: %s", file_list->file_names[index]);
		log_client_activity(client_info, buffer);
	}
}

// Log client activity
void log_client_activity(client_info_t *client_info, const char *message)
{
	printf("Client (%s:%d): %s\n", client_info->client_ip, client_info->client_port, message);

	pthread_mutex_lock(&lock);

	FILE *log_file = fopen("log.txt", "a");
	if (log_file != NULL)
	{
		fprintf(log_file, "Client (%s:%d): %s\n", client_info->client_ip, client_info->client_port, message);
		fclose(log_file);
	}
	else
	{
		printf("Could not open log file\n");
	}

	pthread_mutex_unlock(&lock);
}

// Send file list to the client
void send_file_list(int client_sock)
{
	// First, send the number of files
	send(client_sock, &file_list->num_files, sizeof(file_list->num_files), 0);

	// Then, send each file name and md5
	for (int i = 0; i < file_list->num_files; i++)
	{
		int len = strlen(file_list->file_names[i]) + 1;
		send(client_sock, &len, sizeof(len), 0);			 // Send the length of the file name
		send(client_sock, file_list->file_names[i], len, 0); // Send the file name
		send(client_sock, file_list->md5s[i], 16, 0);		 // Send the file md5
	}
}

// Create the file list
void create_file_list(const char *dir_path)
{
	DIR *dir = opendir(dir_path);
	file_list = malloc(sizeof(file_list_t));

	int count = 0;
	struct dirent *entry;

	// Count files in the directory first
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG)
		{ // Only regular files
			count++;
		}
	}

	file_list->num_files = count;
	file_list->file_names = malloc(count * sizeof(char *));
	file_list->md5s = malloc(count * sizeof(unsigned char *));

	// Reset directory stream and fill in file names
	rewinddir(dir);
	count = 0;
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG)
		{														  // Only regular files
			file_list->file_names[count] = strdup(entry->d_name); // Names

			char *filepath = malloc(strlen(dir_path) + strlen(entry->d_name) + 2);
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

void free_file_list(file_list_t *file_list_ptr)
{
	if (file_list_ptr)
	{
		for (int i = 0; i < file_list_ptr->num_files; i++)
		{
			free(file_list_ptr->file_names[i]);
			free(file_list_ptr->md5s[i]);
		}
		free(file_list_ptr->file_names);
		free(file_list_ptr->md5s);
		free(file_list_ptr);
	}
}

// Helper function to calculate MD5 hash of a file
unsigned char *compute_md5(char *file_name)
{
	FILE *file = fopen(file_name, "rb");

	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

	unsigned char buffer[1024];
	int bytes;
	while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0)
	{
		EVP_DigestUpdate(mdctx, buffer, bytes);
	}

	unsigned char *md5_hash = malloc(EVP_MD_size(EVP_md5()));
	EVP_DigestFinal_ex(mdctx, md5_hash, NULL);

	fclose(file);
	EVP_MD_CTX_free(mdctx);
	return md5_hash;
}
