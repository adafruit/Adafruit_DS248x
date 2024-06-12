#include "Adafruit_DS248x.h"

#define DS18B20_FAMILY_CODE 0x28
#define DS18B20_CMD_CONVERT_T 0x44
#define DS18B20_CMD_MATCH_ROM 0x55
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

Adafruit_DS248x ds248x;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit DS248x test sketch!");
  if (!ds248x.begin(&Wire, DS248X_ADDRESS)) {
    Serial.println(F("DS248x initialization failed."));
    while (1);
  }
  
  Serial.println("DS248x OK!");

  // Speed up I2C, as searching for ROMs, specifically, is slow!
  Wire.setClock(400000);

  while (! ds248x.OneWireReset()) {
    Serial.println("Failed to do a 1W reset");
    if (ds248x.shortDetected()) {
      Serial.println("\tShort detected");
    }
    if (!ds248x.presencePulseDetected()) {
      Serial.println("\tNo presense pulse");
    }
    delay(1000);
  }
  Serial.println("One Wire bus reset OK");
}


void loop() {
  uint8_t rom[8];

  if (!ds248x.OneWireSearch(rom)) {
    Serial.println("No more devices found\n\n");
    return;
  }

  Serial.print("Found device ROM: ");
  for (int i = 0; i < 8; i++) {
    if (rom[i] < 16) {
      Serial.print("0");
    }
    Serial.print(rom[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Check if the device is a DS18B20 (Family code 0x28)
  if (rom[0] == DS18B20_FAMILY_CODE) {
      // Read and print temperature
      float temperature = readTemperature(rom);
      Serial.print("\tTemperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
  }
}

float readTemperature(uint8_t *rom) {
    // Select the DS18B20 device
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_MATCH_ROM); // Match ROM command
    for (int i = 0; i < 8; i++) {
        ds248x.OneWireWriteByte(rom[i]);
    }

    // Start temperature conversion
    ds248x.OneWireWriteByte(DS18B20_CMD_CONVERT_T); // Convert T command
    delay(750); // Wait for conversion (750ms for maximum precision)

    // Read scratchpad
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_MATCH_ROM); // Match ROM command
    for (int i = 0; i < 8; i++) {
        ds248x.OneWireWriteByte(rom[i]);
    }
    ds248x.OneWireWriteByte(DS18B20_CMD_READ_SCRATCHPAD); // Read Scratchpad command

    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        ds248x.OneWireReadByte(&data[i]);
    }

    // Calculate temperature
    int16_t raw = (data[1] << 8) | data[0];
    float celsius = (float)raw / 16.0;

    return celsius;
}
