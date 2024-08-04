#ifndef ADAFRUIT_DS248X_H
#define ADAFRUIT_DS248X_H

#include <Adafruit_I2CDevice.h>
#include <Arduino.h>

#define DS248X_ADDRESS 0x18

// DS248x Command Definitions
#define DS248X_CMD_RESET 0xF0
#define DS248X_CMD_SET_READ_PTR 0xE1
#define DS248X_CMD_WRITE_CONFIG 0xD2
#define DS248X_CMD_1WIRE_RESET 0xB4
#define DS248X_CMD_1WIRE_SINGLE_BIT 0x87
#define DS248X_CMD_1WIRE_WRITE_BYTE 0xA5
#define DS248X_CMD_1WIRE_READ_BYTE 0x96
#define DS248X_CMD_1WIRE_TRIPLET 0x78
#define DS248X_CMD_CHANNEL_SELECT 0xC3

// DS248x Register Definitions
#define DS248X_REG_STATUS 0xF0
#define DS248X_REG_READ_DATA 0xE1
#define DS248X_REG_CONFIG 0xC3

/**! Class to hold interface for DS248x chip */

class Adafruit_DS248x {
public:
  Adafruit_DS248x();
  bool begin(TwoWire *theWire = &Wire, uint8_t address = DS248X_ADDRESS);
  bool reset();
  bool selectChannel(uint8_t chan);

  bool OneWireReset();
  bool OneWireReadByte(uint8_t *byte);
  bool OneWireWriteByte(uint8_t byte);
  bool OneWireReadBit(uint8_t *bit);
  bool OneWireWriteBit(bool bit);
  bool OneWireSearchReset();
  bool OneWireSearch(uint8_t *newAddr);

  // status bit getters
  bool is1WBusy();
  bool presencePulseDetected();
  bool shortDetected();
  bool logicLevel();
  bool singleBitResult();
  bool tripletSecondBit();
  bool branchDirTaken();

  // configuration bit setters
  bool activePullup(bool enable);
  bool powerDown(bool enable);
  bool strongPullup(bool enable);
  bool overdriveSpeed(bool enable);

  bool busyWait(uint16_t timeout_ms);

private:
  uint8_t ROM_NO[8];
  uint8_t LastDiscrepancy;
  uint8_t LastFamilyDiscrepancy;
  bool LastDeviceFlag;

  uint8_t _address;
  Adafruit_I2CDevice *i2c_dev = nullptr;

  uint8_t readConfig();
  bool writeConfig(uint8_t config);

  uint8_t readStatus();

  bool setReadPointer(uint8_t reg);
};

#endif // ADAFRUIT_DS248X_H
