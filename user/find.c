#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *re) 
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // open the dir
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0 || T_DIR != st.type){
    fprintf(2, "find: the first arg must be dir path\n");
    close(fd);
    return;
  }
  // descent into sub-dir
  while(read(fd, &de, sizeof(de)) == sizeof(de)) {
    // splice current path
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    if(de.inum == 0) {
      continue;
    }
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if(stat(buf, &st) < 0){
      printf("find: cannot stat %s\n", buf);
      continue;
    }
    switch(st.type) {
      case T_FILE:
        if (strcmp(re, de.name) == 0) {
          printf("%s\n", buf);
        }
        break;
      case T_DIR:
        // recursion
        if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
          find(buf, re);
        }
        break;
      }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find <path> <expression>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}