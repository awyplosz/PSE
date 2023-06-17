#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define PORT 2000
#define BUFFER_SIZE 1024

sem_t waitlist_sem;  // Semaphore for the wait list
int num_connections = 0;

// Function to handle client connections
void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    sem_wait(&waitlist_sem);
    num_connections++;
    printf("Client %d has connected and grabbed a spot on the waitlist.\n", num_connections);

    // Read the category of seat, number of seats, and client's name
    memset(buffer, 0, sizeof(buffer));
    int category_bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (category_bytes > 0) {
        printf("Category of seat requested: '%s'\n", buffer);
    } else {
        printf("Failed to receive category of seat from the client.\n");
    }

    // Read the number of seats requested
    memset(buffer, 0, sizeof(buffer));
    int num_seats_bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (num_seats_bytes > 0) {
        printf("Number of seats requested: '%s'\n", buffer);
    } else {
        printf("Failed to receive number of seats from the client.\n");
    }

    // Read the client's name
    memset(buffer, 0, sizeof(buffer));
    int name_bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (name_bytes > 0) {
        printf("Client's name: '%s'\n", buffer);
    } else {
        printf("Failed to receive client's name.\n");
    }

    // Prepare a response message
    char response[] = "Thank you for your reservation!";
    send(sock, response, strlen(response), 0);

    num_connections--;
    sem_post(&waitlist_sem);

    // Close the connection
    close(sock);
    free(socket_desc);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create a socket file descriptor. We're all about connections!
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Oh no! Socket creation failed. ");
        return -1;
    }

    printf("server file created\n");


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to localhost port 2000. Let's get this party started!
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed.");
        return -1;
    }

    // Listen for incoming connections. The more, the merrier!
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        return -1;
    }

    printf("server listening\n");
    sem_init(&waitlist_sem, 0, 2);

    // Accept incoming connections and handle them using threads. Time to multitask!
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen))) {
        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        if (pthread_create(&thread_id, NULL, connection_handler, (void *)new_sock) < 0) {
            perror("Thread creation failed. My grand plans ruined!");
            return -1;
        }
    }
    sem_destroy(&waitlist_sem);
    return 0;
}
