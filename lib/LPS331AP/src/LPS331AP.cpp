#include "LPS331AP.hpp"

#define REF_P_XL 0x08
#define REF_P_L	 0x09
#define REF_P_H	 0x0A
#define REF_T_L	 0x0B
#define REF_T_H	 0x0C
#define WHO_AM_I 0x0F

#define RES_CONF 0x10

#define CTRL_REG1  0x20
#define POWER_MODE  7
#define POWER_DOWN (0 << POWER_MODE)
#define POWER_ON	 (1 << POWER_MODE)
#define ODR_SELECT	4
#define ODR0		   (0 << ODR_SELECT)
#define ODR1		   (1 << ODR_SELECT)
#define ODR2		   (2 << ODR_SELECT)
#define ODR3		   (3 << ODR_SELECT)
#define ODR4		   (4 << ODR_SELECT)
#define ODR5		   (5 << ODR_SELECT)
#define ODR6		   (6 << ODR_SELECT)
#define ODR7		   (7 << ODR_SELECT)
#define DIFF_EN              3
#define INT_CIRCUIT_ENABLE	(1 << DIFF_EN)
#define INT_CIRCUIT_DISABLE	(0 << DIFF_EN)
#define BDU                2
#define CONT_DATA_UPDATE  (0 << BDU)
#define BLOCK_DATA_UPDATE	(1 << BDU)
#define DELTA_EN           1
#define DELTA_P_ENABLE		(1 << DELTA_EN)
#define DELTA_P_DISABLE		(0 << DELTA_EN)
#define SIM_SELECT	     0
#define WIRE_INTER_4		(0 << SIM_SELECT)
#define WIRE_INTER_3		(1 << SIM_SELECT)

#define CTRL_REG2	0x21
#define BOOT		  0x80
#define SWRESET		0x04
#define AUTO_ZERO	0x02
#define ONE_SHOT	0x01

#define CTRL_REG3	   0x22
#define INT_H_L		    7
#define INT_ACTIVE_H (0 << INT_H_L)
#define INT_ACTIVE_L (1 << INT_H_L)
#define PP_OD		      6
#define PUSH_PULL		 (0 << PP_OD)
#define OPEN_DRAIN	 (1 << PP_OD)
#define INT2_CTRL	    3
#define INT2_P_HIGH	 (1 << INT2_CTRL)
#define INT2_P_LOW	 (2 << INT2_CTRL)
#define INT2_P_H_L	 (3 << INT2_CTRL)
#define INT2_DATARDY (4 << INT2_CTRL)
#define INT1_CTRL	    0
#define INT1_P_HIGH	 (1 << INT1_CTRL)
#define INT1_P_LOW	 (2 << INT1_CTRL)
#define INT1_P_H_L	 (3 << INT1_CTRL)
#define INT1_DATARDY (4 << INT1_CTRL)


#define INT_CFG_REG	 0x23
#define INT_CFG_LIR	 0x04
#define INT_CFG_PL_E 0x02
#define INT_CFG_PH_E 0x01

#define INT_SRC_REG	0x24
#define INT_SRC_IA	0x04
#define INT_SRC_PL	0x02
#define INT_SRC_PH	0x01

#define THS_P_LOW_REG  0x25
#define THS_P_HIGH_REG 0x26

#define STATUS_REG	0x27
#define STATUS_P_DA	0x02
#define STATUS_T_DA	0x01
#define STATUS_P_OR	0x20
#define STATUS_T_OR	0x10

#define I2C_AUTO_INC 0x80
#define PRESS_OUT_XL 0x28
#define PRESS_OUT_L	 0x29
#define PRESS_OUT_H	 0x2A

#define TEMP_OUT_L 0x2B
#define TEMP_OUT_H 0x2C

#define AMP_CTRL 0x30

#define LPS331AP_DEFAULT_DELAY 200
#define LPS331AP_MIN_DELAY     75

#define LPS331AP_DEVICE_NAME "lps331ap"
#define LPS331AP_DEVICE_ID	 0xbb

