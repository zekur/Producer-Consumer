#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define tam_medio_buffer 7

#define END_WHILE -1

FILE *Fichero1, *Fichero2;
sem_t sem_read_B1;
sem_t sem_fill_B1;
sem_t sem_read_B2;
sem_t sem_fill_B2;
struct timeval start;
struct timeval end;
int id=1;

typedef struct {
  unsigned int ID;
  float Tiempo;
  double Dato;
} memcomp;

memcomp Memoria[2 * tam_medio_buffer];
int k_final=-1;

float time_diff(struct timeval *start, struct timeval *end) {
  return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

void *producer(void *args) {
  int k;
  while (!feof(Fichero1)) {
    sem_wait(&sem_read_B1);

    printf("lleno el buffer 1\n");
    for (k = 0; k < tam_medio_buffer; k++) 
    {
      fscanf(Fichero1, "%lf", &Memoria[k].Dato);
      gettimeofday(&end, NULL);
      if (feof(Fichero1)) {
        k_final = k;
        break;
      }
      Memoria[k].ID = id++;
      Memoria[k].Tiempo = time_diff(&start, &end);

      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID, Memoria[k].Tiempo,
             Memoria[k].Dato);
    }

    sem_post(&sem_fill_B1);

    if (feof(Fichero1)) {
      break;
    }

    sem_wait(&sem_read_B2);

    printf("lleno el buffer 2\n");
    for (k = tam_medio_buffer; k < 2*tam_medio_buffer; k++)
    {
      fscanf(Fichero1, "%lf", &Memoria[k].Dato);
      gettimeofday(&end, NULL);
      if (feof(Fichero1)) {
        k_final = k;
        break;
      }
      Memoria[k].ID = id++;
      Memoria[k].Tiempo = time_diff(&start, &end);
      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID, Memoria[k].Tiempo,
             Memoria[k].Dato);
    }

    sem_post(&sem_fill_B2);
  }
  return 0;
}

void *consumer(void *args) {
  int end_reading = 0,i;
  while (end_reading == 0) {

    sem_wait(&sem_fill_B1);

    printf("Leyendo buffer 1\n");
    for (i = 0; i < tam_medio_buffer; i++) {
      if (i == k_final) {
        end_reading = 1;
        break;
      }
      fprintf(Fichero2, "%u,%0.6f,%lf\n", Memoria[i].ID, Memoria[i].Tiempo,
              Memoria[i].Dato);
//      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[i].ID, Memoria[i].Tiempo,Memoria[i].Dato);
    }

    sem_post(&sem_read_B1);

    if (end_reading == 1) {
      break;
    }

    sem_wait(&sem_fill_B2);

    printf("Leyendo buffer 2\n");
    for (i = tam_medio_buffer; i < 2 * tam_medio_buffer; i++) {
      if (i == k_final) {
        end_reading = 1;
        break;
      }
      fprintf(Fichero2, "%u,%0.6f,%lf\n", Memoria[i].ID, Memoria[i].Tiempo,
              Memoria[i].Dato);
//      printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[i].ID, Memoria[i].Tiempo,Memoria[i].Dato);
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

  if(sem_init(&sem_read_B1, 0, 1)==-1)
  {
    printf("error en la creacion del semaforo\n");
  }
  if(sem_init(&sem_fill_B1, 0, 0)==-1)
  {
    printf("error en la creacion del semaforo\n");
  }
  if(sem_init(&sem_read_B2, 0, 1)==-1)
  {
    printf("error en la creacion del semaforo\n");
  }
  if(sem_init(&sem_fill_B2, 0, 0)==-1)
  {
    printf("error en la creacion del semaforo\n");
  }

  if (pthread_create(&th[0], NULL, &producer, NULL) != 0) {
    printf("Error en la creacion del hilo");
  }
  if (pthread_create(&th[1], NULL, &consumer, NULL) != 0) {
    printf("Error en la creacion del hilo");
  }


  if (pthread_join(th[0], NULL) != 0) {
    printf("Error al unir el hilo");
  }
  if (pthread_join(th[1], NULL) != 0) {
    printf("Error al unir el hilo");
  }

  sem_destroy(&sem_read_B1);
  sem_destroy(&sem_fill_B1);
  sem_destroy(&sem_read_B2);
  sem_destroy(&sem_fill_B2);

  fclose(Fichero1);
  fclose(Fichero2);
  return 0;
}