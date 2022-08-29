#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <definitions.h>
#include <modbus.h>

static char unb_registration[] = {7, 4, 3, 9};

Message get_integer(Message msg) {
  int uart_filestream = open("/dev/serial0", O_WRONLY);
  if (uart_filestream == -1) {
    printf("Erro - Não foi possível iniciar a UART para ler inteiro.\n");
    return msg;
  }
  printf("Info - UART inicializada para ler inteiro.\n");

  struct termios options;
  tcgetattr(uart_filestream, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; // baud rate
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart_filestream, TCIFLUSH);
  tcsetattr(uart_filestream, TCSANOW, &options);

  unsigned char tx_buffer[7];
  tx_buffer[0] = msg.address;
  tx_buffer[1] = msg.code;
  tx_buffer[2] = msg.subcode;
  memcpy(&tx_buffer[3], unb_registration, strlen(unb_registration) + 1);
  printf("buffer = %d", tx_buffer[0]);
  for (int i = 1; i < 7; i++) {
    printf(", %d\n", tx_buffer[i]);
  }

  int res = write(uart_filestream, tx_buffer, sizeof(tx_buffer));
  if (res < 0) {
    printf("UART TX error\n");
  }

  sleep(1);

  unsigned char rx_buffer[8];
  int rx_length = read(uart_filestream, (void*)rx_buffer, 7);
  if (rx_length < 0) {
    printf("Erro na leitura do inteiro.\n");
  } else if (rx_length == 0) {
    printf("Nenhum byte foi encontrado.\n");
  } else {
    rx_buffer[rx_length] = '\0';
    printf("%i Bytes lidos : %s\n", rx_length, rx_buffer);
  }

  close(uart_filestream);
  return msg;
}
