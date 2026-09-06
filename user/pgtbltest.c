#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int
main(void)
{
  char *buf;
  unsigned int mask = 0;

  printf("Iniciando prueba de pgaccess...\n");

  // Reservar 32 paginas en el heap
  buf = sbrk(32 * PGSIZE);
  if (buf == (char *)-1) {
    printf("Error al asignar memoria con sbrk\n");
    exit(1);
  }

  // Limpiar bits previos llamando a pgaccess
  pgaccess(buf, 32, &mask);

  // Acceder unicamente a las paginas 1, 2 y 30
  buf[PGSIZE * 1] += 1;
  buf[PGSIZE * 2] += 1;
  buf[PGSIZE * 30] += 1;

  mask = 0;
  if (pgaccess(buf, 32, &mask) < 0) {
    printf("Fallo en pgaccess\n");
    exit(1);
  }

  printf("Mascara obtenida: 0x%x\n", mask);

  // Verificamos si los bits 1, 2 y 30 se encendieron (valor esperado: 0x40000006)
  if (mask == ((1U << 1) | (1U << 2) | (1U << 30))) {
    printf("Reto 2 SUPERADO: Bits de acceso detectados correctamente.\n");
  } else {
    printf("Reto 2 FALLIDO: La mascara no coincide con los accesos realizados.\n");
  }

  exit(0);
}