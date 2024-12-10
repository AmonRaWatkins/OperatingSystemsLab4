#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_COMMAND_LINE_LEN 1024

char* getCommandLine(char *command_line) {
    do { 
        // Read input from stdin and store it in command_line
        if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
            fprintf(stderr, "fgets error");
            exit(0);
        }
    } while (command_line[0] == 0x0A);  // While just ENTER pressed
    command_line[strlen(command_line) - 1] = '\0'; // Remove newline character
    return command_line;
}

int main(int argc, char const* argv[]) { 
    int sockID = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockID < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in servAddr; 
    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(PORT); 
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    int connectStatus = connect(sockID, (struct sockaddr*)&servAddr, sizeof(servAddr)); 
    if (connectStatus == -1) { 
        perror("connect");
        close(sockID);
        exit(1);
    } 

    printf("Connected to the server.\n");

    char buf[MAX_COMMAND_LINE_LEN];
    char responseData[MAX_COMMAND_LINE_LEN];

    while (1) {
        printf("Enter Command (or menu): ");
        getCommandLine(buf);

        send(sockID, buf, strlen(buf), 0);  // Send command and args to server

        char* token = strtok(buf, " ");
        if (strcmp(token, "exit") == 0) {
            break; // Exit command
        } else if (strcmp(token, "menu") == 0) {
            printf("COMMANDS:\n---------\n1. print\n2. get_length\n3. add_back <value>\n4. add_front <value>\n5. add_position <index> <value>\n6. remove_back\n7. remove_front\n8. remove_position <index>\n9. get <index>\n10. exit\n");
            continue; // Skip receiving response after showing menu
        }

        int n = recv(sockID, responseData, sizeof(responseData) - 1, 0); // Receive response from server
        if (n > 0) {
            responseData[n] = '\0'; // Null-terminate response
            printf("\nSERVER RESPONSE: %s\n", responseData);
        } else {
            printf("\nError: No response from server\n");
        }

        memset(buf, '\0', MAX_COMMAND_LINE_LEN); // Clear buffer for next command
    }

    printf("Closing client.\n");
    close(sockID); // Close socket before exiting
    return 0; 
}
