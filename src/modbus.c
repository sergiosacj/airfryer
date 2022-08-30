#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <crc.h>
#include <definitions.h>
#include <modbus.h>

static char unb_registration[] = {7, 4, 3, 9};
static int size_of_unb_registration = 4;

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
  } else {
    printf("UART started!\n");
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

  short crc = calcula_CRC(buffer, message_size), crc_size = 2;
  memcpy(&buffer[message_size], &crc, crc_size);
  message_size += crc_size;

  int result = write(self->uart_filestream, &buffer, message_size);
  if (result < 0) {
    printf("UART TX error.\n");
  } else {
    printf("UART TX successful.\n");
  }
}

static void message_read(Message *self, int message_size) {
  self->message = malloc(sizeof(char) * message_size);
  int rx_length = read(self->uart_filestream, self->message, message_size);
  if (rx_length < 0) {
    printf("(message_read) UART RX error.\n");
  } else if (rx_length == 0) {
    printf("(message_read) No data available.\n");
  } else {
    self->message[rx_length] = '\0';
    printf("(message_read) %i bytes: %s\n", rx_length, self->message);
  }
}

}
