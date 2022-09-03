#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include <definitions.h>
#include <modbus.h>
#include <gpio.h>
#include <lcd.h>
#include <pid.h>
#include <airfryer.h>

static void process_user_commands();
static void state_machine();
static void handle_sigint(int signum);
static void default_values();
static void start_heating();
static void stop_heating();
static void change_menu_option();
static void update_timer(int value);
static void control_internal_temperature(int reference_temperature);
static void create_csv();
static void update_csv();

static double reference_temperature;
static double internal_temperature;
static double control_signal;
static int milisecond_counter;
static int menu_option;
static bool heating;
DisplayLCD lcd;

void start_airfryer() {
  default_values();

  printf("Criando arquivo de log: %s\n", CSV_FILE_NAME);
  create_csv();

  printf("Configurando constantes do PID...\n");
  pid_setup_constants(KP, KI, KD);

  printf("Configurando tratamento de sinal SIGINT...\n");
  signal(SIGINT, handle_sigint);

  printf("Reiniciando tempo para o tempo mínimo (1 minuto).\n");
  send_timer(lcd.timer);

  printf("Aguardando comando para ligar a airfryer...\n");
  while (1) {
    int user_command = get_user_commands();
    if (user_command == USER_CMD_TURN_ON)
      break;
    else if (user_command != 0)
      printf("Nenhum comando será processado com a airfryer desligada!\n");
  }

  send_system_state(1);

  lcd = start_display();
  if (lcd.fd == -1) {
    printf("Falha ao iniciar display LCD!");
    stop_airfryer();
  }

  while (1) {
    milisecond_counter = 0;
    while (!heating) {
      milisecond_counter++;
      milisecond_counter%=10;
      if (milisecond_counter == 5 || milisecond_counter == 0)
        process_user_commands();
      if (milisecond_counter == 0)
        draw(&lcd);
      usleep(100000);
    }
    printf("Iniciando processo.\n");
    state_machine();
    printf("Processo finalizado.\n");
    default_values();
  }
}

static void process_user_commands() {
  int user_command = get_user_commands();
  switch (user_command) {
    case USER_CMD_TURN_OFF:
      stop_airfryer();
    case USER_CMD_START_HEATING:
      if (!heating)
        start_heating();
      break;
    case USER_CMD_CANCEL_PROCESS:
      if (heating)
        stop_heating();
      break;
    case USER_CMD_INCREASE_TIMER:
      update_timer(60);
      break;
    case USER_CMD_DECREASE_TIMER:
      update_timer(-60);
      break;
    case USER_CMD_MENU:
      if (heating)
        printf("Proibido trocar o modo do menu durante o funcionamento.\n");
      else
        change_menu_option();
      break;
    default:
      break;
  }
}

static void state_machine() {
  while (heating) {
    milisecond_counter++;
    milisecond_counter%=10;
    if (milisecond_counter == 5 || milisecond_counter == 0) {
      reference_temperature = get_reference_temperature();
      control_internal_temperature(reference_temperature);
      process_user_commands();
    }
    if (milisecond_counter == 0) {
      update_timer(-1);
      draw(&lcd);
      update_csv();
    }
    if (lcd.timer <= 0)
      heating = false;
    usleep(100000);
  }
}

static void start_heating() {
  heating = true;
  int count = 0;
  while (reference_temperature > internal_temperature) {
    reference_temperature = get_reference_temperature();
    control_internal_temperature(reference_temperature);
    usleep(200000);
    if (count == 0)
      draw_heating_cooling(&lcd, 'H');
    count++; count%=5;
  }
}

static void stop_heating() {
  heating = false;
  int count = 0;
  reference_temperature = 25; // environment temperature
  while (reference_temperature < internal_temperature) {
    control_internal_temperature(reference_temperature);
    usleep(200000);
    if (count == 0)
      draw_heating_cooling(&lcd, 'C');
    count++; count%=5;
  }
}

static void change_menu_option() {
  // menu_option
  // 0 -> manual
  // 1 -> alimento 1
  // 2 -> alimento 2

  menu_option++;
  menu_option%=3;
  switch (menu_option) {
    case 1:
      lcd.menu_option = 1;
      reference_temperature = 50;
      lcd.timer = 60 * 3;
      break;
    case 2:
      lcd.menu_option = 2;
      reference_temperature = 40;
      lcd.timer = 60 * 2;
      break;
    default:
      lcd.menu_option = 'M';
      break;
  }

  printf("Alterando opção do menu: %c\n", lcd.menu_option);
}

static void update_timer(int value) {
  lcd.timer += value;
  if (lcd.timer < TIME_MIN) {
    lcd.timer = TIME_MIN;
  }
  printf("Atualizando timer: %d\n", lcd.timer);
  send_timer(lcd.timer);
}

static void control_internal_temperature(int reference_temperature) {
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

  printf("Enviando sinal de controle: %lf\n", control_signal);
  send_control_signal(control_signal);
}

void stop_airfryer() {
  int ret = 1;
  while (ret != 0)
    ret = raise(SIGINT);
}

static void handle_sigint(int signum) {
  printf("Desligando airfryer...");
  stop_heating();
  send_timer(0);
  send_working_state(0);
  send_system_state(0);
  clear_display(&lcd);
  int ret = 1;
  while (ret != 0)
    ret = raise(SIGUSR1);
}

static void default_values() {
  printf("Reiniciando valores iniciais.\n");
  reference_temperature = 0;
  internal_temperature = 0;
  control_signal = 0;
  milisecond_counter = 0;
  timer = TIME_MIN;
  menu_option = 0;
  heating = false;
}

static void create_csv() {
  FILE *file = fopen(CSV_FILE_NAME, "w");
  fprintf(file, "Dia,Mês,Ano,Hora,Minuto,Segundo,TI,TR,Sinal de Controle\n");
  fclose(file);
}

static void update_csv() {
  FILE *file = fopen("log.csv", "a");

  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  fprintf(file, "%02d,%02d,%02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year + 1900);
  fprintf(file, ",%02d,%02d,%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  fprintf(file, ",%lf,%lf,%lf", internal_temperature, reference_temperature, control_signal);

  fclose(file);
}
