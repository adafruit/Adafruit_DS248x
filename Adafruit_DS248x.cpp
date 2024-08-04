/*!
 *  @file Adafruit_DS248x.cpp
 *
 *  @mainpage Adafruit DS248x Library
 *
 *  @section intro_sec Introduction
 *
 *  This is a library for the DS248x 1-Wire master with I2C interface.
 *
 *  @section license License
 *
 *  MIT license, all text here must be included in any redistribution.
 *
 *  Copyright (c) 2024 Limor Fried (Adafruit Industries)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "Adafruit_DS248x.h"

/*!
 *    @brief  Instantiates a new DS248x class
 */
Adafruit_DS248x::Adafruit_DS248x() {}

/*!
 *    @brief  Initializes I2C and the device
 *    @param  theWire
 *            Pointer to the desired TwoWire I2C object
 *    @param  address
 *            I2C address of the device
 *    @return True if initialization was successful, otherwise false
 */
bool Adafruit_DS248x::begin(TwoWire *theWire, uint8_t address) {
  if (i2c_dev != nullptr) {
    delete i2c_dev;
  }
  _address = address;
  i2c_dev = new Adafruit_I2CDevice(_address, theWire);
  if (!i2c_dev->begin()) {
    return false;
  }

  return reset();
}

/*!
 *    @brief  Resets the DS248x device
 *    @return True if reset was successful, otherwise false
 */
bool Adafruit_DS248x::reset() {
  uint8_t cmd = DS248X_CMD_RESET;
  if (!i2c_dev->write(&cmd, 1)) {
    return false;
  }

  uint8_t status = readStatus();
  return (status != 0xFF) &&
         (status & 0x10); // Check if the RST bit (bit 4) is set
}

/*!
 *    @brief  Selects which of 8 channels on DS2482-800 chipset
 *    @param  chan Channel to use, from #0 to #7 inclusive
 *    @return True on command success
 */
bool Adafruit_DS248x::selectChannel(uint8_t chan) {
  if (chan > 7)
    return false;
  uint8_t channelcode = chan + (~chan << 4);

  uint8_t cmd[2] = {DS248X_CMD_CHANNEL_SELECT, channelcode};
  uint8_t reply;

  if (!i2c_dev->write_then_read(cmd, 2, &reply, 1)) {
    return false;
  }

  uint8_t returncodes[] = {0xB8, 0xB1, 0xAA, 0xA3, 0x9C, 0x95, 0x8E, 0x87};
  return (returncodes[chan] == reply);
}

/*!
 *    @brief  Sends a 1-Wire reset command and verifies the reset
 *    @return True if the reset was successful, otherwise false
 */
bool Adafruit_DS248x::OneWireReset() {
  // Wait for the bus to be free
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Send the 1-Wire reset command
  uint8_t cmd = DS248X_CMD_1WIRE_RESET;
  if (!i2c_dev->write(&cmd, 1)) {
    return false; // Return false if writing the command fails
  }

  // Read the status register to verify the reset
  uint8_t status = readStatus();
  return (status != 0xFF) && !shortDetected() && presencePulseDetected();
}

/*!
 *    @brief  Writes a byte to the 1-Wire bus
 *    @param  byte
 *            The byte to write
 *    @return True if writing was successful, otherwise false
 */
bool Adafruit_DS248x::OneWireWriteByte(uint8_t byte) {
  // Wait for the bus to be free
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Send the 1-Wire write byte command followed by the byte to write
  uint8_t cmd[2] = {DS248X_CMD_1WIRE_WRITE_BYTE, byte};
  if (!i2c_dev->write(cmd, 2)) {
    return false; // Return false if writing the command fails
  }

  // Wait for the write operation to complete
  return busyWait(1000); // Return false if the bus is busy after the timeout
}

/*!
 *    @brief  Reads a byte from the 1-Wire bus
 *    @param  byte
 *            Pointer to the byte to store the read value
 *    @return True if reading was successful, otherwise false
 */
