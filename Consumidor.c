#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define NUMERO 1010
#define ARCHIVO "/bin/ls"
#define tam_medio_buffer 3
#define FlagB1 -1
#define FlagB2 -2
#define FlagEOF -3

typedef struct 
{
	unsigned int ID;
	float Tiempo;
	double Dato;
}memcomp;

typedef struct 
{
	long Id_Aviso;
	int flag;
}Aviso;

int main( int argc, char **argv)
{
	key_t Clave;
	int Id_Memoria;
	memcomp *Memoria = NULL;

	int Id_Cola_Mensajes;
	Aviso Aviso_fill,Aviso_read;
	Aviso_read.Id_Aviso = 2;

	int k;

	FILE *Fichero;

	    Clave = ftok (ARCHIVO, NUMERO);
		if (Clave == -1)
		{
			printf("No consegui  clave para memoria compartida\n");
			exit(1);
		}
		
		//Solicito al sistema operativo la memoria compartida.
		Id_Memoria = shmget (Clave, 2*tam_medio_buffer*sizeof(memcomp), 0666 | IPC_CREAT);
		if (Id_Memoria == -1)
		{
			printf("No consegui Id para memoria compartida\n");
			exit (2);
		}

		//Asocio el ID de memoria a una variable local
		Memoria = (memcomp *)shmat (Id_Memoria, (const void *)NULL, 0);
	    
		if (Memoria == NULL)
		{
			printf("No consegui asociar la memoria compartida a una variable\n");
			exit (3);
		}

		Id_Cola_Mensajes = msgget (Clave, 0666 | IPC_CREAT);
		if (Id_Cola_Mensajes == -1)
		{
			printf("Error al obtener identificador para cola mensajes\n");
			exit (-1);
		}

		if((Fichero=fopen("Datos.csv","w"))==NULL)
		{
			printf("\n\n ERROR EN LA APERTURA DEL ARCHIVO\n\n");
			return 0;
		}
Aviso_read.flag=FlagB2;
	do
	{
		
		msgrcv (Id_Cola_Mensajes, (struct msgbuf *)&Aviso_fill,sizeof(int), 1, 0);
		if((Aviso_fill.flag == FlagB1)||((Aviso_fill.flag >= 0) && Aviso_fill.flag<tam_medio_buffer))
		{
			
			for(k=0; k<tam_medio_buffer; k++)
			{
				printf("Leyendo buffer1\n");
				fprintf(Fichero, "%u,%f,%lf\n", Memoria[k].ID,Memoria[k].Tiempo,Memoria[k].Dato);
				printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID,Memoria[k].Tiempo,Memoria[k].Dato);
				if(k==Aviso_fill.flag-1)
				{
					break;
				}
			}
			if(k==Aviso_fill.flag-1)
				{
					Aviso_read.flag=FlagEOF;
				}
			else
				{
					Aviso_read.flag=FlagB1;
				}
			}
		else if((Aviso_fill.flag == FlagB2)||((Aviso_fill.flag >= tam_medio_buffer) && Aviso_fill.flag<2*tam_medio_buffer))
		{
			
			for(k=tam_medio_buffer; k<2*tam_medio_buffer; k++)
			{
				printf("Leyendo buffer2\n");
				fprintf(Fichero, "%u,%0.6f,%lf\n", Memoria[k].ID,Memoria[k].Tiempo,Memoria[k].Dato);
				printf("id=%u,tiempo=%f,dato=%lf\n", Memoria[k].ID,Memoria[k].Tiempo,Memoria[k].Dato);
				if(k==Aviso_fill.flag-1)
				{
					break;
				}
			}
			if(k==Aviso_fill.flag-1)
				{
					Aviso_read.flag=FlagEOF;
				}
			else
				{
					Aviso_read.flag=FlagB2;
				}
		}
		msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Aviso_read, sizeof(Aviso_read.flag),0);
	}while(Aviso_read.flag!=FlagEOF);

	shmdt ((const void *) Memoria);
	shmctl (Id_Memoria, IPC_RMID, (struct shmid_ds *)NULL);
	//exit(0);

}