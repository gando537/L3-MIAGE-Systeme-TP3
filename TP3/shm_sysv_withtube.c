#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#define BUF_SIZE 256

#define READ_END 0
#define WRITE_END 1

void handler_sigint(int sig)
{
	(void)sig;
	printf("Parent finishing.\n");

  	//Deleting Shared Memory.
  //	shmctl (shmid, IPC_RMID, NULL);
  	exit(EXIT_SUCCESS);

}

char *ft_read(int fd){
	char *rd;
	char char_rd[2];
	int n;
	if (!(rd = malloc(sizeof(char))))
		return NULL;
	do{
		n = read(fd, char_rd, 1);
		rd = realloc(rd,strlen(rd) + 1);
		char_rd[1] = '\0';
		strncat(rd, char_rd, 1);
	}while (n > 0);

	printf("1\n");
	rd[strlen(rd)] = '\0';
	return (rd);
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

// void process_fils_1(int *fd1, int *fd2) {
// 	printf("\nChild 1 executing...\n");
// 	char message[BUF_SIZE];
// 	close(fd1[0]);
// 	close(fd2[1]);
// 	while(1){
// 		make_message(1, message);
// 		write(fd1[1], message, strlen(message));
// 		sleep(1);
// 		read(fd2[0], message, BUF_SIZE);
// 		write(1, "Message received by child 1: ", 30);
// 		write(1, message, strlen(message));
// 		write(1, "\n", 1);
// 		sleep(1);
// 	}
// }

// void process_fils_2(int *fd1, int *fd2) {
// 	printf("\nChild 2 executing...\n");
// 	char message[BUF_SIZE];
// 	close(fd1[1]);
// 	close(fd2[0]);
// 	while(1){
// 		read(fd1[0], message, BUF_SIZE);
// 		write(1, "Message received by child 2: ", 30);
// 		write(1, message, strlen(message));
// 		write(1, "\n", 1);
// 		sleep(1);
// 		make_message(2, message);
// 		write(fd2[1], message, strlen(message));
// 		sleep(1);
// 	}
// }

void process_fils_1(int *fd1, int *fd2) {
	printf("\nChild 1 executing...\n");
	char message_send[BUF_SIZE];
	char *message_recv;
	close(fd1[0]);
	close(fd2[1]);
	while(1){
		make_message(1, message_send);
		write(fd1[1], message_send, strlen(message_send));
		sleep(1);
		message_recv = ft_read(fd2[0]);
		write(1, "Message received by child 1: ", 30);
		write(1, message_recv, strlen(message_recv));
		free(message_recv);
		write(1, "\n", 1);
		sleep(1);
	}
}

void process_fils_2(int *fd1, int *fd2) {
	printf("\nChild 2 executing...\n");
	char message_send[BUF_SIZE];
	char *message_recv;
	close(fd1[1]);
	close(fd2[0]);
	while(1){
		message_recv = ft_read(fd1[0]);
		write(1, "Message received by child 2: ", 30);
		write(1, message_recv, strlen(message_recv));
		free(message_recv);
		write(1, "\n", 1);
		sleep(1);
		make_message(2, message_send);
		write(fd2[1], message_send, strlen(message_send));
		sleep(1);
	}
}

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;
  	int i;

	/* CREATION DU pipe */
  	int fd1[2]; /* 1 pipe = deux extremites : une pour lire / une pour �crire */
	int fd2[2]; /* 1 pipe = deux extremites : une pour lire / une pour �crire */

	struct sigaction action;
	memset (&action, 0, sizeof (action));
	action.sa_handler=handler_sigint; /* Fonction handler */
	sigaction(SIGINT, &action, NULL);

  	if (pipe(fd1) == -1 || pipe(fd2) == -1) {
    	fprintf(stderr,"Pipe failed");
    	return 1;
  	}

	if (fork() == 0)
		process_fils_1(fd1, fd2);
	else
		if (fork() == 0)
			process_fils_2(fd1, fd2);

  	printf("Parent waiting for children completion...\n");
  	for(i=0;i<2;i++){
    	if (wait(NULL) == -1){
      		printf("Error waiting.\n");
      		exit(EXIT_FAILURE);
    	}
  	}
  	printf("Parent finishing.\n");
  	exit(EXIT_SUCCESS);
}
