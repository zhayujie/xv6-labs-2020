#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i, j, k, pid, len;
  int first_blank;
  char args[MAXARG][32];
  char *p[MAXARG];
  char buf;

  if(argc < 2) {
    fprintf(2, "usage: xargs <cmd> ...\n");
    exit(1);
  }
  while (1)
  {
    // init
    i = 0;
    first_blank = 0;
    memset(args, 0, MAXARG * 32);

    // push command args and stdin args together
    // the first arg must be command itself
    for(j = 1; j < argc; j++) {
      strcpy(args[i++], argv[j]);
    }
    j = 0;  
    while (i < MAXARG-1) {
      if ((len = read(0, &buf, 1)) <= 0) {
        // CTRL+D 
        exit(0);
      }
      if (buf == '\n') {
        break;
      }
      if (buf == ' ') {
        if (first_blank) {
          i++;
          j = 0;
          first_blank = 0;
        }
        continue;
      }
      args[i][j++] = buf;
      first_blank = 1;
    }
    // i: num of args
    for (k = 0; k <= i; k++) {
      p[k] = args[k];
    }
    // the last arg must be 0
    p[k] = 0;
    // exec command
    if ((pid = fork()) == 0) {
      exec(argv[1], p);
      exit(0);
    }
    wait(0);
  }
}
