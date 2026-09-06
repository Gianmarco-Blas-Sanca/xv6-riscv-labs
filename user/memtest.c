#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  uint64 m1 = getfreemem();
  printf("Memoria libre inicial: %ld bytes\n", m1);

  sbrk(4096 * 10); // Solicitar 10 paginas
  uint64 m2 = getfreemem();
  printf("Memoria libre tras sbrk(10 paginas): %ld bytes\n", m2);
  printf("Diferencia de memoria: %ld bytes\n", m1 - m2);

  sbrk(-4096 * 10); // Liberar 10 paginas
  printf("Memoria libre tras liberar: %ld bytes\n", getfreemem());

  exit(0);
}