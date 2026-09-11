#pragma once

#include <SPI.h>
extern SPIClass c6l_spi;
#define SPI c6l_spi
#include <SSD1306Spi.h>
#undef SPI
#include <helpers/ui/DisplayDriver.h>

#define SSD1306_BLACK BLACK
#define SSD1306_WHITE WHITE

class UnitC6LOLEDDisplay : public DisplayDriver {
  SSD1306Spi display;
  bool _isOn;
  uint8_t _color;
  int _cursorX;
  int _cursorY;

public:
  UnitC6LOLEDDisplay();

  bool begin();
  void sleep();
  void wake();
  bool isOn() override { return _isOn; }
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(ColorVal bkg = UIColor::window_bkg) override;
  void setTextSize(int sz) override;
  void setColor(ColorVal c) override;
  void setCursor(int x, int y) override;
  void print(const char* str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t* bits, int w, int h) override;
  uint16_t getTextWidth(const char* str) override;
  void endFrame() override;
};