bool Adafruit_DS248x::OneWireReadByte(uint8_t *byte) {
  // Wait for the bus to be free
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Send the 1-Wire read byte command
  uint8_t cmd = DS248X_CMD_1WIRE_READ_BYTE;
  if (!i2c_dev->write(&cmd, 1)) {
    return false; // Return false if writing the command fails
  }

  // Wait for the read operation to complete
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Read the byte from the read data register
  if (!setReadPointer(DS248X_REG_READ_DATA)) {
    return false; // Return false if setting the read pointer fails
  }

  if (!i2c_dev->read(byte, 1)) {
    return false; // Return false if reading the byte fails
  }

  return true;
}

/*!
 *    @brief  Reads a single bit from the 1-Wire bus
 *    @param  bit
 *            Pointer to the boolean to store the read value
 *    @return True if reading was successful, otherwise false
 */
bool Adafruit_DS248x::OneWireReadBit(uint8_t *bit) {
  // Wait for the bus to be free
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  if (!OneWireWriteBit(1)) {
    return false; // Return false if writing the command fails
  }

  // Wait for the read operation to complete
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Read the single bit result
  uint8_t status = readStatus();
  if (status == 0xFF) {
    return false; // Return false if reading the status fails
  }

  *bit = singleBitResult();
  return true;
}

/*!
 *    @brief  Writes a single bit to the 1-Wire bus
 *    @param  bit
 *            The boolean value to write
 *    @return True if writing was successful, otherwise false
 */
bool Adafruit_DS248x::OneWireWriteBit(bool bit) {
  // Wait for the bus to be free
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Send the 1-Wire single bit write command followed by the bit to write
  uint8_t cmd[2] = {DS248X_CMD_1WIRE_SINGLE_BIT,
                    bit ? (uint8_t)0x80 : (uint8_t)0x00};

  return i2c_dev->write(cmd, 2);
}

/*!
 *    @brief  Waits for the 1-Wire bus to be not busy, up to the specified
 * timeout
 *    @param  timeout_ms
 *            Maximum time to wait in milliseconds
 *    @return True if the bus is not busy before the timeout, otherwise false
 */
bool Adafruit_DS248x::busyWait(uint16_t timeout_ms) {
  uint32_t start = millis();
  while ((millis() - start) < timeout_ms) {
    if (!is1WBusy()) {
      return true;
    }
    delay(1); // Short delay to prevent busy waiting
  }
  return false;
}

/*!
 *    @brief  Configures the active pullup (APU) bit in the configuration
 * register
 *    @param  enable
 *            True to enable the active pullup, false to disable
 *    @return True if configuration was successful, otherwise false
 */
bool Adafruit_DS248x::activePullup(bool enable) {
  uint8_t config = readConfig();
  if (config == 0xFF) {
    return false; // Return false if reading the config failed
  }

  if (enable) {
    config |= 0x01; // Set the APU bit
  } else {
    config &= ~0x01; // Clear the APU bit
  }

  return writeConfig(config);
}

/*!
 *    @brief  Configures the power down (PDN) bit in the configuration register
 *    @param  enable
 *            True to enable power down, false to disable
 *    @return True if configuration was successful, otherwise false
 */
bool Adafruit_DS248x::powerDown(bool enable) {
  uint8_t config = readConfig();
  if (config == 0xFF) {
    return false; // Return false if reading the config failed
  }

  if (enable) {
    config |= 0x02; // Set the PDN bit
  } else {
    config &= ~0x02; // Clear the PDN bit
  }

  return writeConfig(config);
}

/*!
 *    @brief  Configures the strong pullup (SPU) bit in the configuration
 * register
 *    @param  enable
 *            True to enable the strong pullup, false to disable
 *    @return True if configuration was successful, otherwise false
 */
bool Adafruit_DS248x::strongPullup(bool enable) {
  uint8_t config = readConfig();
  if (config == 0xFF) {
    return false; // Return false if reading the config failed
  }

  if (enable) {
    config |= 0x04; // Set the SPU bit
  } else {
    config &= ~0x04; // Clear the SPU bit
  }

  return writeConfig(config);
}

/*!
 *    @brief  Configures the overdrive speed (1WS) bit in the configuration
 * register
 *    @param  enable
 *            True to enable the overdrive speed, false to disable
 *    @return True if configuration was successful, otherwise false
 */
