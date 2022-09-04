#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <bme280.h>
#include <definitions.h>
#include <environment_temperature.h>

static const double DEFAULT_ENVIRONMENT_TEMPERATURE = 25;
static int i2c_fd;

static void user_delay_ms(uint32_t period);
static int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len);
static int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len);

double get_environment_temperature() {
  struct bme280_dev dev;

  /* Variable to define the result */
  int8_t rslt = BME280_OK;

  /* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
  dev.dev_id = BME280_I2C_ADDR_PRIM;
  dev.intf = BME280_I2C_INTF;
  dev.read = user_i2c_read;
  dev.write = user_i2c_write;
  dev.delay_ms = user_delay_ms;

  if ((i2c_fd = open(I2C_FILE, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open the i2c bus %s\n", I2C_FILE);
    return DEFAULT_ENVIRONMENT_TEMPERATURE;
  }

  if (ioctl(i2c_fd, I2C_SLAVE, dev.dev_id) < 0) {
    fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
    return DEFAULT_ENVIRONMENT_TEMPERATURE;
  }

  /* Initialize the bme280 */
  rslt = bme280_init(&dev);
  if (rslt != BME280_OK) {
    fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
    return DEFAULT_ENVIRONMENT_TEMPERATURE;
  }

  /* Variable to define the selecting sensors */
  uint8_t settings_sel = 0;
  /* Variable to store minimum wait time between consecutive measurement in force mode */
  uint32_t req_delay;
  /* Structure to get the pressure, temperature and humidity values */
  struct bme280_data comp_data;

  /* Recommended mode of operation: Indoor navigation */
  dev.settings.osr_h = BME280_OVERSAMPLING_1X;
  dev.settings.osr_p = BME280_OVERSAMPLING_16X;
  dev.settings.osr_t = BME280_OVERSAMPLING_2X;
  dev.settings.filter = BME280_FILTER_COEFF_16;

  settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL |
                 BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

  /* Set the sensor settings */
  rslt = bme280_set_sensor_settings(settings_sel, &dev);
  if (rslt != BME280_OK) {
    fprintf(stderr, "Failed to set sensor settings (code %+d).", rslt);
    return DEFAULT_ENVIRONMENT_TEMPERATURE;
  }

  /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
   *  and the oversampling configuration. */
  req_delay = bme280_cal_meas_delay(&dev.settings);

  /* Set the sensor to forced mode */
  rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
  if (rslt != BME280_OK) {
    fprintf(stderr, "Failed to set sensor mode (code %+d).", rslt);
    return DEFAULT_ENVIRONMENT_TEMPERATURE;
  }

  /* Wait for the measurement to complete and return data */
  dev.delay_ms(req_delay);
  rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
  if (rslt != BME280_OK) {
    fprintf(stderr, "Failed to get sensor data (code %+d).", rslt);
    return DEFAULT_ENVIRONMENT_TEMPERATURE;
  }

  return comp_data.temperature;
}

static void user_delay_ms(uint32_t period) {
  /* Milliseconds convert to microseconds */
  usleep(period * 1000);
}

static int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  write(i2c_fd, &reg_addr, 1);
  read(i2c_fd, data, len);
  return 0;
}

static int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  int8_t *buf;
  buf = malloc(len + 1);
  buf[0] = reg_addr;
  memcpy(buf + 1, data, len);
  if (write(i2c_fd, buf, len + 1) < len)
    return BME280_E_COMM_FAIL;
  free(buf);
  return BME280_OK;
}
