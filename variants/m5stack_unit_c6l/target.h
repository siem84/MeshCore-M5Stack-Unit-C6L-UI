#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include <RadioLib.h>
#include <UnitC6LBoard.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#include <helpers/ESP32Board.h>
#include <helpers/radiolib/CustomSX1262Wrapper.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/SensorManager.h>
#if defined(DISPLAY_CLASS) || defined(C6L_OLED_TEST)
  #include <UnitC6LOLEDDisplay.h>
  extern UnitC6LOLEDDisplay display;
#endif

extern UnitC6LBoard board;
extern WRAPPER_CLASS radio_driver;
extern AutoDiscoverRTCClock rtc_clock;
extern SensorManager sensors;

bool radio_init();
mesh::LocalIdentity radio_new_identity();