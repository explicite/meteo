#include "BH17xx.hpp"

#define BH1750_POWER_DOWN            0x00
#define BH1750_ONE_TIME_H_RES_MODE   0x20 /* auto-mode for BH1721 */
#define BH1750_CHANGE_INT_TIME_H_BIT 0x40
#define BH1750_CHANGE_INT_TIME_L_BIT 0x60

BH17xx::BH17xx(u8 ADDR, Type type) {
  _ADDR = ADDR;
  switch (type) {
    case BH1710:
      _mtregMin = 140;
      _mtregMax = 1022;
      _mtregDefault = 300;
      _mtregToUsec = 400;
      _mtregToScale = 250000000;
      _inc = 2;
      _intTimeLowMask = 0x001F;
      _intTimeHighMask = 0x03E0;
      break;
    case BH1721:
      _mtregMin = 140;
      _mtregMax = 1020;
      _mtregDefault = 300;
      _mtregToUsec = 400;
      _mtregToScale = 250000000;
      _inc = 2;
      _intTimeLowMask = 0x0010;
      _intTimeHighMask = 0x03E0;
      break;
    case BH1750:
      _mtregMin = 31;
      _mtregMax = 254;
      _mtregDefault = 69;
      _mtregToUsec = 1740;
      _mtregToScale = 57500000;
      _inc = 1;
      _intTimeLowMask = 0x001F;
      _intTimeHighMask = 0x00E0;
      break;
  }
}

int BH17xx::init(u8 SDA, u8 SCL) {
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
}

int BH17xx::changeIntTime(uint usec) {
  int error;
  u16 val;
  u8 regval;
  u8* cmd;

  if ((usec % _mtregToUsec) != 0) return -1;
  val = usec / _mtregToUsec;

  cmd = BH1750_POWER_DOWN;
  error = _writeReg(cmd, sizeof cmd);
  if (error < 0) return error;

  regval = (val & _intTimeHighMask) >> 5;
  cmd = (u8*)(BH1750_CHANGE_INT_TIME_H_BIT | regval);
  error = _writeReg(cmd, 0x02);;

  if (error < 0) return error;

  regval = val & _intTimeLowMask;
  cmd = (u8*)(BH1750_CHANGE_INT_TIME_L_BIT | regval);
  error = _writeReg(cmd, 0x02);

  if (error < 0) return error;

  _mtreg = val;

  return 0;
}

int BH17xx::getIlluminance() {
  int error;
  u8  data[2];
  int value;

  long delay = (_mtregToUsec * _mtreg) + 40000;
  error = _command(BH1750_ONE_TIME_H_RES_MODE, data);
  if (error < 0) return error;

  delayMicroseconds(delay);

  error = _readReg(data, 0x02);
  if (error < 0) return error;

  value = ((data[0] << 8) | data[1]) / 1.2;

  return value;
}

int BH17xx::_writeReg(u8* reg, u8 reglen) {
  Wire.beginTransmission(_ADDR);
  for(int i = 0; i < reglen; i++) {
      reg += i;
      Wire.write(*reg);
  }
  return Wire.endTransmission();
}

int BH17xx::_readReg(u8* reg, u8 reglen) {
  Wire.requestFrom(_ADDR, reglen);
  while(Wire.available() < reglen) {
  }
  for(int i = 0; i < reglen; i++) {
      reg[i] = Wire.read();
  }
  return Wire.endTransmission();
}

int BH17xx::_command(u8 cmd, u8* buf) {
  int error;
  error = _writeReg(&cmd, sizeof cmd);
  if(error < 0) return error;
  delay(25);
  return _readReg(buf, 0x02);
}
