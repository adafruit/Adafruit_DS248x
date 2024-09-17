#include "Adafruit_DS248x.h"

#define DS18B20_CMD_SKIP_ROM 0xCC
#define DS18B20_CMD_CONVERT_T 0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

Adafruit_DS248x ds248x;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit DS2482-800 8 channel test sketch!");
  if (!ds248x.begin(&Wire, DS248X_ADDRESS)) {
    Serial.println(F("DS2482-800 initialization failed."));
    while (1);
  }
  Serial.println("DS2482-800 OK!");
}


void loop() {

  for (int i = 0; i < 8; i++) {
    ds248x.selectChannel(i);
    Serial.print("Reading channel: ");
    Serial.println(i);
    float temperature = readTemperature(i);
    Serial.print("\tTemperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    delay(1000);
  }
  
}

float readTemperature(uint8_t channel) {
    // Select the channel on the DS2482-800
    if (!ds248x.selectChannel(channel)) {
        // Handle error if channel selection fails
        Serial.println("Failed to select channel");
        return NAN; // Return 'Not a Number' to indicate an error
    }

    // Start temperature conversion
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_SKIP_ROM); // Skip ROM command
    ds248x.OneWireWriteByte(DS18B20_CMD_CONVERT_T); // Convert T command
    delay(750); // Wait for conversion (750ms for maximum precision)

    // Read scratchpad
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_SKIP_ROM); // Skip ROM command
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
