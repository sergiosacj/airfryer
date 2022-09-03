#ifndef GPIO_H_
#define GPIO_H_

#define FAN_MIN_VALUE 40

void setup_gpio();
void update_resistor(float value);
void update_fan(float value);

#endif
