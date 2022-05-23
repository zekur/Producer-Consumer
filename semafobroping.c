#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define tam_medio_buffer 4

FILE *Fichero1, *Fichero2;
sem_t sem_read_B1;
sem_t sem_fill_B1;
sem_t sem_read_B2;
sem_t sem_fill_B2;
struct timeval start;
struct timeval end;

pthread_mutex_t mutexBuffer;

typedef struct {
  unsigned int ID;
  float Tiempo;
  double Dato;
} memcomp;

memcomp Memoria[2 * tam_medio_buffer];
int k = 0, whileend = -1, i = 0;

float time_diff(struct timeval *start, struct timeval *end) {
  return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

void *producer(void *args) {
  while (whileend == -1) {
    sem_wait(&sem_read_B1);

    // Add to the buffer
    k = 0;
    do {
      fscanf(Fichero1, "%lf", &Memoria[k].Dato);
      gettimeofday(&end, NULL);
      if (feof(Fichero1)) {
        whileend = k;
        break;
      }
      Memoria[k].ID = k;
      Memoria[k].Tiempo = time_diff(&start, &end);

      printf("lleno el buffer1\n");
      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID, Memoria[k].Tiempo,
             Memoria[k].Dato);
      k++;

    } while (k < tam_medio_buffer);

    sem_post(&sem_fill_B1);

    if (whileend != -1) {
      break;
    }

    sem_wait(&sem_read_B2);

    do {
      fscanf(Fichero1, "%lf", &Memoria[k].Dato);
      gettimeofday(&end, NULL);
      if (feof(Fichero1)) {
        whileend = k;
        break;
      }
      Memoria[k].ID = k;
      Memoria[k].Tiempo = time_diff(&start, &end);

      printf("lleno el buffer2\n");
      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID, Memoria[k].Tiempo,
             Memoria[k].Dato);
      k++;

    } while (k < 2 * tam_medio_buffer);

    sem_post(&sem_fill_B2);
  }
  return 0;
}

void *consumer(void *args) {
  int end = 0;
  while (end == 0) {

    sem_wait(&sem_fill_B1);

    for (i = 0; i < tam_medio_buffer; i++) {
      if (i == whileend) {
        end = 1;
        break;
      }
      printf("Leyendo buffer1\n");
      fprintf(Fichero2, "%u,%0.6f,%lf\n", Memoria[i].ID, Memoria[i].Tiempo,
              Memoria[i].Dato);
      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[i].ID, Memoria[i].Tiempo,
             Memoria[i].Dato);
    }

    sem_post(&sem_read_B1);

    if (end == 1) {
      break;
    }

    sem_wait(&sem_fill_B2);

    for (i = tam_medio_buffer; i < 2 * tam_medio_buffer; i++) {
      if (i == whileend) {
        end = 1;
        break;
      }
      printf("Leyendo buffer2\n");
      fprintf(Fichero2, "%u,%0.6f,%lf\n", Memoria[i].ID, Memoria[i].Tiempo,
              Memoria[i].Dato);
      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[i].ID, Memoria[i].Tiempo,
             Memoria[i].Dato);
    }

    sem_post(&sem_read_B2);
  }
  return 0;
}

int main(int argc, char *argv[]) {

  gettimeofday(&start, NULL);
  pthread_t th[2];

  if ((Fichero1 = fopen(argv[1], "r")) == NULL) {
    printf("\n\n ERROR EN LA APERTURA DEL ARCHIVO\n\n");
    return 0;
  }

  if ((Fichero2 = fopen("Datos2.csv", "w")) == NULL) {
    printf("\n\n ERROR EN LA APERTURA DEL ARCHIVO\n\n");
    return 0;
  }

  sem_init(&sem_read_B1, 0, 1);
  sem_init(&sem_fill_B1, 0, 0);
  sem_init(&sem_read_B2, 0, 1);
  sem_init(&sem_fill_B2, 0, 0);

  if (pthread_create(&th[0], NULL, &producer, NULL) != 0) {
    printf("Failed to create thread");
  }
  if (pthread_create(&th[1], NULL, &consumer, NULL) != 0) {
    printf("Failed to create thread");
  }

  if (pthread_join(th[0], NULL) != 0) {
    printf("Failed to join thread");
  }
  if (pthread_join(th[1], NULL) != 0) {
    printf("Failed to join thread");
  }

  sem_destroy(&sem_read_B1);
  sem_destroy(&sem_fill_B1);
  sem_destroy(&sem_read_B2);
  sem_destroy(&sem_fill_B2);

  fclose(Fichero1);
  fclose(Fichero2);
  return 0;
}