bool Adafruit_DS248x::overdriveSpeed(bool enable) {
  uint8_t config = readConfig();
  if (config == 0xFF) {
    return false; // Return false if reading the config failed
  }

  if (enable) {
    config |= 0x08; // Set the 1WS bit
  } else {
    config &= ~0x08; // Clear the 1WS bit
  }

  return writeConfig(config);
}

/*!
 *    @brief  Checks if the 1-Wire bus is busy
 *    @return True if the 1-Wire bus is busy, otherwise false
 */
bool Adafruit_DS248x::is1WBusy() {
  uint8_t status = readStatus();
  return status != 0xFF && (status & 0x01); // Check the 1-Wire busy bit
}

/*!
 *    @brief  Checks if a presence pulse was detected
 *    @return True if a presence pulse was detected, otherwise false
 */
bool Adafruit_DS248x::presencePulseDetected() {
  uint8_t status = readStatus();
  return status != 0xFF &&
         (status & 0x02); // Check the presence pulse detected bit
}

/*!
 *    @brief  Checks if a short was detected
 *    @return True if a short was detected, otherwise false
 */
bool Adafruit_DS248x::shortDetected() {
  uint8_t status = readStatus();
  return status != 0xFF && (status & 0x04); // Check the short detected bit
}

/*!
 *    @brief  Checks the logic level of the 1-Wire bus
 *    @return True if the logic level is high, otherwise false
 */
bool Adafruit_DS248x::logicLevel() {
  uint8_t status = readStatus();
  return status != 0xFF && (status & 0x08); // Check the logic level bit
}

/*!
 *    @brief  Checks the result of a single bit read
 *    @return True if the single bit read is high, otherwise false
 */
bool Adafruit_DS248x::singleBitResult() {
  uint8_t status = readStatus();
  return status != 0xFF && (status & 0x20); // Check the single bit result bit
}

/*!
 *    @brief  Checks the result of the second bit of a triplet
 *    @return True if the second bit of the triplet is high, otherwise false
 */
bool Adafruit_DS248x::tripletSecondBit() {
  uint8_t status = readStatus();
  return status != 0xFF && (status & 0x40); // Check the triplet second bit
}

/*!
 *    @brief  Checks the direction taken by the branch in a triplet
 *    @return True if the branch direction taken is high, otherwise false
 */
bool Adafruit_DS248x::branchDirTaken() {
  uint8_t status = readStatus();
  return status != 0xFF &&
         (status & 0x80); // Check the branch direction taken bit
}

/*!
 *    @brief  Reads the configuration register
 *    @return The configuration byte
 */
uint8_t Adafruit_DS248x::readConfig() {
  if (!setReadPointer(DS248X_REG_CONFIG)) {
    return 0xFF; // Return an invalid value if setting the pointer fails
  }

  uint8_t config;
  if (!i2c_dev->read(&config, 1)) {
    return 0xFF; // Return an invalid value if reading fails
  }
  return config;
}

/*!
 *    @brief  Writes the configuration register
 *    @param  config
 *            The configuration byte to write
 *    @return True if writing was successful, otherwise false
 */
bool Adafruit_DS248x::writeConfig(uint8_t config) {
  // Wait for the bus to be free
  if (!busyWait(1000)) {
    return false; // Return false if the bus is busy after the timeout
  }

  // Upper nibble must be the one's complement of the lower nibble
  uint8_t config_value = (config & 0x0F) | ((~config & 0x0F) << 4);

  uint8_t cmd[2] = {DS248X_CMD_WRITE_CONFIG, config_value};
  return i2c_dev->write(cmd, 2);
}

/*!
 *    @brief  Reads the status register
 *    @return The status byte
 */
uint8_t Adafruit_DS248x::readStatus() {
  if (!setReadPointer(DS248X_REG_STATUS)) {
    return 0xFF; // Return an invalid value if setting the pointer fails
  }

  uint8_t status;
  if (!i2c_dev->read(&status, 1)) {
    return 0xFF; // Return an invalid value if reading fails
  }
  return status;
}

