#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <definitions.h>
#include <modbus.h>
#include <lcd.h>
#include <pid.h>

static float reference_temperature = 0;
static double internal_temperature = 0;
static double control_signal = 0;
static int milisecond_counter = 0;
static int timer = 0;

void start_airfryer() {
  printf("Aguardando comando para ligar a airfryer...\n");
  pid_setup_constants(KP, KI, KD);
  signal(SIGINT, handle_sigint);
  // SETUP INITIAL VALUTE TO ZERO

  while (1) {
    int user_command = get_user_commands();
    if (user_command == USER_CMD_TURN_ON)
      break;
    else if (user_command != 0)
      printf("Nenhum comando será processado com a airfryer desligada!\n");
  }

  send_system_state(1);
}

void state_machine() {
  switch (milisecond_counter) {
    default:
      break;
  }
  usleep(100000);
}

void control_internal_temperature() {
  reference_temperature = get_reference_temperature();
  pid_update_reference(reference_temperature);

  internal_temperature = get_internal_temperature();
  control_signal = pid_control(internal_temperature);

  if (control_signal < 100) {
    update_resistor(0);
    update_fan(-1 * control_signal);
  } else {
    update_fan(0);
    update_resistor(control_signal);
  }

  send_control_signal(control_signal);
}

void stop_airfryer() {
  int ret = 1;
  while (ret != 0)
    ret = raise(SIGINT);
}

void handle_sigint(int signum) {
  send_working_state(0);
  send_system_state(0);
  int ret = 1;
  while (ret != 0)
    ret = raise(SIGUSR1);
}
