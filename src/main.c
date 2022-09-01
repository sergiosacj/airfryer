#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <lcd.h>
#include <gpio.h>
#include <modbus.h>
#include <airfryer.h>

void test_gpio();
void test_display();
void test_uart();

int main(int argc, char *argv[])
{
  DisplayLCD lcd = start_display();
  if (lcd.fd == -1) {
    printf("Falha ao iniciar display!");
    return 1;
  }

  start_airfryer();
  test_gpio();
  test_display(&lcd);
  test_uart();
  stop_airfryer();
}

void test_gpio() {
  setup_gpio();
  update_resistor(100);
  sleep(4);
  update_fan(100);
  sleep(2);
  update_resistor(0);
  update_fan(0);
}

void test_display(DisplayLCD *lcd) {
  draw(lcd);
  sleep(10);
  clear_display(lcd);
}

void test_uart() {
  while(1) {
    int option;
    printf("Escolha X tal que:\n");
    printf("X == 1 -> get_internal_temperature.\n");
    printf("X == 2 -> get_reference_temperature.\n");
    printf("X == 3 -> get_user_commands.\n");
    printf("X == 4 -> send_control_signal.\n");
    printf("X == 5 -> send_system_state.\n");
    printf("X == 6 -> send_working_state.\n");
    printf("X == 7 -> send_timer.\n");
    printf("else -> Encerrar programa.\n");
    scanf("%d", &option);
    float value_f;
    int value_i = 0;

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
        send_system_state(1);
        break;
      case 6:
        send_working_state(1);
        break;
      case 7:
        send_timer(1);
        break;
      default:
        value_i = -1;
        break;
    }
    if (value_i == -1)
      break;
  }
}
