#include "kernel/types.h"
#include "user/user.h"

int
main(void) 
{
  int parent_fd[2], child_fd[2];
  char buf, msg = '0';
  
  pipe(parent_fd);
  pipe(child_fd);
  if (fork() == 0) {
    read(parent_fd[0], &buf, 1);
    printf("%d: received ping\n", getpid());
    write(child_fd[1], &msg, 1);        
  } else {
    write(parent_fd[1], &msg, 1);
    read(child_fd[0], &buf, 1);
    printf("%d: received pong\n", getpid());
  }
  exit(0);
}