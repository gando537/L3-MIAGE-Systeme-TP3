/* Exemple de tube/pipe : Processus unique.
   Author : GM */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]){

  /* CREATION DU pipe */
  int fd[2]; /* 1 pipe = deux extremites : une pour lire / une pour �crire */
  if (pipe(fd) == -1) {
    fprintf(stderr,"Pipe failed");
    return 1;
  }

  char msg[BUFFER_SIZE] = "Bienvenu !";
  /* Write to the pipe */
  write(fd[WRITE_END], msg, strlen(msg)+1);
  printf("Le proc ecrit : %s\n", msg);
  fflush(stdout);
  /* RAZ msg in stack */
  memset(msg, 0, BUFFER_SIZE);
  /* Read the pipe */
  read(fd[READ_END], msg, BUFFER_SIZE);
  printf("Le proc lit : %s\n",msg);
  fflush(stdout);
  /* Close the pipe */
  close(fd[READ_END]);
  close(fd[WRITE_END]);
  return 0;
}

