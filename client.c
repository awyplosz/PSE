#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define PORT 2000
#define BUFFER_SIZE 1024

void server_response(int sock);

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
    printf("Welcome to the concert ticket sales server !\n");
    printf("You are in the waiting list\n");
    printf("There are 3 categories of seat you can buy :\n");
    printf("grandstand 1 : 100 euros\ngrandstand 2 : 50 euros \nstanding : 80 euros\n"); 
    printf("You can buy several tickets in the same category\n"); 
    printf("Which category do you want to buy : ");
    fgets(category, sizeof(category), stdin);
    category[strcspn(category, "\n")] = '\0';
    send(sock, category, strlen(category) + 1, 0);

    server_response(sock);

    char numPlaces[BUFFER_SIZE];
    printf("Choose the number of seats : ");
    fgets(numPlaces, sizeof(numPlaces), stdin);
    numPlaces[strcspn(numPlaces, "\n")] = '\0';
    send(sock, numPlaces, strlen(numPlaces) + 1, 0);

    server_response(sock);

    char name[BUFFER_SIZE];
    printf("Name and surname: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    send(sock, name, strlen(name) + 1, 0);
    

    server_response(sock);
    
    char treasury[BUFFER_SIZE];
    fgets(treasury, sizeof(treasury), stdin);
    treasury[strcspn(treasury, "\n")] = '\0';
    send(sock, treasury, strlen(treasury) + 1, 0);

    server_response(sock);


    // Close the socket
    close(sock);
    return 0;
}

void server_response(int sock){

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        printf("Server's response: %s\n", buffer);
    } else if (bytes_received == 0) {
        printf("Connection closed by the server\n");
        return;
    } else {
        printf("Error receiving server's response\n");
        return;
    }

}
