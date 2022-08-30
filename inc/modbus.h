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
float get_internal_temperature();
float get_reference_temperature();
int get_user_commands();
void send_control_signal(int signal);
void send_ref_signal(float signal);
void send_system_state(short state);
void send_working_state(short state);
void send_timer(int timer);

#endif
