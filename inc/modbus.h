#ifndef MODBUS_H_
#define MODBUS_H_

typedef struct Message
{
  char address;
  char code;
  char subcode;
} Message;

Message get_float(Message msg);
Message get_integer(Message msg);
Message send_float(Message msg);
Message send_integer(Message msg);

#endif
