#ifndef bh17xx_h
#define bh17xx_h

#include <Wire.h>
#include <Arduino.h>

enum Type {
  BH1710,
  BH1721,
  BH1750,
};

class BH17xx {
  public:
    BH17xx(u8 ADDR, Type type);
    int init(u8 SDA, u8 SCL);
    int changeIntTime(uint usec);
    int getIlluminance();
  private:
    bool _exists;
    u8 _ADDR;
    u8 _SDA;
    u8 _SCL;

    u16 _mtreg;
    u16 _mtregMin;
    u16 _mtregMax;
    u16 _mtregDefault;
    int _mtregToUsec;
    int _mtregToScale;
    int _inc;
    u16 _intTimeLowMask;
    u16 _intTimeHighMask;

    int _writeReg(u8* reg, u8 reglen);
    int _readReg(u8* reg, u8 reglen);
    int _command(u8 cmd, u8* buf);
};

#endif
