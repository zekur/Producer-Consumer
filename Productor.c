#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define NUMERO 1010
#define ARCHIVO "/bin/ls"
#define tam_medio_buffer 3
#define FlagB1 -1
#define FlagB2 -2
#define FlagEOF -3

// funcion que permite calcular diferencia de tiempos con precision de
// microsegundos
float time_diff(struct timeval *start, struct timeval *end) {
  return (end->tv_sec - start->tv_sec) + 1e-6 * (end->tv_usec - start->tv_usec);
}

// estructura que se va a almacenar en la memoria compartida
typedef struct {
  unsigned int ID;
  float Tiempo;
  double Dato;
} memcomp;

// estructura del mensaje que se va a usar para avisarle al consumidor que puede
// leer del buffer 1 o 2
typedef struct {
  long Id_Aviso;
  int flag;
} Aviso;

Aviso Aviso_fill, Aviso_read;

// estructuras de tiempo para definir un tiempo de inicio y otro de fin para
// tomar el tiempo que tarda el programa en leer datos
struct timeval start;
struct timeval end;

FILE *Fichero;
int id=1;
// Defino las funciones de llenado del buffer
void Llenar_Buffer_1(memcomp *Memoria) {
  int k = 0;
  do {
    fscanf(Fichero, "%lf", &Memoria[k].Dato);
    gettimeofday(&end, NULL);
    if (feof(Fichero)) {
      Aviso_fill.flag = k;
      return;
    }
    Memoria[k].ID = id++;
    Memoria[k].Tiempo = time_diff(&start, &end);

    printf("lleno el buffer 1\n");
    printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID, Memoria[k].Tiempo,
           Memoria[k].Dato);
    k++;

  } while (k < tam_medio_buffer);

  Aviso_fill.flag = FlagB1;
}

void Llenar_Buffer_2(memcomp *Memoria) {
  int k = tam_medio_buffer;
  do {
    fscanf(Fichero, "%lf", &Memoria[k].Dato);
    gettimeofday(&end, NULL);
    if (feof(Fichero)) {
      Aviso_fill.flag = k;
      return;
    }
    Memoria[k].ID = id++;
    Memoria[k].Tiempo = time_diff(&start, &end);

    printf("lleno el buffer 2\n");
    printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID, Memoria[k].Tiempo,
           Memoria[k].Dato);
    k++;

  } while (k < 2 * tam_medio_buffer);

  Aviso_fill.flag = FlagB2;
}

int main(int argc, char **argv) {

  gettimeofday(&start, NULL);

  key_t Clave;
  int Id_Memoria;
  memcomp *Memoria = NULL;

  int Id_Cola_Mensajes;

  Aviso_fill.Id_Aviso = 1;

  {
    if (argc != 2) {
      printf("\n\n\tModo de uso: Nombre_Programa Nombre_Archivo\n");
      exit(1);
    }

    // Consigo la clave usando ARCHIVO y NUMEMER

    Clave = ftok(ARCHIVO, NUMERO);
    if (Clave == -1) {
      printf("No consegui  clave para memoria compartida\n");
      exit(1);
    }

    // Solicito al sistema operativo la memoria compartida.
    Id_Memoria =
        shmget(Clave, 2 * tam_medio_buffer * sizeof(memcomp), 0666 | IPC_CREAT);
    if (Id_Memoria == -1) {
      printf("No consegui Id para memoria compartida\n");
      exit(2);
    }

    // Asocio el ID de memoria a una variable local
    Memoria = (memcomp *)shmat(Id_Memoria, (const void *)NULL, 0);

    if (Memoria == NULL) {
      printf("No consegui asociar la memoria compartida a una variable\n");
      exit(3);
    }

    Id_Cola_Mensajes = msgget(Clave, 0600 | IPC_CREAT);
    if (Id_Cola_Mensajes == -1) {
      printf("Error al obtener identificador para cola mensajes\n");
      exit(-1);
    }

    if ((Fichero = fopen(argv[1], "r")) == NULL) {
      printf("\n\n ERROR EN LA APERTURA DEL ARCHIVO\n\n");
      return 0;
    }
  }
  Llenar_Buffer_1(Memoria);
  msgsnd(Id_Cola_Mensajes, (struct msgbuf *)&Aviso_fill,
         sizeof(Aviso_fill.flag), 0);
  Llenar_Buffer_2(Memoria);
  msgsnd(Id_Cola_Mensajes, (struct msgbuf *)&Aviso_fill,
         sizeof(Aviso_fill.flag), 0);
  do {
    msgrcv(Id_Cola_Mensajes, (struct msgbuf *)&Aviso_read, sizeof(int), 2, 0);
    if (Aviso_read.flag == FlagB1) {
      Llenar_Buffer_1(Memoria);
    } else if (Aviso_read.flag == FlagB2) {
      Llenar_Buffer_2(Memoria);
    }
    msgsnd(Id_Cola_Mensajes, (struct msgbuf *)&Aviso_fill,
           sizeof(Aviso_fill.flag), 0);
  } while (Aviso_read.flag != FlagEOF);

  shmdt((const void *)Memoria);

  shmctl(Id_Memoria, IPC_RMID, (struct shmid_ds *)NULL);
  msgctl(Id_Cola_Mensajes, IPC_RMID, (struct msqid_ds *)NULL);

  exit(0);
}