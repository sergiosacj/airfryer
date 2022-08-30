#include <stdio.h>
#include <time.h>
#include <string.h>

#include <definitions.h>
#include <modbus.h>

void manage_time() {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  printf("%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

int main(int argc, char *argv[])
{
  while(1) {
    int option;
    printf("Escolha X tal que:\n");
    printf("X == 1 -> Solicitar Inteiro.\n");
    printf("X != 1 -> Encerrar programa.\n");
    scanf("%d", &option);
    float internal_temperature;

    switch (option) {
      case 1:
        internal_temperature = get_internal_temperature();
        printf("%f\n", internal_temperature);
      default:
        return 0;
    }
  }
}

