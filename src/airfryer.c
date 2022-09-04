#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include <environment_temperature.h>
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
static void start_cooling();
static void change_menu_option();
static void update_timer(int value);
static void control_internal_temperature(char temperature_name);
static void create_csv();
static void update_csv();
static void update_temperature();

static double control_signal;
static int milisecond_counter;
static int menu_option;
static bool heating;
DisplayLCD lcd;

void start_airfryer() {
  lcd = start_display();
  if (lcd.fd == -1) {
    printf("Falha ao iniciar display LCD!");
    stop_airfryer();
  }

  default_values();
  setup_gpio();

  printf("Criando arquivo de log: %s\n", CSV_FILE_NAME);
  create_csv();

  printf("Configurando constantes do PID...\n");
  pid_setup_constants(KP, KI, KD);

  printf("Configurando tratamento de sinal SIGINT...\n");
  signal(SIGINT, handle_sigint);

  printf("Aguardando comando para ligar a airfryer...\n");
  while (1) {
    int user_command = get_user_commands();
    if (user_command == -1) continue;
    if (user_command == USER_CMD_TURN_ON)
      break;
    else if (user_command != 0)
      printf("Nenhum comando será processado com a airfryer desligada!\n");
  }

  send_system_state(1);
  printf("Airfryer foi ligada!\n");

  while (1) {
    while (!heating) {
      milisecond_counter++;
      milisecond_counter%=10;
      if (milisecond_counter == 5 || milisecond_counter == 0) {
        update_temperature();
        process_user_commands();
      }
      if (milisecond_counter == 0)
        draw(&lcd);
      usleep(100000);
    }
    milisecond_counter = 0;
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
        start_cooling();
      break;
    case USER_CMD_INCREASE_TIMER:
      update_timer(60);
      break;
    case USER_CMD_DECREASE_TIMER:
      if (lcd.timer == 60)
        break;
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
  send_working_state(1);
  while (heating) {
    milisecond_counter++;
    milisecond_counter%=10;
    if (milisecond_counter == 0) {
      update_timer(-1);
      draw(&lcd);
      update_csv();
    }
    if (milisecond_counter == 5 || milisecond_counter == 0) {
      control_internal_temperature('R');
      process_user_commands();
    }
    if (lcd.timer <= 0)
      heating = false;
    usleep(100000);
  }
  start_cooling();
}

static void start_heating() {
  printf("Aquecendo...\n");
  heating = true;
  int count = 0;
  while (lcd.reference_temperature > lcd.internal_temperature) {
    control_internal_temperature('R');
    usleep(200000);
    if (count == 0)
      draw_heating_cooling(&lcd, 'H');
    count++; count%=5;
  }
}

static void start_cooling() {
  printf("Resfriando...\n");
  heating = false;
  int count = 0;
  double acceptable_error = ACCEPTABLE_ERROR;
  while (lcd.environment_temperature < lcd.internal_temperature - acceptable_error) {
    control_internal_temperature('E');
    usleep(200000);
    if (count == 0)
      draw_heating_cooling(&lcd, 'C');
    count++; count%=5;
  }
  send_working_state(0);
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
      lcd.menu_option = 'A';
      lcd.timer = 60 * 3;
      send_timer(lcd.timer);
      break;
    case 2:
      lcd.menu_option = 'B';
      lcd.timer = 60 * 2;
      send_timer(lcd.timer);
      break;
    default:
      lcd.menu_option = 'M';
      lcd.timer = TIME_MIN;
      send_timer(lcd.timer);
      break;
  }

  printf("Alterando opção do menu: %c\n", lcd.menu_option);
}

static void update_timer(int value) {
  if (lcd.timer + value <= 0 && value == TIME_MIN)
    return;
  lcd.timer += value;
  printf("Atualizando timer: %d\n", lcd.timer);
  send_timer(lcd.timer);
}

static void control_internal_temperature(char temperature_name) {
  update_temperature();
  if (temperature_name == 'E') // environment_temperature
    pid_update_reference(lcd.environment_temperature);
  else
    pid_update_reference(lcd.reference_temperature);

  control_signal = pid_control(lcd.internal_temperature);

  if (control_signal < 0) {
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
  printf("Desligando airfryer...\n");
  start_cooling();
  stop_gpio();
  send_timer(0);
  send_system_state(0);
  clear_display(&lcd);
  int ret = 1;
  while (ret != 0)
    ret = raise(SIGUSR1);
}

static void default_values() {
  printf("Reiniciando valores iniciais.\n");
  lcd.reference_temperature = 0;
  lcd.internal_temperature = 0;
  control_signal = 0;
  milisecond_counter = 0;
  lcd.timer = TIME_MIN;
  send_timer(lcd.timer);
  printf("Atualizando timer: %d\n", lcd.timer);
  menu_option = 0;
  heating = false;
}

static void create_csv() {
  if (access(CSV_FILE_NAME, F_OK) == 0)
    return; // file exists
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
  fprintf(file, ",%.1lf,%.1lf,%.1lf\n", lcd.internal_temperature, lcd.reference_temperature, control_signal);

  fclose(file);
}

static void update_temperature() {
  double reference_temperature = get_reference_temperature();
  if (reference_temperature != -1)
    lcd.reference_temperature = reference_temperature;
  double internal_temperature = get_internal_temperature();
  if (internal_temperature != -1)
    lcd.internal_temperature = internal_temperature;

  lcd.environment_temperature = get_environment_temperature();
  printf("environment_temperature = %lf\n", lcd.environment_temperature);
}
