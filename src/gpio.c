#include <gpio.h>
#include <wiringPi.h>
#include <softPwm.h>

#include <gpio.h>
#include <definitions.h>

void setup_gpio() {
  wiringPiSetup();
  pinMode(GPIO_RESISTOR, OUTPUT);
  pinMode(GPIO_FAN, OUTPUT);
  softPwmCreate(GPIO_RESISTOR, 0, GPIO_PWM_RANGE);
  softPwmCreate(GPIO_FAN, FAN_MIN_VALUE, GPIO_PWM_RANGE);
}

void update_resistor(int value) {
  softPwmWrite(GPIO_RESISTOR, value);
}

void update_fan(int value) {
  if (value < FAN_MIN_VALUE)
    value = FAN_MIN_VALUE;
  softPwmWrite(GPIO_FAN, value);
}
