#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#define BUF_SIZE 256

void handler_sigint(int sig)
{
	(void)sig;
	printf("Parent finishing.\n");

  	//Deleting semaphores..
  	sem_unlink ("/toti");

  	//Deleting Shared Memory.
  //	shmctl (shmid, IPC_RMID, NULL);
  	exit(EXIT_SUCCESS);

}

void make_message(int num , char *message){
	/* Remplit le buffer "message" */
	char buftime [26];
	time_t timer ;
	struct tm *tm_info ;
	time(&timer ) ;
	tm_info = localtime(&timer);
	strftime(buftime, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	sprintf(message,"%s %d at %s\n", "Hello, I'm child number", num, buftime);
}

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;
  	int i;
  	char *virtualaddr;
 	int shmid;
  	sem_t *s;
  	key_t key;

	struct sigaction action;
	memset (&action, 0, sizeof (action));
	action.sa_handler=handler_sigint; /* Fonction handler */
	sigaction(SIGINT, &action, NULL);

  	/*----- Attaching the shared mem to my address space  */
  	key = ftok(argv[0],'R'); /* Generation de la key */
  	shmid = shmget(key, 1024, 0644|IPC_CREAT); /* Creation du segment
                                                memoire : 1024 octets */
  	if (0 > shmid){
    	perror("Shared Mem creation error\n");
    	exit(1);
  	}
  	/* => virtualaddr will be available across fork ! */
  	virtualaddr = shmat(shmid, (void*)0, 0); /* Attachement à l'espace mem du processus */

  	/*--- Create POSIX Named Semaphores, and initialising with 1 */
  	int init_sem_value = 1; /* Dijkstra sem */
  	s = sem_open("/tmp/toti", O_CREAT|O_RDWR, 0644, init_sem_value);

  	switch (fork()){ /*----- child 1 */
  		case -1:
    		printf("Error forking child 1!\n");  exit(1);
  		case 0:
    		{
      			//s = sem_open ("/toti", O_RDWR);
     			printf("\nChild 1 executing...\n");
				sem_wait(s);
				while (1) {
      				char buf[BUF_SIZE];

       				make_message (1,virtualaddr);
      				sleep(1); /* La fabrication du message prend un peu de temps */
      				sem_post(s);

      				/*Child 1 reading from shared mem */
      				sem_wait(s);
      				strcpy (buf, virtualaddr);
      				printf("Message received by child 1: %s\n", buf);
      			}
    		}
  		default: break;
  	}

  	switch (fork()){ /*----- child 2 */
  		case -1:
    		printf("Error forking child 2!\n");
			exit(1);
  		case 0:
    		{
      			//s = sem_open ("/toti", O_RDWR);
				printf("\nChild 2 executing...\n");

				while (1) {
      				char buf[BUF_SIZE];
      				/*Child 2 reading from shared memory*/
      				sem_wait(s);
      				strcpy (buf, virtualaddr);
      				printf("Message received by child 2: %s\n", buf);

      				/*Child 2 writing in shared mem*/
      				make_message(2, virtualaddr);
      				sleep(1); /* La fabrication du message prend un peu de temps*/
      				sem_post(s);
				}
    		}
  		default:
    		break;
  		}

  	printf("Parent waiting for children completion...\n");
  	for(i=0;i<=1;i++){
    	if (wait(NULL) == -1){
      		printf("Error waiting.\n");
      		exit(EXIT_FAILURE);
    	}
  	}
  	printf("Parent finishing.\n");

  	//Deleting semaphores..
  	sem_unlink ("/tmp/toti");

  	//Deleting Shared Memory.
  	shmctl (shmid, IPC_RMID, NULL);
  	exit(EXIT_SUCCESS);
}
