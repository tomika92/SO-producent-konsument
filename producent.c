#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

int semafor;
void create_sem(key_t klucz);
void ini_sem(int sem, int wart);
void clear_sem();
void semafor_p(int sem_nr);
void semafor_v(int sem_nr);

int main()
{
	FILE *wejscie;
	int seg_id;
	char litera;
	char *seg_wskaz;//wskaznik do pierwszego bajtu pamieci
	key_t klucz;
	key_t klucz_pd;

	//srand(time(NULL));

	klucz_pd=ftok(".",'S');//tworzenie klucza dla segmantu pamieci dzielonej
	if(klucz_pd==-1)
	{
		perror("Blad funkcji ftok() (pd) (P)");
		exit(1);
	}

	seg_id=shmget(klucz_pd,1,IPC_CREAT|0666);//tworzenie segmatu pamieci dzielonej
	if(seg_id==-1)
	{
		perror("Blad funkcji shmget (P)");
		exit(1);
	}
	printf("Utworzono segment pamieci dzielonej (P)\n");

	seg_wskaz=(char*)shmat(seg_id,0,0);//uzyskanie adresu do pamieci dzielonej

	klucz=ftok(".",'F');//utworzenie klucza dla semaforow
	if(klucz==-1)
	{
		perror("Blad funkcji ftok() (sem) (P)");
		exit(1);
	}

	create_sem(klucz);//utworzenie zbioru semaforow
	ini_sem(0,1);//inicjalizacja semaforow
	ini_sem(1,0);

	wejscie=fopen("we","r");

	if(wejscie==NULL)
	{
		printf("Blad otwarcia pliku wejsciowego\n");
		clear_sem();

		int wartosc;
		wartosc=shmctl(seg_id,IPC_RMID,0);//usuwanie pamieci dzielonej
		if(wartosc==-1)
		{
			perror("Blad usuniecia pamieci dzielonej (P)");
			exit(1);
		}
		if((shmdt(seg_wskaz))==-1)//odlaczanie pamieci dzielonej
		{
			perror("Nie odlaczono segmantu pamieci dzielonej (P)");
			exit(1);
		}
		exit(1);
	}

	do
	{
		semafor_p(0);
		litera=getc(wejscie);
		printf("Producent wpisuje do pd znak %c \n", litera);
		sleep(1);
		*seg_wskaz=litera;
		semafor_v(1);
	}while(litera!=EOF);

	fclose(wejscie);

	int wartosc;
	wartosc=shmctl(seg_id,IPC_RMID,0);//usuniecie segmentu pamieci dzielonej
	if(wartosc==-1)
	{
		perror("Blad usuniecia pamieci dzielonej (P)");
		exit(1);
	}

	if(shmdt(seg_wskaz)==-1)//odlaczenie segmantu pamieci dzielonej
	{
		perror("Nie odlaczono segmentu pamieci dzielonej (P)");
		exit(1);
	}
	printf("Odlaczono segment pamieci dzielonej (P)\n");
}

void create_sem(key_t klucz)
{
	semafor=semget(klucz,2,IPC_CREAT|0666);
	if(semafor==-1)
	{
		perror("Nie udalo sie utworzyc semafora (P)");
		exit(1);
	}
	printf("Semafor zostal utworzony (P)\n");
}

void ini_sem(int sem, int wart)
{
	int wartosc;
	wartosc=semctl(semafor,sem,SETVAL,wart);
	if(wartosc==-1)
	{
		perror("Blad inicjowania semafora (P)");
		exit(1);
	}
	printf("Semafor zostal ustawiony (P)\n");
}

void semafor_p(int sem_nr)
{
	int zmiana;
	struct sembuf bufor_sem;
	bufor_sem.sem_num=sem_nr;
	bufor_sem.sem_op=-1;
	bufor_sem.sem_flg=0;
	zmiana=semop(semafor,&bufor_sem,1);
	if(zmiana==-1)
	{
		if(errno==EINTR)
		{
			semafor_p(sem_nr);
		}
		else
		{
			perror("Nie zamknieto semafora (P)");
			exit(1);
		}
	}
	printf("Semafor zamkniety (P)\n");
}

void semafor_v(int sem_nr)
{
	int zmiana;
	struct sembuf bufor_sem;
	bufor_sem.sem_num=sem_nr;
	bufor_sem.sem_op=1;
	bufor_sem.sem_flg=0;
	zmiana=semop(semafor,&bufor_sem,1);
	if(zmiana==-1)
	{
		perror("Nie otwarto semafora (P)");
		exit(1);
	}
	printf("Semafor otwarty (P)\n");
}

void clear_sem()
{
	int usun_sem;
	usun_sem=semctl(semafor,0,IPC_RMID,NULL);
	if(usun_sem==-1)
	{
		perror("Nie usunieto semafora (P)");
		exit(1);
	}
	printf("Semafor zostal usuniety (P)\n");
}