#include "UnitC6LOLEDDisplay.h"

#include "UnitC6LBoard.h"

ColorVal UIColor::window_bkg = BLACK;
ColorVal UIColor::title_bkg = BLACK;
ColorVal UIColor::title_txt = WHITE;
ColorVal UIColor::primary_txt = WHITE;
ColorVal UIColor::secondary_txt = WHITE;
ColorVal UIColor::warning_txt = WHITE;
ColorVal UIColor::popup_bkg = BLACK;
ColorVal UIColor::popup_txt = WHITE;
ColorVal UIColor::corp_blue = WHITE;

namespace {
constexpr int OLED_DC = 18;
constexpr int OLED_RESET = 15;
constexpr int OLED_CS = 6;
}

UnitC6LOLEDDisplay::UnitC6LOLEDDisplay()
    : DisplayDriver(64, 48),
      display(OLED_RESET, OLED_DC, OLED_CS, GEOMETRY_64_48),
      _isOn(false),
      _color(WHITE),
      _cursorX(0),
      _cursorY(0) {}

bool UnitC6LOLEDDisplay::begin() {
  if (_isOn) return true;

  display.setHorizontalOffset(32);
  _isOn = display.init();
  if (_isOn) {
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.display();
  }
  return _isOn;
}

void UnitC6LOLEDDisplay::sleep() {
  if (_isOn) {
    display.displayOff();
    _isOn = false;
  }
}

void UnitC6LOLEDDisplay::wake() {
  if (!_isOn) {
    display.displayOn();
    _isOn = true;
  }
}

void UnitC6LOLEDDisplay::turnOn() {
  if (!_isOn && !begin()) return;
  display.displayOn();
}

void UnitC6LOLEDDisplay::turnOff() {
  display.displayOff();
  _isOn = false;
}

void UnitC6LOLEDDisplay::clear() {
  display.clear();
  display.display();
}

void UnitC6LOLEDDisplay::startFrame(ColorVal bkg) {
  display.clear();
  _color = WHITE;
  display.setColor(_color ? WHITE : BLACK);
  display.setFont(ArialMT_Plain_10);
  _cursorX = 0;
  _cursorY = 0;
}

void UnitC6LOLEDDisplay::setTextSize(int sz) {
  (void)sz;
}

void UnitC6LOLEDDisplay::setColor(ColorVal c) {
  _color = c;
  display.setColor(_color ? WHITE : BLACK);
}

void UnitC6LOLEDDisplay::setCursor(int x, int y) {
  _cursorX = x;
  _cursorY = y;
}

void UnitC6LOLEDDisplay::print(const char* str) {
  display.drawString(_cursorX, _cursorY, String(str));
}

void UnitC6LOLEDDisplay::fillRect(int x, int y, int w, int h) {
  display.setColor(_color ? WHITE : BLACK);
  display.fillRect(x, y, w, h);
}

void UnitC6LOLEDDisplay::drawRect(int x, int y, int w, int h) {
  display.setColor(_color ? WHITE : BLACK);
  display.drawRect(x, y, w, h);
}

void UnitC6LOLEDDisplay::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  display.setColor(_color ? WHITE : BLACK);
  display.drawXbm(x, y, w, h, bits);
}

uint16_t UnitC6LOLEDDisplay::getTextWidth(const char* str) {
  return display.getStringWidth(str);
}

void UnitC6LOLEDDisplay::endFrame() {
  display.display();
}
