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
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
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
    return (status != 0xFF) && (status & 0x10); // Check if the RST bit (bit 4) is set
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
 *    @brief  Waits for the 1-Wire bus to be not busy, up to the specified timeout
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
 *    @brief  Configures the active pullup (APU) bit in the configuration register
 *    @param  enable
 *            True to enable the active pullup, false to disable
 *    @return True if configuration was successful, otherwise false
 */
/*!
 *    @brief  Configures the active pullup (APU) bit in the configuration register
 *    @param  enable
 *            True to enable the active pullup, false to disable
 *    @return True if configuration was successful, otherwise false
 */
/*!
 *    @brief  Configures the active pullup (APU) bit in the configuration register
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
 *    @brief  Configures the strong pullup (SPU) bit in the configuration register
 *    @param  enable
 *            True to enable the strong pullup, false to disable
 *    @return True if configuration was successful, otherwise false
 */
/*!
 *    @brief  Configures the strong pullup (SPU) bit in the configuration register
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
 *    @brief  Configures the overdrive speed (1WS) bit in the configuration register
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
    return status != 0xFF && (status & 0x02); // Check the presence pulse detected bit
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
    return status != 0xFF && (status & 0x80); // Check the branch direction taken bit
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

    uint8_t cmd[2] = { DS248X_CMD_WRITE_CONFIG, config_value };
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
    uint8_t cmd[2] = { DS248X_CMD_SET_READ_PTR, reg };
    return i2c_dev->write(cmd, 2);
}
