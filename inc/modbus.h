#ifndef MODBUS_H_
#define MODBUS_H_

typedef struct Message
{
  int uart_filestream;
  char address;
  char code;
  char subcode;
  char *message;
} Message;

Message new_message(char code, char subcode);

#endif
