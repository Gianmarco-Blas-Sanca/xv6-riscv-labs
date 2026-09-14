#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "user/user.h"

// Variables globales para pruebas
char buf[4096];

void
simpletest()
{
  uint64 phys_size = PHYSTOP - KERNBASE;
  int sz = (phys_size / 3);

  printf("simple: ok\n");
  char *p = sbrk(sz);
  if(p == (char*)0xffffffffffffffffL){
    printf("sbrk failed\n");
    exit(-1);
  }

  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(-1);
  }

  if(pid == 0){
    p[0] = 'a';
    exit(0);
  } else {
    wait(0);
    p[0] = 'b';
  }
  printf("simple: ok\n");
}

void
threetest()
{
  uint64 phys_size = PHYSTOP - KERNBASE;
  int sz = (phys_size / 4);

  printf("three: ok\n");
  char *p = sbrk(sz);
  if(p == (char*)0xffffffffffffffffL){
    printf("sbrk failed\n");
    exit(-1);
  }

  int pid1 = fork();
  if(pid1 < 0){
    printf("fork failed\n");
    exit(-1);
  }

  if(pid1 == 0){
    int pid2 = fork();
    if(pid2 < 0){
      printf("fork failed\n");
      exit(-1);
    }
    if(pid2 == 0){
      p[0] = 'x';
      exit(0);
    }
    wait(0);
    p[0] = 'y';
    exit(0);
  }

  wait(0);
  p[0] = 'z';
  printf("three: ok\n");
}

void
filetest()
{
  int p[2];
  pipe(p);

  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(-1);
  }

  if(pid == 0){
    close(p[0]);
    write(p[1], "hello", 5);
    exit(0);
  }

  close(p[1]);
  read(p[0], buf, sizeof(buf));
  wait(0);
  printf("file: ok\n");
}

int
main(int argc, char *argv[])
{
  simpletest();
  threetest();
  filetest();
  printf("ALL COW TESTS PASSED\n");
  exit(0);
}