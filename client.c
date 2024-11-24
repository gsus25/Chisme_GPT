#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_PREPAID_MESSAGES 10
#define POSTPAID 1
#define PREPAID 0

void *send_messages(void *arg) {
    int sockfd = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int user_id = rand() % 1000; // ID aleatorio para el cliente
    int type = -1; // Variable para almacenar el tipo de cliente
    int message_count = 0;

    // Recibir el tipo de usuario asignado por el servidor
    memset(buffer, 0, BUFFER_SIZE);
    recv(sockfd, buffer, BUFFER_SIZE, 0);
    
    // El servidor envía el tipo de usuario
    if (strstr(buffer, "Pos-pago") != NULL) {
        type = POSTPAID;
    } else {
        type = PREPAID;
    }

    printf("[CLIENTE %d] Tipo de usuario: %s\n", user_id, type == POSTPAID ? "Pos-pago" : "Pre-pago");

    for (int i = 0; i < 15; i++) { // Intentar enviar 15 mensajes
        if (type == PREPAID && message_count >= MAX_PREPAID_MESSAGES) {
            printf("[CLIENTE %d] Límite alcanzado para pre-pago. Intentando cambiar a pos-pago...\n", user_id);

            // Probabilidad de cambiar de pre-pago a post-pago (por ejemplo, 30% de probabilidad)
            if (rand() % 10 < 3) {
                type = POSTPAID;
                printf("[CLIENTE %d] Ahora es un cliente Pos-pago.\n", user_id);

                // Notificar al servidor que el cliente ha cambiado a post-pago
                snprintf(buffer, BUFFER_SIZE, "Cambio a pos-pago. \n");
                send(sockfd, buffer, strlen(buffer), 0);
            } else {
                printf("[CLIENTE %d] No se ha cambiado a pos-pago. No se pueden enviar más mensajes.\n", user_id);
                break;
            }
        }

        // Crear el mensaje con el tipo de cliente fijo
        snprintf(buffer, BUFFER_SIZE, "%d|%d|Mensaje %d del cliente %d (%s)", 
                 user_id, type, message_count + 1, user_id, type == POSTPAID ? "pos-pago" : "pre-pago");
        
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);
        printf("[CLIENTE %d] Respuesta del servidor: %s\n", user_id, buffer);

        message_count++;
        usleep(500000); // Pausa de 500ms entre mensajes
    }

    close(sockfd);
    return NULL;
}

int main() {
    srand(time(NULL) + getpid()); // Semilla única por cliente

    int sockfd;
    struct sockaddr_in server_addr;

    // Crear socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error al crear el socket");
        return -1;
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Conectar al servidor
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al conectar al servidor");
        return -1;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, send_messages, &sockfd);
    pthread_join(thread, NULL);

    return 0;
}