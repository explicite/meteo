#include "SI70xx.hpp"

/* Commands */
#define CMD_MEASURE_HUMIDITY_HOLD       0xE5
#define CMD_MEASURE_HUMIDITY_NO_HOLD    0xF5
#define CMD_MEASURE_TEMPERATURE_HOLD    0xE3
#define CMD_MEASURE_TEMPERATURE_NO_HOLD 0xF3
#define CMD_MEASURE_THERMISTOR_HOLD     0xEE
#define CMD_READ_PREVIOUS_TEMPERATURE   0xE0
#define CMD_RESET                       0xFE
#define CMD_WRITE_REGISTER_1            0xE6
#define CMD_READ_REGISTER_1             0xE7
#define CMD_WRITE_REGISTER_2            0x50
#define CMD_READ_REGISTER_2             0x10
#define CMD_WRITE_REGISTER_3            0x51
#define CMD_READ_REGISTER_3             0x11
#define CMD_WRITE_COEFFICIENT           0xC5
#define CMD_READ_COEFFICIENT            0x84

/* User Register 1 */
#define REG1_RESOLUTION_MASK    0x81
#define REG1_RESOLUTION_H12_T14 0x00
#define REG1_RESOLUTION_H08_T12 0x01
#define REG1_RESOLUTION_H10_T13 0x80
#define REG1_RESOLUTION_H11_T11 0x81
#define REG1_LOW_VOLTAGE        0x40
#define REG1_ENABLE_HEATER      0x04

/* User Register 2 */
#define REG2_VOUT             0x01
#define REG2_VREF_VDD         0x02
#define REG2_VIN_BUFFERED     0x04
#define REG2_RESERVED         0x08
#define REG2_FAST_CONVERSION  0x10
#define REG2_MODE_CORRECTION  0x20
#define REG2_MODE_NO_HOLD     0x40

/* Device Identification */
#define ID_SAMPLE 0xFF
#define ID_SI7006 0x06
#define ID_SI7013 0x0D
#define ID_SI7020 0x14
#define ID_SI7021 0x15

/* Coefficients */
#define COEFFICIENT_BASE  0x82
#define COEFFICIENT_COUNT 9

/* Thermistor Correction Coefficients */
static struct {
   s16 input[COEFFICIENT_COUNT];
   u16 output[COEFFICIENT_COUNT];
   s16 slope[COEFFICIENT_COUNT];
} si70xx_coefficient_table = {
   /* input  */ {19535, 15154, 11187,  7982,  5592,  3895,  2721,  1916,  1367},
   /* output */ {11879, 15608, 19338, 23067, 26797, 30527, 34256, 37986, 41715},
   /* slope  */ { -218,  -241,	-298,  -400,  -563,  -813, -1186, -1739, -2513}
};

SI70xx::SI70xx(u8 ADDR) {
  _ADDR = ADDR;
};

int SI70xx::init(u8 SDA, u8 SCL) {
  Wire.begin(SDA, SCL);
  Wire.beginTransmission(_ADDR);
  if (Wire.endTransmission() == 0) {
    _exists = true;
    _SDA = SDA;
    _SCL = SCL;
    return 0;
  } else {
    return -1;
  }
};

int SI70xx::getDeviceId(u8* id) {
	u8 buf[6];
	int  error;

	/* Put the 2-byte command into the buffer */
	buf[0] = 0xFC;
	buf[1] = 0xC9;

	/* Send the command */
	error = _writeReg(buf, 2);
	if (error < 0) return error;

	/* Receive the 6-byte result */
	error = _readReg(buf, 6);
	if (error < 0) return error;

	/* Return the device ID */
	*id = buf[0];

	return 0;  /* Success */
}

int SI70xx::getHumidity() {
  int value;
  int humidity;

  /* Measure the humidity value */
  value = _measure(CMD_MEASURE_HUMIDITY_HOLD);
  if (value < 0) return value;

  /* Convert the value to milli-percent (pcm) relative humidity */
	value = ((value * 15625) >> 13) - 6000;

  /* Limit the humidity to valid values */
	if (value < 0) {
		humidity = 0;
	} else if (value > 100000) {
		humidity = 100000;
	} else {
		humidity = value;
  }

  return humidity;
}

int SI70xx::getTemperature() {
    int value;
    int temperature;

    /* Measure the temperature value */
    value = _measure(CMD_MEASURE_TEMPERATURE_HOLD);
    if (value < 0) return value;

    /* Convert the value to millidegrees Celsius */
    temperature = ((value * 21965) >> 13) - 46850;

    return temperature;
}

int SI70xx::_measure(u8 cmd) {
  int error;
	u8  data[4];

	/* Measure the humidity or temperature value */
	error = _command(cmd, data);
	if (error < 0) return error;

	/* Swap the bytes and clear the status bits */
	return ((data[0] * 256) + data[1]) & ~3;
}

int SI70xx::_writeReg(u8* reg, u8 reglen) {
    Wire.beginTransmission(_ADDR);
    for(int i = 0; i < reglen; i++) {
        reg += i;
        Wire.write(*reg);
    }
    return Wire.endTransmission();
}

int SI70xx::_readReg(u8* reg, u8 reglen) {
    Wire.requestFrom(_ADDR, reglen);
    while(Wire.available() < reglen) {
    }
    for(int i = 0; i < reglen; i++) {
        reg[i] = Wire.read();
    }
    return Wire.endTransmission();
}

int SI70xx::_command(u8 cmd, u8* buf) {
    int error;
    error = _writeReg(&cmd, sizeof cmd);
    if(error < 0) return error;
    delay(25);
    return _readReg(buf, 0x02);
}
