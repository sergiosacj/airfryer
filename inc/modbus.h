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
double get_internal_temperature();
float get_reference_temperature();
int get_user_commands();
void send_control_signal(int signal);
void send_system_state(char state);
void send_working_state(char state);
void send_timer(int timer);

#endif
