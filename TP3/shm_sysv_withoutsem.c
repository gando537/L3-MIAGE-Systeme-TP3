/*  Version "-1" d'un �change SHM System V 
    entre deux fils.
    Author : GM
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#define BUF_SIZE 256

int main(int argc, char *argv[]){
  (void)argc;
  (void)argv;
  int i;
  int shmid;
  key_t key;

  char *virtualaddr;   /* Cette variable h�rit�e par tous contient,
                          apres l'appel shmat, un pointeur sur le 
                          segment m�moire partag� */

  /*--------------------------------------------------------
   * Etape 1 : attacher la mem partagee � l'espace d'adressage du  pere */

  key = ftok(argv[0],'R'); /* Generer une cle : Cette clef permettra
                              aux fils de retrouver le segment memoire */
  shmid = shmget(key, 1024, 0644|IPC_CREAT); /* Creation du segment memoire : 1024 octets */
  if (0 > shmid){ /* La creation peut echouer !!! */
    perror("Shared Mem creation error\n");
    exit(1);  
  }
  
  virtualaddr = shmat(shmid, (void*)0, 0); /* Attachement � l'espace mem du processus :
                                              virtualaddr pointe sur cet espace*/
  /* => virtualaddr will be available across fork ! */
  
  /*--------------------------------------------------------
   * Etape 2 : Cr�ation du fils 1 
   * => Il ecrit une information dans la shm 
   *    puis il lit une information dans la shm
   */
  switch (fork()){
  case -1:
    printf("Error forking child 1!\n"); exit(1);
  case 0:
    {
      char buf[BUF_SIZE];
      printf("\nChild 1 executing...\n");

      /* Child 1 writing in shared mem : adresse h�ritee */
      strcpy (virtualaddr, "Bonjour, Je suis ");
      sleep(1); /* supposons que la fabrication du message prenne un
                   certain temps et qu'il soit NON ATOMIC */
      strcat (virtualaddr, "francais !");
      printf("Message sent by child 1: %s\n", virtualaddr);

      /* Child 1 reading from shared mem */
      strcpy (buf, virtualaddr);
      printf("Message received by child 1: %s\n", buf);

      /* printf("Exiting child 1...\n"); */
      _exit(0);
    }
    break;
  default:
    break;
  }

  /*--------------------------------------------------------
   * Etape 3 : Cr�ation du fils 2 
   *           Lui lit puis ecrit
   */  
  switch (fork()){
  case -1:
    printf("Error forking child 2!\n"); exit(1);
  case 0:
    {
      char buf[BUF_SIZE];
      printf("\nChild 2 executing...\n");
      
      /* Child 2 read from shared memory */
      strcpy (buf, virtualaddr);
      printf("Message received by child 2: %s\n", buf);
       
      /* Child 2 writing in shared mem */
      strcpy (virtualaddr, "Hello, I'm ");
      sleep(1); /* supposons que la fabrication du message prenne un
		   peu de temps et qu'il soit NON ATOMIC */
      strcat (virtualaddr, "english !");
       
      printf("Message sent by child 2: %s\n", virtualaddr);
          
      /* printf("Exiting child 2...\n"); */
      _exit(EXIT_SUCCESS);
    }
    break;  
  default: break;
  }
  
  /*--------------------------------------------------------
   *  Etape 4 : Wait 
   */  
  printf("Parent waiting for children completion...\n");
  for(i=0;i<=1;i++){
    if (wait(NULL) == -1){
      printf("Error waiting.\n");
      exit(EXIT_FAILURE);
    }
  }  
  printf("Parent finishing.\n");

  /*--------------------------------------------------------
   * Etape 5 : Deleting Shared Memory.
   */
  shmctl (shmid, IPC_RMID, NULL);
  exit(EXIT_SUCCESS);
}
