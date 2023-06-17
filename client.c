#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erreur lors de la création du socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Adresse invalide / Adresse non supportée\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connexion échouée\n");
        return -1;
    }

    // Lire les entrées de l'utilisateur et envoyer les données au serveur
    char category[BUFFER_SIZE];
    printf("Catégorie de la place (gradin 1, gradin 2, fosse) : ");
    fgets(category, sizeof(category), stdin);
    category[strcspn(category, "\n")] = '\0';
    send(sock, category, strlen(category), 0);

    char numPlaces[BUFFER_SIZE];
    printf("Nombre de places : ");
    fgets(numPlaces, sizeof(numPlaces), stdin);
    numPlaces[strcspn(numPlaces, "\n")] = '\0';
    send(sock, numPlaces, strlen(numPlaces), 0);

    char name[BUFFER_SIZE];
    printf("Nom et prénom : ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    send(sock, name, strlen(name), 0);

    // Attendre la réponse du serveur et l'afficher
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        printf("Réponse du serveur : %s\n", buffer);
    } else {
        printf("Erreur lors de la réception de la réponse du serveur\n");
    }

    close(sock);
    return 0;
}
