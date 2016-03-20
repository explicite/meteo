#ifndef lps331ap_h
#define lps331ap_h

#include <Wire.h>
#include <Arduino.h>

class LPS331AP {
  public:
    LPS331AP(u8 ADDR);
    int init(u8 SDA, u8 SCL);
  private:
    bool _exists;
    u8 _ADDR;
    u8 _SDA;
    u8 _SCL;

    u32 _delayMs;
    bool _enabled;
    bool _needResume;

    int _writeReg(u8* reg, u8 reglen);
    int _readReg(u8* reg, u8 reglen);
    int _command(u8 cmd, u8 reglen, u8* buf);
    int _mesure(s32* data);
};

#endif
