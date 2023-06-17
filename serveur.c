#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 2000
#define BUFFER_SIZE 1024

// Function to handle client connections. Who needs just one thread when you can have many?
void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};

    // Read the category of seat the client desires. 
    memset(buffer, 0, sizeof(buffer));
    int category_bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (category_bytes > 0) {
        printf("Oh, the client wants a seat in the '%s' category. Fancy!\n", buffer);
    } else {
        printf("Something went wrong while receiving the category of seat from the client.\n");
    }

    // Read the number of seats the client is demanding.
    memset(buffer, 0, sizeof(buffer));
    int num_seats_bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (num_seats_bytes > 0) {
        printf("The client's feeling greedy and wants '%s' seats. Better find 'em!\n", buffer);
    } else {
        printf("Oops! Failed to get the number of seats requested by the client. They won't be happy about that.\n");
    }

    // Read the client's name and surname. 
    memset(buffer, 0, sizeof(buffer));
    int name_bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (name_bytes > 0) {
        printf("Ah, the client goes by '%s'. Sounds fancy and important!\n", buffer);
    } else {
        printf("Oh no, we missed the client's name! What a blunder...\n");
    }

    // Prepare a sassy response message. Gotta show off that personality!
    char response[] = "Thank you for gracing us with your presence and reserving a seat! Enjoy the show!";
    send(sock, response, strlen(response), 0);

    // Close the connection. Time to bid farewell.
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

    // Set those socket options for address and port reuse. It's all about efficiency!
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt? Who needs that? Well, apparently I do.");
        return -1;
    }

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

    return 0;
}
