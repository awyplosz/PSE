#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct Client {
    int socket;
    int rank;
    struct Client *next;
};

struct Queue {
    struct Client *front, *rear;
    int size;
};

struct Queue *createQueue() {
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->front = queue->rear = NULL;
    queue->size = 0;
    return queue;
}

int isQueueEmpty(struct Queue *queue) {
    return (queue->front == NULL);
}

void enqueue(struct Queue *queue, struct Client *client) {
    if (queue->rear == NULL) {
        queue->front = queue->rear = client;
    } else {
        queue->rear->next = client;
        queue->rear = client;
    }
    client->next = NULL;
    queue->size++;
}

struct Client *dequeue(struct Queue *queue) {
    if (isQueueEmpty(queue)) {
        printf("La file d'attente est vide.\n");
        return NULL;
    }
    struct Client *client = queue->front;
    queue->front = client->next;
    if (queue->front == NULL)
        queue->rear = NULL;
    queue->size--;
    return client;
}

void *handleClient(void *arg) {
    struct Client *client = (struct Client *)arg;
    char buffer[BUFFER_SIZE] = {0};

    // Envoyer la place dans la liste d'attente
    char response[50];
    snprintf(response, sizeof(response), "Vous êtes en position %d dans la liste d'attente.", client->rank);
    send(client->socket, response, strlen(response), 0);

    // Recevoir les informations du client
    recv(client->socket, buffer, BUFFER_SIZE, 0);

    // Traiter les informations du client

    // Fermer la connexion du client en liste d'attente
    close(client->socket);
    printf("Client %d (en liste d'attente) déconnecté\n", client->rank);

    // Libérer la mémoire pour la structure Client
    free(client);

    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    pthread_t threads[MAX_CLIENTS];
    int client_rank = 1;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur lors de la création du socket");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Erreur lors du bind");
        return -1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur lors de la mise en écoute");
        return -1;
    }

    printf("Le serveur est en attente de connexions...\n");

    struct Queue *waiting_queue = createQueue();

    while (1) {
        socklen_t client_address_size = sizeof(client_address);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size)) < 0) {
            perror("Erreur lors de l'acceptation de la connexion client");
            return -1;
        }

        if (client_rank <= MAX_CLIENTS) {
            struct Client *client = (struct Client *)malloc(sizeof(struct Client));
            client->socket = client_socket;
            client->rank = client_rank;

            pthread_create(&threads[client_rank - 1], NULL, handleClient, (void *)client);

            printf("Client %d connecté\n", client_rank);

            client_rank++;
        } else {
            struct Client *client = (struct Client *)malloc(sizeof(struct Client));
            client->socket = client_socket;
            client->rank = client_rank;
            enqueue(waiting_queue, client);

            char response[50];
            snprintf(response, sizeof(response), "Vous êtes en position %d dans la liste d'attente.", client_rank);
            send(client_socket, response, strlen(response), 0);
            printf("Client mis en liste d'attente\n");
        }

        while (!isQueueEmpty(waiting_queue)) {
            struct Client *waiting_client = dequeue(waiting_queue);

            if (waiting_client != NULL) {
                pthread_create(&threads[client_rank - 1], NULL, handleClient, (void *)waiting_client);

                printf("Client %d (en liste d'attente) connecté\n", client_rank);

                client_rank++;
            }
        }
    }

    close(server_socket);

    return 0;
}
