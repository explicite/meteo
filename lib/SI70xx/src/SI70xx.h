#ifndef si70xx_h
#define si70xx_h

#include <Wire.h>
#include <Arduino.h>

class SI70xx {
  public:
     SI70xx(u8 ADDR);
     int init(u8 SDA, u8 SCL);
     int getDeviceId(u8 *id);
     int getHumidity();
     int getTemperature();
  private:
    bool _exists;
    u8 _ADDR;
    u8 _SDA;
    u8 _SCL;

    int _writeReg(u8 * reg, u8 reglen);
    int _readReg(u8 * reg, u8 reglen);
    int _command(u8 cmd, u8 * buf);
    int _measure(u8 cmd);
};

#endif
