// Demo specifically for ESP32-S2 Feather with TFT!

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "Adafruit_DS248x.h"

#define DS18B20_FAMILY_CODE 0x28
#define DS18B20_CMD_CONVERT_T 0x44
#define DS18B20_CMD_MATCH_ROM 0x55
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_DS248x ds248x;

void setup() {
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
  delay(10);

  Serial.begin(115200);
  //while (!Serial) delay(10);

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLUE);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
    
  if (!ds248x.begin(&Wire, DS248X_ADDRESS)) {
    Serial.println(F("DS248x initialization failed."));
    tft.setCursor(10, 10);
    tft.println("DS248x init failed.");
    while (1);
  }
  
  Serial.println("DS248x OK!");
}

void loop() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0, 0);

  if (!ds248x.OneWireReset()) {
    Serial.println("Failed to do a 1W reset");
    tft.println("1W reset failed.");
    delay(1000);
    return;
  }
  
  Serial.println("One Wire bus reset OK");

  while (1) {
    uint8_t rom[8];
  
    if (!ds248x.OneWireSearch(rom)) {
      Serial.println("No more devices found\n\n");
      break;
    }
  
    Serial.print("Found device ROM: ");
    tft.print("ROM ");
    for (int i = 0; i < 8; i++) {
      if (rom[i] < 16) {
        Serial.print("0");
        tft.print("0");
      }
      Serial.print(rom[i], HEX);
      tft.print(rom[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    tft.println();
  
    // Check if the device is a DS18B20 (Family code 0x28)
    if (rom[0] != DS18B20_FAMILY_CODE) {
      Serial.println("Not a DS18B20?");
      continue;
    }
    
    // Read and print temperature
    float temperature = readTemperature(rom);
    Serial.print("\tTemperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    tft.print("Temp: ");
    tft.print(temperature);
    tft.println(" C\n");
  }
  delay(5000);
}

float readTemperature(uint8_t *rom) {
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_MATCH_ROM);
    for (int i = 0; i < 8; i++) {
        ds248x.OneWireWriteByte(rom[i]);
    }

    ds248x.OneWireWriteByte(DS18B20_CMD_CONVERT_T);
    delay(750);

    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_MATCH_ROM);
    for (int i = 0; i < 8; i++) {
        ds248x.OneWireWriteByte(rom[i]);
    }
    ds248x.OneWireWriteByte(DS18B20_CMD_READ_SCRATCHPAD);

    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        ds248x.OneWireReadByte(&data[i]);
    }

    int16_t raw = (data[1] << 8) | data[0];
    float celsius = (float)raw / 16.0;

    return celsius;
}
