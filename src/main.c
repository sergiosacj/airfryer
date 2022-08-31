#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <definitions.h>
#include <modbus.h>
#include <lcd.h>

void manage_time() {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  printf("%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

int main(int argc, char *argv[])
{
  DisplayLCD lcd = start_display();
  if (lcd.fd == -1)
    printf("Falha ao iniciar display!");
    return 1;

  start_airfryer();
  stop_airfryer();
}
