#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <definitions.h>
#include <modbus.h>
#include <lcd.h>
#include <pid.h>

void start_airfryer() {
  printf("Aguardando comando para ligar a airfryer...\n");

  while (1) {
    int value_i = get_user_commands();
    if (value_i == USER_CMD_TURN_ON)
      break;
    else
      printf("Nenhum comando será processado com a airfryer desligada!\n");
  }

  send_system_state(1);
}

void stop_airfryer() {
  // desligar ventoinha e resistor
  send_working_state(0);
  send_system_state(0);
}
