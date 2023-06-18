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
pthread_mutex_t treasury_mutex;
int treasury = 0;
int num_connections = 0;

int grandstand1_tickets = 10;
int grandstand1_price = 100;

int grandstand2_tickets = 10;
int grandstand2_price = 50;

int standing_tickets = 10;
int standing_price = 80;

int cart = 0;


// Function to handle client connections
void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    int available_tickets = 0;
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

        if (strcmp(category, "grandstand 1") == 0) {
            available_tickets = grandstand1_tickets;
        } else if (strcmp(category, "grandstand 2") == 0) {
            available_tickets = grandstand2_tickets;
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
        }   else {
        // Error receiving category.
        char response[] = "Error receiving category. Please try again.";
        send(sock, response, strlen(response), 0);
        }

        // Respond with the ticket availability.
        char answer[BUFFER_SIZE];
        snprintf(answer, sizeof(answer), "Available tickets for %s : %d", category, available_tickets);
        send(sock, answer, strlen(answer), 0);

        // Read the number of seats requested and calculate the cart
        char num_seat[BUFFER_SIZE];
        memset(num_seat, 0, sizeof(num_seat));
        int num_seats_bytes = recv(sock, num_seat, sizeof(buffer) - 1, 0);
        int requested_seats = atoi(num_seat);
        if (num_seats_bytes > 0) {
            
            printf("Number of seats requested: '%s'\n", num_seat);
        } 
        else {
            printf("Failed to receive number of seats from the client.\n");
        }

            if (available_tickets >= requested_seats) {
                // Update the ticket count.
                if (strcmp(category, "grandstand 1") == 0) {
                    cart = grandstand1_price * requested_seats;
                } else if (strcmp(category, "grandstand 2") == 0) {
                    cart = grandstand2_price * requested_seats;
                } else if (strcmp(category, "standing") == 0) {
                    cart = standing_price * requested_seats;
                }

                char response[] = "Great there are enough tickets ! Who is the honorable buyer ?";
                send(sock, response, strlen(response), 0);

            } 
            else {
                // Not enough tickets available.
                char response[] = "Apologies, but there are not enough tickets available.\n";
                send(sock, response, strlen(response), 0);
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



        // Ask the client if they want to buy the ticket
        char resp[BUFFER_SIZE] = {0};
        strcat(resp, "Your total cart is ");
        char cart_str[10];
        sprintf(cart_str, "%d", cart);
        strcat(resp, cart_str);
        strcat(resp, " euros. Do you want to buy the ticket(s)? (yes/no): ");
        send(sock, resp, strlen(resp), 0);

        // Receive the client's response
        char treasure[BUFFER_SIZE];
        memset(treasure, 0, sizeof(treasure));
        int bytes_recvd = recv(sock, treasure, sizeof(treasure) - 1, 0);
        if (bytes_recvd > 0) {
        // Check the ticket availability based on the category.

        if (strcmp(treasure, "yes") == 0){
            // Acquire the mutex to access the treasury
                pthread_mutex_lock(&treasury_mutex);

                // Add the cart amount to the treasury
                treasury += cart;

                if (strcmp(category, "grandstand 1") == 0) {
                    grandstand1_tickets -= requested_seats;
                } else if (strcmp(category, "grandstand 2") == 0) {
                    grandstand2_tickets -= requested_seats;
                } else if (strcmp(category, "standing") == 0) {
                    standing_tickets -= requested_seats;
                }

                // Release the mutex
                pthread_mutex_unlock(&treasury_mutex);

                // Prepare a sassy response message. Gotta show off that personality!
                char success_response[] = "Congratulations! You have successfully purchased the ticket. Enjoy the show!";
                send(sock, success_response, strlen(success_response), 0);
            } else {
                // Prepare a response message for the client who didn't buy the ticket
                char failure_response[] = "Ticket purchase declined. We hope to see you at another event!";
                send(sock, failure_response, strlen(failure_response), 0);
            }
            } else {
                printf("Error receiving client's response\n");
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
    pthread_mutex_init(&treasury_mutex, NULL);

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
    pthread_mutex_destroy(&treasury_mutex);
    return 0;
}
