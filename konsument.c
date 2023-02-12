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
void clear_sem();
void semafor_p(int sem_nr);
void semafor_v(int sem_nr);

int main()
{
	FILE *wyjscie;
	int seg_id;
	char litera;
	char *seg_wskaz;//wskaznik do pierwszego bajtu pamieci dzielonej
	key_t klucz;
	key_t klucz_pd;

	//srand(time(NULL));

	klucz_pd=ftok(".",'S');//klucz do segmentu pamieci dzielonej
	if(klucz_pd==-1)
	{
		perror("Blad funkcji ftok() (pd) (K)");
		exit(1);
	}

	seg_id=shmget(klucz_pd,1,0666);//uzyskiwanie dostepu do segmentu pamieci dzielonej
	if(seg_id==-1)
	{
		perror("Blad funckji shmget (K)");
		exit(1);
	}
	printf("Uzyskano dostep do segmentu pamieci dzielonej (K)\n");

	seg_wskaz=(char*)shmat(seg_id,0,0);//uzyskanie adresu pamieci dzielonej

	klucz=ftok(".",'F');//klucz dla semaforow
	if(klucz==-1)
	{
		perror("Blad funkcji ftok() (K)");
		exit(1);
	}

	create_sem(klucz);//uzyskanie dostepu do semaforow

	wyjscie=fopen("wy","w");

	if(wyjscie==NULL)
	{
		printf("Blad otwarcia pliku wyjsciowego (K)\n");
		if((shmdt(seg_wskaz))==-1)//odlaczanie pamieci dzielonej;
		{
			perror("Nie odlaczono segmantu pamieci dzielonej (K)");
		}
		exit(1);
	}
	do
	{
		semafor_p(1);//zamkniecie semafora po stronie konsumenta
		litera=*seg_wskaz;
		printf("Konsument pobiera z pd znak %c\n", litera);
		sleep(1);
		if(litera!=EOF)
		{
			putc(litera,wyjscie);
		}
		semafor_v(0);//otwarcie semafora po stronie producenta
	}while(litera!=EOF);

	fclose(wyjscie);
	if(shmdt(seg_wskaz)==-1)//odlaczanie pamieci dzielonej
	{
		perror("Nie odlaczono segmentu pamieci dzielonej (K)");
		exit(1);
	}
	printf("Odlaczono segment pamieci dzielonej (K)\n");

	clear_sem();//usuniecie semaforow
}

void create_sem(key_t klucz)
{
	semafor=semget(klucz,2,IPC_CREAT|0666);
	if(semafor==-1)
	{
		perror("Nie udalo sie uzyskac dostepu do semafora (K)");
		exit(1);
	}
	printf("Uzyskano dostep do semafora (K)\n");
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
		perror("Nie zamknieto semafora (K)");
		exit(1);
		}
	}
	printf("Semafor zamkniety (K)\n");
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
		perror("Nie otwarto semafora (K)");
		exit(1);
	}
	printf("Semafor otwarty (K)\n");
}

void clear_sem()
{
	int usun_sem;
	usun_sem=semctl(semafor,0,IPC_RMID,NULL);
	if(usun_sem==-1)
	{
		perror("Semafor nie zostal usuniety (K)");
		exit(1);
	}
	printf("Semafor usunieto (K)\n");
}