LPS331AP::LPS331AP(u8 ADDR) {
  _ADDR = ADDR;
}

int LPS331AP::init(u8 SDA, u8 SCL) {
    Wire.begin(SDA, SCL);
    Wire.beginTransmission(_ADDR);
    if (Wire.endTransmission() == 0) {

      _SDA = SDA;
      _SCL = SCL;

      int error;
      u8 value;

      error = _command(WHO_AM_I, 0x01, &value);
      if (error < 0) return error;

      if (LPS331AP_DEVICE_ID != value) return -1;

      error = _writeByte(RES_CONF, 0x34);
      if (error < 0) return error;

      error = _writeByte(CTRL_REG1, POWER_DOWN);
      if (error < 0) return error;

      error = _writeByte(CTRL_REG1, POWER_DOWN | ODR7 | INT_CIRCUIT_DISABLE |
        BLOCK_DATA_UPDATE	| DELTA_P_DISABLE);

      if (error >= 0) {
        _exists = true;
      }
      return error;
    } else {
      return -1;
    }
}

int LPS331AP::enable() {
  int error;
  error = _powerOn();

  if (error < 0) return error;

  _enabled = true;
}

int LPS331AP::disable() {
  int error;
  error = _powerOff();

  if (error < 0) return error;

  _enabled = false;
}

int LPS331AP::isExists() {
  return _exists;
}

int LPS331AP::isEnabled() {
  return _enabled;
}

float LPS331AP::getPressure() {
  int error;
  s32 pressure;
  error = _pressure(&pressure);
  if (error < 0) return error;

  return pressure / 4096.0;
}

float LPS331AP::getTemperature() {
  int error;
  s32 temperature;
  error = _temperature(&temperature);
  if (error < 0) return error;
  return 42.5 + (- ( (65535-temperature) + 1) / 480.0);
}

int LPS331AP::_powerOff() {
  int error;
  u8 value;

  error = _command(CTRL_REG1, 0x01, &value);
  if (error < 0) return error;

  value &= ~POWER_ON;

  error = _writeByte(CTRL_REG1, value);

  return error;
}

int LPS331AP::_powerOn() {
  int error;
  u8 value;

  error = _command(CTRL_REG1, 0x01, &value);
  if (error < 0) return error;

  value |= POWER_ON;

  error = _writeByte(CTRL_REG1, value);

  return error;
}

int LPS331AP::_writeReg(u8* reg, u8 reglen) {
  Wire.beginTransmission(_ADDR);
  for(int i = 0; i < reglen; i++) {
      reg += i;
      Wire.write(*reg);
  }
  return Wire.endTransmission();
}

int LPS331AP::_readReg(u8* reg, u8 reglen) {
  Wire.requestFrom(_ADDR, reglen);
  while(Wire.available() < reglen) {
  }
  for(int i = 0; i < reglen; i++) {
      reg[i] = Wire.read();
  }
  return Wire.endTransmission();
}

int LPS331AP::_writeByte(u8 command, u8 value) {
  Wire.beginTransmission(_ADDR);
  Wire.write(command);
  Wire.write(value);

  return Wire.endTransmission();
}

int LPS331AP::_command(u8 cmd, u8 reglen, u8* reg) {
  int error;
  error = _writeReg(&cmd, sizeof cmd);
  if(error < 0) return error;
  delay(25);
  return _readReg(reg, reglen);
}

int LPS331AP::_pressure(s32* data) {
  int error;
  u8 buf[3] = { 0 };

  error = _command(PRESS_OUT_XL | I2C_AUTO_INC, 0x03, buf);
  if (error < 0) return error;

  *data = buf[2] << 16 | buf[1] << 8 | buf[0];
  return 0;
}

int LPS331AP::_temperature(s32* data) {
  int error;
  u8 buf[2] = { 0 };

  error = _command(TEMP_OUT_L | I2C_AUTO_INC, 0x02, buf);
  if (error < 0) return error;

  *data = buf[1] << 8 | buf[0];
  return 0;
}
