#include "kernel/types.h"
#include "user/user.h"

void
primes(int *p) {
  int n, first, len;
  int in, out;
  int fd[2];
  in = p[0];

  // close output fd
  close(p[1]);
  // read first num from prev progress (prime num)
  if ((len = read(in, &first, sizeof(int))) <= 0) {
    exit(0);
  }
  printf("prime %d\n", first);

  pipe(fd);
  out = fd[1];
  if (fork() == 0) {
    // child
    primes(fd);
  } else {
    // parent
    while ((len = read(in, &n, sizeof(int))) > 0) {
      if (n % first != 0) {
        write(out, &n, sizeof(int));
      }
    }
    close(out);
    wait(0);
    exit(0);
  } 
}

int
main(void) {
  int i;
  int p[2];
  
  pipe(p);
  if (fork() == 0) {
    primes(p);
  } else {
    for (i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(int));
    }
    close(p[1]);
    wait(0);
  }
  exit(0);
}
