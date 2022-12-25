#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#define BUF_SIZE 2

#define READ_END 0
#define WRITE_END 1

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\n"

char *ft_read(int fd){
	char *rd;
	char char_rd[1];
	int nbCharRead;
	if (!(rd = malloc(sizeof(char))))
		return NULL;
	while((nbCharRead = read(fd, char_rd, 1))){
		rd = realloc(rd,strlen(rd) + 1);
		strncat(rd, char_rd, 1);
	}
	rd[strlen(rd)] = '\0';
	return (rd);
}

char *sh_read_line(FILE *f){
  char *line = NULL;
  size_t bufsize = 0; // donc getline realise l'allocation
  getline(&line, &bufsize, f);
  return line;
}

char ** sh_split_line( char *line){
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void handler_sigint(int sig)
{
	(void)sig;
	printf("\nParent finishing.\n");
  	exit(EXIT_SUCCESS);
}

char *sh_cmd(){
  	char *prompt = "l3miage shell > ";
  	char *line;

	printf("%s",prompt);
	fflush(stdout);

	line = sh_read_line(stdin);
	if (!line || *line == '\0')
		return (NULL);
	return(line);
}

void process_fils(char **args, int *fd) {
	write(1, "\nCommand executed by child...\n", 31);
	close(STDOUT_FILENO);
	close(fd[0]);
	dup2(fd[1], STDOUT_FILENO);
	execvp(args[0], args);
}

void process_pere(int *fd) {
	char *message;
	close(fd[1]);
	message = ft_read(fd[0]);
	write(1, "\nresult received by father ...\n", 32);
	write(1, message, strlen(message));
	write(1, "\n", 1);
}

void create_pipe(int *fd){

	if (pipe(fd) == -1) {
    	fprintf(stderr,"Pipe failed");
    	return;
  	}
}

void close_pipe(int *fd){
	close(fd[1]);
	close(fd[0]);
}

void sh_execute(){

	pid_t pid;
	char **args;
	char *cmd;
	int status;

	/* CREATION DU pipe */
  	int fd[2]; /* 1 pipe = deux extremites : une pour lire / une pour �crire */

	while (1){
		cmd = sh_cmd();
		if (!cmd)
			continue;
		args = sh_split_line(cmd);
		create_pipe(fd);
		pid = fork();
		switch (pid) {
			case -1 : perror("Erreur de création du processus");
				exit(1);
			case 0 : /* Ce code s'exécute chez le fils */
				process_fils(args, fd);
				exit(1);
			default : /* Ce code s'exécute chez le père */
				process_pere(fd);
				wait(&status);
				close_pipe(fd);
				free(args);
				free(cmd);
		}
	}
}

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	struct sigaction action;
	memset (&action, 0, sizeof (action));
	action.sa_handler = handler_sigint; /* Fonction handler */
	sigaction(SIGINT, &action, NULL);

	sh_execute();
  	exit(EXIT_SUCCESS);
}