bool Adafruit_DS248x::setReadPointer(uint8_t reg) {
  uint8_t cmd[2] = {DS248X_CMD_SET_READ_PTR, reg};
  return i2c_dev->write(cmd, 2);
}

/*!
 *    @brief  Resets the search state for 1-Wire devices
 *    @return True if the search reset was successful, otherwise false
 */
bool Adafruit_DS248x::OneWireSearchReset() {
  // Reset search state
  LastDiscrepancy = 0;
  LastDeviceFlag = false;
  LastFamilyDiscrepancy = 0;

  return true;
}

/*!
 *    @brief  Searches for the next 1-Wire device
 *    @param  newAddr
 *            Pointer to an array to store the found address
 *    @return True if a new device was found, otherwise false
 */
bool Adafruit_DS248x::OneWireSearch(uint8_t *newAddr) {
  bool search_result = false;
  uint8_t id_bit_number = 1;
  uint8_t last_zero = 0;
  uint8_t rom_byte_number = 0;
  uint8_t rom_byte_mask = 1;
  uint8_t id_bit, cmp_id_bit;
  bool search_direction;

  // If the last call was not the last one
  if (!LastDeviceFlag) {
    // Perform a 1-Wire reset
    if (!OneWireReset()) {
      // Reset the search state
      LastDiscrepancy = 0;
      LastDeviceFlag = false;
      LastFamilyDiscrepancy = 0;
      return false;
    }

    // Issue the search command
    if (!OneWireWriteByte(0xF0)) { // Normal search command
      return false;
    }

    // Loop to do the search
    do {
      // Read a bit and its complement
      if (!OneWireReadBit(&id_bit) || !OneWireReadBit(&cmp_id_bit)) {
        return false;
      }

      // Check for no devices on the 1-Wire
      if (id_bit && cmp_id_bit) {
        break; // No devices participating
      } else {
        // All devices coupled have 0 or 1
        if (id_bit != cmp_id_bit) {
          search_direction = id_bit; // Bit write value for search
        } else {
          // If this discrepancy is before the Last Discrepancy
          // on a previous next, then pick the same as last time
          if (id_bit_number < LastDiscrepancy) {
            search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          } else {
            // If equal to last, pick 1, if not then pick 0
            search_direction = (id_bit_number == LastDiscrepancy);
          }

          // If 0 was picked, then record its position in LastZero
          if (!search_direction) {
            last_zero = id_bit_number;

            // Check for Last discrepancy in family
            if (last_zero < 9) {
              LastFamilyDiscrepancy = last_zero;
            }
          }
        }

        // Set or clear the bit in the ROM byte rom_byte_number with mask
        // rom_byte_mask
        // Serial.print(id_bit_number); Serial.print(" : ");
        // Serial.println(search_direction);
        if (search_direction) {
          ROM_NO[rom_byte_number] |= rom_byte_mask;
        } else {
          ROM_NO[rom_byte_number] &= ~rom_byte_mask;
        }

        // Serial number search direction write bit
        if (!OneWireWriteBit(search_direction)) {
          return false;
        }

        // Increment the byte counter id_bit_number and shift the mask
        // rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;

        // If the mask is 0, then go to new SerialNum byte rom_byte_number and
        // reset mask
        if (rom_byte_mask == 0) {
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    } while (rom_byte_number < 8); // Loop until through all ROM bytes 0-7

    // If the search was successful, then
    if (!(id_bit_number < 65)) {
      // Search successful, so set LastDiscrepancy, LastDeviceFlag,
      // search_result
      LastDiscrepancy = last_zero;

      // Check for last device
      if (LastDiscrepancy == 0) {
        LastDeviceFlag = true;
      }

      search_result = true;
    }
  }

  // If no device found, then reset counters so next 'search' will be like a
  // first
  if (!search_result || !ROM_NO[0]) {
    LastDiscrepancy = 0;
    LastDeviceFlag = false;
    LastFamilyDiscrepancy = 0;
    search_result = false;
  }

  for (int i = 0; i < 8; i++) {
    newAddr[i] = ROM_NO[i];
  }

  return search_result;
}
