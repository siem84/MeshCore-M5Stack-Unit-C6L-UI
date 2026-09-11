#include <Arduino.h>
#include "target.h"

UnitC6LBoard board;

SPIClass c6l_spi(0);

namespace {
constexpr uint8_t C6L_IO_ADDRESS = 0x43;
constexpr uint8_t C6L_IO_INPUT = 0x0F;
constexpr uint8_t C6L_IO_CONFIG = 0x03;
constexpr uint8_t C6L_IO_OUTPUT = 0x05;
constexpr uint8_t C6L_IO_HIGH_IMPEDANCE = 0x07;
constexpr uint8_t C6L_IO_PULL_ENABLE = 0x0B;
constexpr uint8_t C6L_IO_PULL_SELECT = 0x0D;
constexpr uint8_t C6L_IO_ID = 0x01;
constexpr uint8_t C6L_BUTTON_MASK = 1 << 0;
constexpr uint8_t C6L_LNA_MASK = 1 << 5;
constexpr uint8_t C6L_ANT_MASK = 1 << 6;
constexpr uint8_t C6L_RESET_MASK = 1 << 7;
constexpr uint8_t C6L_RF_MASK = C6L_LNA_MASK | C6L_ANT_MASK | C6L_RESET_MASK;
constexpr int C6L_OLED_CS = 6;
constexpr uint32_t C6L_BUTTON_DEBOUNCE_MS = 40;
bool c6l_rf_ready = false;

bool readExpanderRegister(uint8_t reg, uint8_t& value) {
  Wire.beginTransmission(C6L_IO_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(C6L_IO_ADDRESS, 1) != 1) {
    return false;
  }
  value = Wire.read();
  return true;
}

bool updateExpanderRegister(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t current;
  if (!readExpanderRegister(reg, current)) return false;
  Wire.beginTransmission(C6L_IO_ADDRESS);
  Wire.write(reg);
  Wire.write((current & ~mask) | (value & mask));
  return Wire.endTransmission() == 0;
}
}

#if defined(DISPLAY_CLASS) || defined(C6L_OLED_TEST)
  #ifdef DISPLAY_CLASS
    DISPLAY_CLASS display;
  #else
    UnitC6LOLEDDisplay display;
  #endif
#endif

#if defined(P_LORA_SCLK)
  RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, c6l_spi);
#else
  RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY);
#endif

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);
SensorManager sensors;

void UnitC6LBoard::begin() {
  ESP32Board::begin();
  pinMode(C6L_OLED_CS, OUTPUT);
  digitalWrite(C6L_OLED_CS, HIGH);
  pinMode(P_LORA_NSS, OUTPUT);
  digitalWrite(P_LORA_NSS, HIGH);
  c6l_spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);
  c6l_rf_ready = false;
  expanderReady = readExpanderRegister(C6L_IO_ID, expanderId) &&
      updateExpanderRegister(C6L_IO_CONFIG, C6L_BUTTON_MASK, 0) &&
      updateExpanderRegister(C6L_IO_HIGH_IMPEDANCE, C6L_BUTTON_MASK, 0) &&
      updateExpanderRegister(C6L_IO_PULL_ENABLE, C6L_BUTTON_MASK, C6L_BUTTON_MASK) &&
      updateExpanderRegister(C6L_IO_PULL_SELECT, C6L_BUTTON_MASK, C6L_BUTTON_MASK);
  if (!expanderReady) {
    Serial.println("ERROR: C6L IO expander initialization failed");
    return;
  }

  // Preload LOW before enabling the RF outputs; preserve P0 and all other bits.
  if (!updateExpanderRegister(C6L_IO_OUTPUT, C6L_RF_MASK, 0) ||
      !updateExpanderRegister(C6L_IO_CONFIG, C6L_RF_MASK, C6L_RF_MASK) ||
      !updateExpanderRegister(C6L_IO_HIGH_IMPEDANCE, C6L_RF_MASK, 0)) {
    Serial.println("ERROR: C6L RF expander configuration failed");
    return;
  }

  // M5Stack sequence: SX_NRST LOW for 100 ms, HIGH, antenna switch ON, LNA ON.
  delay(100);
  if (!updateExpanderRegister(C6L_IO_OUTPUT, C6L_RESET_MASK, C6L_RESET_MASK) ||
      !updateExpanderRegister(C6L_IO_OUTPUT, C6L_ANT_MASK, C6L_ANT_MASK) ||
      !updateExpanderRegister(C6L_IO_OUTPUT, C6L_LNA_MASK, C6L_LNA_MASK)) {
    Serial.println("ERROR: C6L RF reset/enable sequence failed");
    return;
  }
  c6l_rf_ready = true;
}

bool UnitC6LBoard::isButtonPressed() {
  if (!expanderReady) return false;
  uint8_t input;
  if (!readExpanderRegister(C6L_IO_INPUT, input)) return buttonStable;

  const bool rawPressed = (input & C6L_BUTTON_MASK) == 0;
  const uint32_t now = millis();
  if (rawPressed != buttonLastRaw) {
    buttonLastRaw = rawPressed;
    buttonChangedAt = now;
  } else if (rawPressed != buttonStable && now - buttonChangedAt >= C6L_BUTTON_DEBOUNCE_MS) {
    buttonStable = rawPressed;
  }
  return buttonStable;
}

bool UnitC6LBoard::readButtonRawLevel(bool& level) {
  if (!expanderReady) return false;

  uint8_t input;
  if (!readExpanderRegister(C6L_IO_INPUT, input)) return false;

  level = (input & C6L_BUTTON_MASK) != 0;
  return true;
}

bool UnitC6LBoard::readButtonDiagnostics(bool& level, uint8_t& id) {
  if (!readExpanderRegister(C6L_IO_ID, id)) return false;
  uint8_t input;
  if (!readExpanderRegister(C6L_IO_INPUT, input)) return false;

  level = (input & C6L_BUTTON_MASK) != 0;
  return true;
}

void UnitC6LBoard::onBootComplete() {
#if defined(P_LORA_TX_NEOPIXEL_LED)
  setStatusLed(0, 32, 0);
  delay(150);
  setStatusLed(0, 0, 0);
#endif
}

bool radio_init() {
  if (!c6l_rf_ready) {
    Serial.println("ERROR: C6L RF hardware not initialized");
    return false;
  }
  fallback_clock.begin();
  rtc_clock.begin(Wire);

#if defined(P_LORA_SCLK)
  return radio.std_init(&c6l_spi);
#else
  return radio.std_init();
#endif
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);  // create new random identity
}
