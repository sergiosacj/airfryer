#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <crc.h>
#include <definitions.h>
#include <modbus.h>

static void message_open_uart(Message *self);
static void message_close_uart(Message *self);
static void message_request(Message *self, char *data, int data_size);
static int message_read(Message *self, int message_size);
static short validate_message(Message *self);

static char unb_registration[] = {7, 4, 3, 9};
static int size_of_unb_registration = 4;
static int size_of_message_response = 7; // address + code + subcode + data (4 bytes)

Message new_message(char code, char subcode) {
  Message msg = {
    .uart_filestream = -1,
    .address = SERVER_ADDRESS,
    .code = code,
    .subcode = subcode
  };
  return msg;
}

static void message_open_uart(Message *self) {
  self->uart_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (self->uart_filestream == -1) {
    printf("Failed to open /dev/serial0.\n");
  }

  struct termios options;
  tcgetattr(self->uart_filestream, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; // baud rate
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(self->uart_filestream, TCIFLUSH);
  tcsetattr(self->uart_filestream, TCSANOW, &options);
}

static void message_close_uart(Message *self) {
  free(self->message);
  close(self->uart_filestream);
}

static void message_request(Message *self, char *data, int data_size) {
  unsigned char buffer[100];
  int message_size = 3;
  buffer[0] = self->address;
  buffer[1] = self->code;
  buffer[2] = self->subcode;

  memcpy(&buffer[message_size], data, data_size);
  message_size += data_size;

  short crc = calculate_CRC(buffer, message_size), crc_size = 2;
  memcpy(&buffer[message_size], &crc, crc_size);
  message_size += crc_size;

  int result = write(self->uart_filestream, &buffer, message_size);
  if (result < 0) {
    printf("UART TX error.\n");
  }
}

static int message_read(Message *self, int message_size) {
  self->message = malloc(sizeof(char) * message_size);
  int rx_length = read(self->uart_filestream, self->message, message_size);
  if (rx_length < 0)
    printf("Descartando %d bytes lidos incorretamente.\n", message_size);
  else if (rx_length == 0)
    printf("Nada para ler.\n");
  else
    self->message[rx_length] = '\0';

  return validate_message(self);
}

static short validate_message(Message *self) {
  if (self->code != self->message[1] || self->subcode != self->message[2]) {
    printf("Mensagem inv??lida\n");
    return 1;
  }
  printf("Mensagem v??lida\n");
  return 0;
}

#define define_get_message(T) \
T get_message_##T(char subcode) { \
  Message msg = new_message(CODE_REQUEST, subcode); \
  message_open_uart(&msg); \
  message_request(&msg, unb_registration, size_of_unb_registration); \
  usleep(200000); \
  short message_valid = message_read(&msg, size_of_message_response); \
  if (message_valid > 0) \
    return -1; \
  T message_response; \
  memcpy(&message_response, &(msg.message[3]), sizeof(T)); \
  message_close_uart(&msg); \
  return message_response; \
}

define_get_message(int)
define_get_message(float)

#define get_message(T) get_message_##T

double get_internal_temperature() {
  return (double) get_message(float)(SUB_CODE_REQUEST_INTERNAL_TEMPERATURE);
}

double get_reference_temperature() {
  return (double) get_message(float)(SUB_CODE_REQUEST_REF_TEMPERATURE);
}

int get_user_commands() {
  return get_message(int)(SUB_CODE_REQUEST_USER_COMMANDS);
}

#define define_send_message(T) \
void send_message_##T(char subcode, T data) { \
  int data_size = sizeof(T); \
  Message msg = new_message(CODE_SEND, subcode); \
  message_open_uart(&msg); \
  char message[data_size + size_of_unb_registration]; \
  memcpy(message, unb_registration, size_of_unb_registration); \
  memcpy(&message[size_of_unb_registration], &data, data_size); \
  message_request(&msg, message, data_size + size_of_unb_registration); \
  usleep(200000); \
  message_close_uart(&msg); \
}

define_send_message(int)
define_send_message(char)
define_send_message(float)

#define send_message(T) send_message_##T

void send_control_signal(int signal) {
  send_message(int)(SUB_CODE_SEND_CONTROL_SIGNAL, signal);
}

void send_system_state(char state) {
  send_message(char)(SUB_CODE_SEND_ON_OFF, state);
}

void send_working_state(char state) {
  send_message(char)(SUB_CODE_SEND_WORKING_STATE, state);
}

void send_timer(int timer) {
  send_message(int)(SUB_CODE_SEND_TIMER, timer);
}
