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
  Message msg = {
    .address = 0,
    .code = 1,
    .subcode = 2
  };
  get_integer(msg);
  return 0;
}
