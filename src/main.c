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
    printf("X == 1 -> get_internal_temperature.\n");
    printf("X == 2 -> get_reference_temperature.\n");
    printf("X == 3 -> get_user_commands.\n");
    printf("X == 4 -> send_control_signal.\n");
    printf("X == 5 -> send_ref_signal.\n");
    printf("X == 6 -> send_system_state.\n");
    printf("X == 7 -> send_working_state.\n");
    printf("X == 8 -> send_timer.\n");
    printf("else -> Encerrar programa.\n");
    scanf("%d", &option);
    float value_f;
    int value_i;

    switch (option) {
      case 1:
        value_f = get_internal_temperature();
        printf("%f\n", value_f);
        break;
      case 2:
        value_f = get_reference_temperature();
        printf("%f\n", value_f);
        break;
      case 3:
        value_i = get_user_commands();
        printf("%d\n", value_i);
        break;
      case 4:
        send_control_signal(1);
        break;
      case 5:
        send_ref_signal(1);
        break;
      case 6:
        send_system_state(1);
        break;
      case 7:
        send_working_state(1);
        break;
      case 8:
        send_timer(1);
        break;
      default:
        return 0;
    }
  }
}

