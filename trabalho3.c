#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>

int *Buffer;
int N, M, C;
int *quantidadePrimos;

int in = 0, out = 0;

sem_t sem_vazio, sem_cheio;
sem_t mutexProd, mutexCons;

int ehPrimo(long long int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (int i = 3; i <= sqrt(n); i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void Insere(int item) {
    sem_wait(&sem_vazio);
    sem_wait(&mutexProd);
    Buffer[in] = item;
    in = (in + 1) % M;
    sem_post(&mutexProd);
    sem_post(&sem_cheio);
}

int Retira() {
    sem_wait(&sem_cheio);
    sem_wait(&mutexCons);
    int item = Buffer[out];
    out = (out + 1) % M;
    sem_post(&mutexCons);
    sem_post(&sem_vazio);
    return item;
}

void *produtor(void *arg) {
    for (int i = 0; i < N; i++) {
        int numero = rand() % 100000;
        Insere(numero);
    }
    for (int i = 0; i < C; i++) {
        Insere(-1);
    }
    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    long id = (long)arg;
    int contadorLoc = 0;
    while (1) {
        int numero = Retira();
        if (numero == -1)
            break;
        if (ehPrimo(numero))
            contadorLoc++;
    }
    quantidadePrimos[id] = contadorLoc;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  
    srand((unsigned)time(NULL));

    N = atoi(argv[1]);
    M = atoi(argv[2]);
    C = atoi(argv[3]);

    Buffer = malloc(sizeof(int) * M);
    quantidadePrimos = malloc(C * sizeof(int));

    for (int i = 0; i < C; i++) {
        quantidadePrimos[i] = 0;
    }

    sem_init(&sem_vazio, 0, M);
    sem_init(&sem_cheio, 0, 0);
    sem_init(&mutexProd, 0, 1);
    sem_init(&mutexCons, 0, 1);

    pthread_t t_produtor;
    pthread_t t_consumidores[C];

    pthread_create(&t_produtor, NULL, produtor, NULL);
    for (long i = 0; i < C; i++) {
        pthread_create(&t_consumidores[i], NULL, consumidor, (void *)i);
    }

    pthread_join(t_produtor, NULL);
    for (int i = 0; i < C; i++) {
        pthread_join(t_consumidores[i], NULL);
    }

    int total = 0;
    int vencedor = 0;
    for (int i = 0; i < C; i++) {
        total += quantidadePrimos[i];
        if (quantidadePrimos[i] > quantidadePrimos[vencedor]) {
            vencedor = i;
        }
    }

    printf("Total de primos: %d\n", total);
    printf("Thread vencedora: %d com %d primos\n", vencedor, quantidadePrimos[vencedor]);

    free(Buffer);
    free(quantidadePrimos);

    sem_destroy(&sem_vazio);
    sem_destroy(&sem_cheio);
    sem_destroy(&mutexProd);
    sem_destroy(&mutexCons);

    return 0;
}
