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

int LPS331AP::_command(u8 cmd, u8 reglen, u8* buf) {
  int error;
  error = _writeReg(&cmd, sizeof cmd);
  if(error < 0) return error;
  delay(25);
  return _readReg(buf, reglen);
}

int LPS331AP::_mesure(s32* data) {
  int error;
  u8 buf[3] = { 0 };

  error = _command(PRESS_OUT_XL | I2C_AUTO_INC, 0x03, buf);
  if (error < 0) return error;

  data[0] = (s8)buf[2] << 16 | buf[1] << 8 | buf[0];

  error = _command(TEMP_OUT_L | I2C_AUTO_INC, 0x02, buf);
  if (error < 0) return error;

  data[1] = (s8)buf[1] << 8 | buf[0];

  return 0;
}
