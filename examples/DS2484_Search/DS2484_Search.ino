#include "Adafruit_DS248x.h"

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

  while (true) {
    if (!ds248x.OneWireSearch(rom)) {
      break;
    }

    Serial.print("ROM: ");
    for (int i = 0; i < 8; i++) {
      if (rom[i] < 16) {
        Serial.print("0");
      }
      Serial.print(rom[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("No more devices found.");

  while (1);
}
