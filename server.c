#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h> 
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define POSTPAID 1
#define PREPAID 0
#define MAX_PREPAID_MESSAGES 10
#define MAX_CONCURRENT_MESSAGES 10

typedef struct {
    int user_id;
    int type; 
    char message[BUFFER_SIZE];
} Message;

Message postpaid_queue[BUFFER_SIZE];
Message prepaid_queue[BUFFER_SIZE];
int postpaid_count = 0;
int prepaid_count = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
sem_t semaphore;

int client_id_counter = 1;

void enqueue_message(Message *queue, int *count, Message msg) {
    queue[*count] = msg;
    (*count)++;
}

Message dequeue_message(Message *queue, int *count) {
    Message msg = queue[0];
    for (int i = 1; i < *count; i++) {
        queue[i - 1] = queue[i];
    }
    (*count)--;
    return msg;
}

void *process_messages(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        
        if (postpaid_count > 0) {
            Message msg = dequeue_message(postpaid_queue, &postpaid_count);
            pthread_mutex_unlock(&queue_mutex);
            printf("[SERVIDOR] Mensaje del usuario %d: %s\n", msg.user_id, msg.message);
        }
        else if (prepaid_count > 0) {
            Message msg = dequeue_message(prepaid_queue, &prepaid_count);
            pthread_mutex_unlock(&queue_mutex);
            printf("[SERVIDOR] Mensaje del usuario %d: %s\n", msg.user_id, msg.message);
        }
        else {
            pthread_cond_wait(&queue_cond, &queue_mutex);
            pthread_mutex_unlock(&queue_mutex);
        }

        usleep(500000);
    }
    return NULL;
}

void *handle_client(void *arg) {
    int client_fd = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int user_id = client_id_counter++;
    int type = (rand() % 2 == 0) ? PREPAID : POSTPAID;
    int message_count = 0;

    snprintf(buffer, BUFFER_SIZE, "Tipo de cliente: %s", type == POSTPAID ? "Pos-pago" : "Pre-pago");
    send(client_fd, buffer, strlen(buffer), 0);

    printf("[SERVIDOR] Usuario %d conectado, Tipo de cliente: %s.\n", user_id, type == POSTPAID ? "Pos-pago" : "Pre-pago");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("[SERVIDOR] Usuario %d desconectado.\n", user_id);
            close(client_fd);
            break;
        }

        if (strstr(buffer, "Cambio a pos-pago") != NULL) {
            pthread_mutex_lock(&queue_mutex);
            if (type == PREPAID && message_count >= MAX_PREPAID_MESSAGES) {
                printf("[SERVIDOR] Usuario %d cambiado a Pos-pago.\n", user_id);
                type = POSTPAID;
            }
            pthread_mutex_unlock(&queue_mutex);
        }

        pthread_mutex_lock(&queue_mutex);

        if (type == PREPAID && message_count >= MAX_PREPAID_MESSAGES) {
            snprintf(buffer, BUFFER_SIZE, "Usuario %d: Límite alcanzado. Cambie a pos-pago.", user_id);
            send(client_fd, buffer, strlen(buffer), 0);
        } else {
            Message msg = {user_id, type};
            strncpy(msg.message, buffer, BUFFER_SIZE);

            if (type == POSTPAID) {
                enqueue_message(postpaid_queue, &postpaid_count, msg);
            } else {
                enqueue_message(prepaid_queue, &prepaid_count, msg);
            }

            pthread_cond_signal(&queue_cond);

            snprintf(buffer, BUFFER_SIZE, "Mensaje procesado por ChismeGPT para usuario %d.", user_id);
            send(client_fd, buffer, strlen(buffer), 0);
            message_count++;
        }

        pthread_mutex_unlock(&queue_mutex);

        sem_wait(&semaphore); 

        usleep(500000);
        
        sem_post(&semaphore);
    }

    return NULL;
}

int main() {
    srand(time(NULL));
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    sem_init(&semaphore, 0, MAX_CONCURRENT_MESSAGES);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[ERROR] Error al crear socket del servidor");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] Error al hacer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("[ERROR] Error al escuchar");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Servidor iniciado y esperando conexiones...\n");

    pthread_t processor_thread;
    pthread_create(&processor_thread, NULL, process_messages, NULL);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("[ERROR] Error al aceptar conexión");
            continue;
        }

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, &client_fd);
        pthread_detach(client_thread);
    }

    close(server_fd);

    sem_destroy(&semaphore);

    return 0;
}