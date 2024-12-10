#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 12345
#define ACK "ACK"

// Linked List Node
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Linked List Structure
typedef struct {
    Node* head;
} list_t;

// Function prototypes
list_t* list_alloc();
void list_free(list_t*);
void list_add_to_front(list_t*, int);
int list_remove_at_index(list_t*, int);
int list_length(list_t*);
char* listToString(list_t*);

int main() {
    int servSockD, clientSocket;
    struct sockaddr_in servAddr;
    char buf[1024];
    char sbuf[1024];
    char* token;

    // Create server socket
    servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0) {
        perror("socket");
        exit(1);
    }

    // Define server address
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to the specified IP and port
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for connections
    listen(servSockD, 1);
    printf("Server is running and waiting for connections on port %d...\n", PORT);

    // Accept a client connection
    clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        perror("accept");
        exit(1);
    }
    printf("Client connected successfully.\n");

    // Create an empty linked list
    list_t *mylist = list_alloc();

    while (1) {
        // Receive messages from client socket
        int n = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            printf("Client disconnected or error occurred.\n");
            break;
        }

        buf[n] = '\0'; // Null-terminate the received string
        printf("Received command: %s\n", buf);

        token = strtok(buf, " ");
        if (!token) {
            snprintf(sbuf, sizeof(sbuf), "Error: Empty command received.");
        } else if (strcmp(token, "exit") == 0) {
            snprintf(sbuf, sizeof(sbuf), "Server exiting...");
            break; // Exit command
        } else if (strcmp(token, "get_length") == 0) {
            int length = list_length(mylist);
            snprintf(sbuf, sizeof(sbuf), "Length: %d", length);
        } else if (strcmp(token, "add_front") == 0) {
            token = strtok(NULL, " "); // Get next token (value)
            if (!token) {
                snprintf(sbuf, sizeof(sbuf), "Error: Missing value for add_front");
            } else {
                int val = atoi(token);
                list_add_to_front(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            }
        } else if (strcmp(token, "remove_position") == 0) {
            token = strtok(NULL, " ");
            if (!token) {
                snprintf(sbuf, sizeof(sbuf), "Error: Missing index for remove_position");
            } else {
                int idx = atoi(token);
                int removed_value = list_remove_at_index(mylist, idx);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, removed_value);
            }
        } else if (strcmp(token, "print") == 0) {
            char *list_str = listToString(mylist);
            snprintf(sbuf, sizeof(sbuf), "%s", list_str);
            free(list_str); // Free memory allocated by `listToString`
        } else {
            snprintf(sbuf, sizeof(sbuf), "Unknown command: %s", token);
        }

        // Send response back to client socket
        send(clientSocket, sbuf, strlen(sbuf), 0);
        printf("Sent response: %s\n", sbuf);
    }

    // Clean up
    printf("Shutting down server.\n");
    close(clientSocket);
    close(servSockD);
    list_free(mylist); // Free memory allocated for the linked list

    return 0;
}

// Linked List Functions Implementation

list_t* list_alloc() {
    list_t* list = malloc(sizeof(list_t));
    if (!list) {
        perror("list_alloc");
        exit(1);
    }
    list->head = NULL; // Initialize head to NULL
    return list;
}

void list_free(list_t *list) {
    Node *current = list->head;
    Node *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

void list_add_to_front(list_t *list, int value) {
    Node *new_node = malloc(sizeof(Node));
    if (!new_node) {
        perror("list_add_to_front");
        exit(1);
    }
    new_node->data = value;
    new_node->next = list->head;
    list->head = new_node;
}

int list_remove_at_index(list_t *list, int index) {
    if (index < 0 || !list->head) return -1; // Invalid index

    Node *current = list->head;
    Node *previous = NULL;

    for (int i = 0; i < index; i++) {
        if (!current) return -1; // Index out of range
        previous = current;
        current = current->next;
    }

    int removed_value = current->data;

    if (!previous) { // Removing head
        list->head = current->next;
    } else {
        previous->next = current->next;
    }

    free(current);
    return removed_value;
}

int list_length(list_t *list) {
    int length = 0;
    Node *current = list->head;

    while (current != NULL) {
        length++;
        current = current->next;
    }

    return length;
}

char* listToString(list_t *list) {
    char *result = malloc(1024);
    if (!result) {
        perror("listToString");
        exit(1);
    }
    strcpy(result, "List: ");

    Node *current = list->head;

    while (current != NULL) {
        char buffer[50];
        sprintf(buffer, "%d -> ", current->data);
        strcat(result, buffer);
        current = current->next;
    }

    strcat(result, "NULL");

    return result;
}
