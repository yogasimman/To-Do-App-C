#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <json-c/json.h>
#include "file_path.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int sock, client_socket;
struct sockaddr_in address;
TreeNode* root; // Root of the AVL tree
char method[8];
char uri[128];

void handle_post(char *body) {
    // Locate the start of the JSON data
    char *json_start = strstr(body, "\r\n\r\n"); // Locate the end of the headers
    if (!json_start) {
        printf("Error: Invalid request format. No JSON found.\n");
        return;
    }
    json_start += 4; // Move past the "\r\n\r\n" to the start of the JSON

    // Ensure the JSON data is null-terminated
    if (json_start[strlen(json_start) - 1] != '\0') {
        json_start[strlen(json_start)] = '\0';
    }

    // Parse the JSON data
    struct json_object *parsed_json = json_tokener_parse(json_start);
    
    if (parsed_json == NULL) {
        printf("Error: Failed to parse JSON.\n");
        // Send a bad request response
        char *response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid JSON data";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    struct json_object *title;
    struct json_object *content;

    // Check if required fields are present
    if (json_object_object_get_ex(parsed_json, "title", &title) &&
        json_object_object_get_ex(parsed_json, "content", &content)) {
        
        FILE *fp = fopen("client/tasks.json", "r+");
        if (!fp) {
            fp = fopen("client/tasks.json", "w");  // Create file if it doesn't exist
        }
        
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *json_data = malloc(length + 1);
        if (json_data == NULL) {
            printf("Error: Memory allocation failed.\n");
            fclose(fp);
            json_object_put(parsed_json);
            return;
        }

        fread(json_data, 1, length, fp);
        json_data[length] = '\0';  // Null-terminate the string

        struct json_object *task_array = json_tokener_parse(json_data);
        if (!task_array || !json_object_is_type(task_array, json_type_array)) {
            task_array = json_object_new_array();  // Create a new array if file is empty or invalid
        }

        // Add the new task to the array
        json_object_array_add(task_array, parsed_json);

        // Write back the updated array to the file
        fseek(fp, 0, SEEK_SET);
        fwrite(json_object_to_json_string(task_array), 1, strlen(json_object_to_json_string(task_array)), fp);
        ftruncate(fileno(fp), ftell(fp));  // Truncate the file to the new length

        fclose(fp);
        free(json_data);
        json_object_put(task_array);

        // Send a success response
        char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nTask added successfully";
        send(client_socket, response, strlen(response), 0);
    } else {
        // Send a bad request response if JSON is invalid
        char *response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid JSON data";
        send(client_socket, response, strlen(response), 0);
        json_object_put(parsed_json);
    }
}



void handle_delete(char *body) {
    // Find the start of the JSON body
    char *json_start = strstr(body, "\r\n\r\n"); // Locate the end of the headers
    if (!json_start) {
        printf("Error: Invalid request format.\n");
        return;
    }
    json_start += 4; // Skip past the "\r\n\r\n" to the start of the JSON

    // Parse the JSON data
    struct json_object *parsed_json = json_tokener_parse(json_start);
    struct json_object *title;

    if (json_object_object_get_ex(parsed_json, "title", &title)) {
        FILE *fp = fopen("client/tasks.json", "r+");
        if (!fp) {
            printf("Error opening file.\n");
            return;
        }

        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *json_data = malloc(length + 1);
        fread(json_data, 1, length, fp);
        json_data[length] = '\0';  // Null-terminate the string

        struct json_object *task_array = json_tokener_parse(json_data);
        if (!task_array || !json_object_is_type(task_array, json_type_array)) {
            printf("Error parsing JSON data.\n");
            fclose(fp);
            free(json_data);
            return;
        }

        // Create a new JSON array without the deleted task
        int array_length = json_object_array_length(task_array);
        struct json_object *new_array = json_object_new_array();
        int task_found = 0;

        for (int i = 0; i < array_length; i++) {
            struct json_object *task = json_object_array_get_idx(task_array, i);
            struct json_object *task_title;

            if (json_object_object_get_ex(task, "title", &task_title)) {
                if (strcmp(json_object_get_string(task_title), json_object_get_string(title)) != 0) {
                    json_object_array_add(new_array, json_object_get(task));  // Copy the task if not matched
                } else {
                    task_found = 1;  // Mark the task as found
                }
            }
        }

        if (task_found) {
            // Task was found and deleted
            fseek(fp, 0, SEEK_SET);
            fwrite(json_object_to_json_string(new_array), 1, strlen(json_object_to_json_string(new_array)), fp);
            ftruncate(fileno(fp), ftell(fp));  // Truncate the file to the new length
            char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nTask deleted successfully";
            send(client_socket, response, strlen(response), 0);
        } else {
            // Task not found, send 404 response
            char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nTask not found";
            send(client_socket, response, strlen(response), 0);
        }

        fclose(fp);
        free(json_data);
        json_object_put(task_array);
        json_object_put(new_array);
    } else {
        // Send a bad request response if title is not found
        char *response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid JSON data";
        send(client_socket, response, strlen(response), 0);
    }

    json_object_put(parsed_json);
}

void handle_get(char* buffer) {
    
    sscanf(buffer, "GET %s", uri);
    // Strip query parameters if present
    char *query_start = strchr(uri, '?');
    if (query_start) {
        *query_start = '\0'; // Null-terminate the string at the start of query parameters
    }
    printf("Requested URI: %s\n", uri);

    TreeNode* result = search(root, uri);
    if (result != NULL) {
        printf("File found. Serving...\n");
        if (result->val->filepath == NULL) {
            char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
            send(client_socket, not_found, strlen(not_found), 0);
            return;
        }

        char response[BUFFER_SIZE];
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", result->val->ContentType);
        send(client_socket, response, strlen(response), 0);

        fseek(result->val->filepath, 0, SEEK_SET);

        char file_buffer[BUFFER_SIZE];
        size_t n;
        while ((n = fread(file_buffer, 1, BUFFER_SIZE, result->val->filepath)) > 0) {
            send(client_socket, file_buffer, n, 0);
        }

        fseek(result->val->filepath, 0, SEEK_SET);
    } else {
        char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
        send(client_socket, not_found, strlen(not_found), 0);
    }
}


void shutdownServer() {
    // Deallocate the AVL tree and other heap memory
    deallocateTree(root); // root is the root of the AVL tree

    // Close the server socket
    close(sock);

    printf("Server shutdown and memory deallocated successfully.\n");
}

// Function to handle SIGTERM signal
void signalHandler(int signum) {
    shutdownServer();
    exit(signum);
}

int main() {
    root = NULL;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    socklen_t addrlen;
    char buffer[BUFFER_SIZE];

    signal(SIGTERM, signalHandler);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Binding to the Port Number Failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 10) < 0) {
        perror("Failed to Listen");
        exit(EXIT_FAILURE);
    }

    // Initialize AVL tree with paths
    readFile(&root);

    while (1) {
        client_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        memset(buffer, 0, sizeof(buffer));

        int len = recv(client_socket, buffer, sizeof(buffer), 0);
        if (len == 0) {
            perror("Error from client side");
            continue;
        }

        sscanf(buffer, "%s", method);

        if (strstr(method, "GET") != NULL) {
            handle_get(buffer);
        } else if (strstr(method, "POST") != NULL) {
            handle_post(buffer);
        } else if (strstr(method, "SHUTDOWN") != NULL) { // Custom shutdown command
            shutdownServer();
            break; // Exit the loop to end the server
        } else if (strstr(method, "DELETE") != NULL) {
            handle_delete(buffer);
        } else {
            // Send 403 Forbidden
            char* forbidden = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod not allowed";
            send(client_socket, forbidden, strlen(forbidden), 0);
        }
        close(client_socket);
    }

    return 0;
}
