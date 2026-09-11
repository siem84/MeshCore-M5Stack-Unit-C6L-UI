#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <helpers/ESP32Board.h>

extern SPIClass c6l_spi;

class UnitC6LBoard : public ESP32Board {
public:
  void begin();
  void onBootComplete() override;
  bool isButtonPressed();
  bool readButtonRawLevel(bool& level);
  bool readButtonDiagnostics(bool& level, uint8_t& id);

#if defined(P_LORA_TX_NEOPIXEL_LED)
  void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {
    neopixelWrite(P_LORA_TX_NEOPIXEL_LED, red, green, blue);
  }
#endif

  const char* getManufacturerName() const override {
    return "Unit C6L";
  }

private:
  bool expanderReady = false;
  bool buttonStable = false;
  bool buttonLastRaw = false;
  uint32_t buttonChangedAt = 0;
  uint8_t expanderId = 0;
};
