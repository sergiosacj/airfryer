#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <definitions.h>
#include <lcd.h>

static void lcd_init(DisplayLCD *self);
static void go_to_line(DisplayLCD *self, int line);
static void display_double(DisplayLCD *self, double f);
static void display_int(DisplayLCD *self, int i);
static void display_char(DisplayLCD *self, char c);
static void display_string(DisplayLCD *self, const char *s);
static void lcd_byte(DisplayLCD *self, int bits, int mode);
static void lcd_toggle_enable(DisplayLCD *self, int bits);

DisplayLCD start_display() {
  DisplayLCD lcd = {
    .fd = wiringPiI2CSetup(I2C_ADDR),
    .internal_temperature = 0,
    .reference_temperature = 0,
    .timer = 0,
    .menu_option = 'M',
  };

  if (lcd.fd == -1)
    return lcd;

  lcd_init(&lcd);
  return lcd;
}

void draw(DisplayLCD *self) {
  clear_display(self);

  go_to_line(self, LINE1);
  display_string(self, "Tempo: ");
  display_int(self, self->timer);
  display_string(self, " MO: ");
  display_char(self, self->menu_option);

  go_to_line(self, LINE2);
  display_string(self, "TI:");
  display_double(self, self->internal_temperature);
  display_string(self, " TR:");
  display_double(self, self->reference_temperature);
}

void draw_heating_cooling(DisplayLCD *self, char option) {
  clear_display(self);

  go_to_line(self, LINE1);
  if (option == 'H')
    display_string(self, "Aquecendo...");
  else
    display_string(self, "Resfriando...");
  go_to_line(self, LINE2);
  display_string(self, "TI: ");
  display_double(self, self->internal_temperature);
  display_string(self, " TR: ");
  display_double(self, self->reference_temperature);
}

void clear_display(DisplayLCD *self) {
  lcd_byte(self, 0x01, LCD_CMD);
  lcd_byte(self, 0x02, LCD_CMD);
}

static void lcd_init(DisplayLCD *self) {
  lcd_byte(self, 0x33, LCD_CMD); // Initialise
  lcd_byte(self, 0x32, LCD_CMD); // Initialise
  lcd_byte(self, 0x06, LCD_CMD); // Cursor move direction
  lcd_byte(self, 0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(self, 0x28, LCD_CMD); // Data length, number of lines, font size
  clear_display(self);
  delayMicroseconds(500);
}

static void go_to_line(DisplayLCD *self, int line) {
  lcd_byte(self, line, LCD_CMD);
}

static void display_double(DisplayLCD *self, double lf) {
  char buffer[20];
  sprintf(buffer, "%.1lf", lf);
  display_string(self, buffer);
}

static void display_int(DisplayLCD *self, int i) {
  char buffer[20];
  sprintf(buffer, "%02d", i);
  display_string(self, buffer);
}

static void display_char(DisplayLCD *self, char c) {
  lcd_byte(self, c, LCD_CHR);
}

static void display_string(DisplayLCD *self, const char *s) {
  while(*s) lcd_byte(self, *(s++), LCD_CHR);
}

static void lcd_byte(DisplayLCD *self, int bits, int mode) {
  int bits_high;
  int bits_low;

  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  wiringPiI2CReadReg8(self->fd, bits_high);
  lcd_toggle_enable(self, bits_high);

  wiringPiI2CReadReg8(self->fd, bits_low);
  lcd_toggle_enable(self, bits_low);
}

static void lcd_toggle_enable(DisplayLCD *self, int bits) {
  delayMicroseconds(500);
  wiringPiI2CReadReg8(self->fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(self->fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}
