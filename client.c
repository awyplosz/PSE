#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 2000
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    // Read user inputs and send data to the server
    char category[BUFFER_SIZE];
    printf("Category of the seat (grandstand 1, grandstand 2, standing): ");
    fgets(category, sizeof(category), stdin);
    category[strcspn(category, "\n")] = '\0';
    send(sock, category, strlen(category) + 1, 0);

    char numPlaces[BUFFER_SIZE];
    printf("Number of seats: ");
    fgets(numPlaces, sizeof(numPlaces), stdin);
    numPlaces[strcspn(numPlaces, "\n")] = '\0';
    send(sock, numPlaces, strlen(numPlaces) + 1, 0);

    char name[BUFFER_SIZE];
    printf("Name and surname: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    send(sock, name, strlen(name) + 1, 0);

    // Wait for the server's response and display it
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        printf("Server's response: %s\n", buffer);
    } else {
        printf("Error receiving server's response\n");
    }

    // Close the socket
    close(sock);
    return 0;
}
