#ifndef LCD_H_
#define LCD_H_

#define I2C_ADDR      0x27 // I2C device address
#define LCD_CHR       1    // Mode - Sending data
#define LCD_CMD       0    // Mode - Sending command
#define LINE1         0x80 // 1st line
#define LINE2         0xC0 // 2nd line
#define LCD_BACKLIGHT 0x08 // On
#define ENABLE  0b00000100 // Enable bit

typedef struct LCD
{
  int fd;
  double internal_temperature;
  double reference_temperature;
  int timer;
  char menu_option;
} DisplayLCD;

DisplayLCD start_display();
void draw(DisplayLCD *self);
void clear_display(DisplayLCD *self);
void draw_heating_cooling(DisplayLCD *self, char option);

#endif
