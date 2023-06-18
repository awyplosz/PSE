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
int granstand1_tickets = 10;
int granstand2_tickets = 10;
int standing_tickets = 10;


// Function to handle client connections
void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    sem_wait(&waitlist_sem);
    num_connections++;
    printf("Client %d has connected and grabbed a spot on the waitlist.\n", num_connections);


    // Read the category of seat the client desires.
    char category[BUFFER_SIZE];
    memset(category, 0, sizeof(category));
    int bytes_received = recv(sock, category, sizeof(category) - 1, 0);
    if (bytes_received > 0) {
        // Check the ticket availability based on the category.
        int available_tickets = 0;
        if (strcmp(category, "grandstand 1") == 0) {
            available_tickets = granstand1_tickets;
        } else if (strcmp(category, "grandstand 2") == 0) {
            available_tickets = granstand2_tickets;
        } else if (strcmp(category, "standing") == 0) {
            available_tickets = standing_tickets;
        } else {
             // Invalid category entered by the client.
            char response[] = "Invalid category. Please choose a valid category.";
            send(sock, response, strlen(response), 0);
            sem_post(&waitlist_sem);
            close(sock);
            free(socket_desc);
            pthread_exit(NULL);
        }

    // Respond with the ticket availability.
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "Available tickets for %s : %d", category, available_tickets);
    send(sock, response, strlen(response), 0);

    // Read the number of seats requested
    char num_seat[BUFFER_SIZE];
    memset(num_seat, 0, sizeof(num_seat));
    int num_seats_bytes = recv(sock, num_seat, sizeof(buffer) - 1, 0);
    if (num_seats_bytes > 0) {
        printf("Number of seats requested: '%s'\n", num_seat);
    } 
    else {
        printf("Failed to receive number of seats from the client.\n");
    }

    // Read the client's name
    char client_name[BUFFER_SIZE];
    memset(client_name, 0, sizeof(buffer));
    int name_bytes = recv(sock, client_name, sizeof(buffer) - 1, 0);
    if (name_bytes > 0) {
        printf("Client's name: '%s'\n", client_name);
    } else {
        printf("Failed to receive client's name.\n");
    }

    int requested_seats = atoi(num_seat);
        if (available_tickets >= requested_seats) {
            // Update the ticket count.
            if (strcmp(category, "grandstand 1") == 0) {
                granstand1_tickets -= requested_seats;
            } else if (strcmp(category, "grandstand 2") == 0) {
                granstand2_tickets -= requested_seats;
            } else if (strcmp(category, "standing") == 0) {
                standing_tickets -= requested_seats;
            }

            // Prepare a response message.
            char response[] = "Ticket(s) reserved! Enjoy the show!\n";
            send(sock, response, strlen(response), 0);
        } 
        else {
            // Not enough tickets available.
            char response[] = "Apologies, but there are not enough tickets available.\n";
            send(sock, response, strlen(response), 0);
        }
   
    } 
    else {
        // Error receiving category.
        char response[] = "Error receiving category. Please try again.";
        send(sock, response, strlen(response), 0);
    }

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
