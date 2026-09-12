#include <Arduino.h>   // needed for PlatformIO
#include <Mesh.h>
#include "MyMesh.h"
#ifdef C6L_OLED_TEST
  #include <time.h>
  #include <stdlib.h>
  #include <helpers/ui/buzzer.h>
  genericBuzzer c6l_buzzer;
#endif

// Believe it or not, this std C function is busted on some platforms!
static uint32_t _atoi(const char* sp) {
  uint32_t n = 0;
  while (*sp && *sp >= '0' && *sp <= '9') {
    n *= 10;
    n += (*sp++ - '0');
  }
  return n;
}

// interface manager
#include <helpers/MultiSerialInterface.h>
MultiSerialInterface interface_manager;

// include bluetooth interface
#if defined(BLE_PIN_CODE)
  #ifdef ESP32
    // include esp32 bluetooth interface
    #include <helpers/esp32/SerialBLEInterface.h>
    SerialBLEInterface bluetooth_interface;
  #elif defined(NRF52_PLATFORM)
    // include nrf52 bluetooth interface
    #include <helpers/nrf52/SerialBLEInterface.h>
    SerialBLEInterface bluetooth_interface;
  #else
    #error "SerialBLEInterface is not defined for this platform"
  #endif
#endif

// include wifi interface
#ifdef WIFI_SSID
  #ifndef TCP_PORT
    #define TCP_PORT 5000
  #endif
  #ifdef ESP32
    // include esp32 wifi interface
    #include <helpers/esp32/SerialWifiInterface.h>
    SerialWifiInterface wifi_interface;
  #else
    #error "SerialWifiInterface is not defined for this platform"
  #endif
#endif

// include usb interface
#if defined(ENABLE_USB_INTERFACE)
  #include <helpers/ArduinoSerialInterface.h>
  ArduinoSerialInterface usb_serial_interface;
#endif

// include ethernet interface
#if defined(ETHERNET_ENABLED)
  #include <helpers/ethernet/EthernetInterface.h>
  ETHERNET_CLASS ethernet_interface;
#endif

// include hardware serial interface
#if defined(SERIAL_RX)
  #include <helpers/ArduinoSerialInterface.h>
  ArduinoSerialInterface hardware_serial_interface;
  HardwareSerial companion_serial(1);
#endif

// platform file system
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  #include <InternalFileSystem.h>
  #if defined(QSPIFLASH)
    #include <CustomLFS_QSPIFlash.h>
    DataStore store(InternalFS, QSPIFlash, rtc_clock);
  #else
    #if defined(EXTRAFS)
      #include <CustomLFS.h>
      CustomLFS ExtraFS(0xD4000, 0x19000, 128);
      DataStore store(InternalFS, ExtraFS, rtc_clock);
    #else
      DataStore store(InternalFS, rtc_clock);
    #endif
  #endif
#elif defined(RP2040_PLATFORM)
  #include <LittleFS.h>
  DataStore store(LittleFS, rtc_clock);
#elif defined(ESP32)
  #include <SPIFFS.h>
  DataStore store(SPIFFS, rtc_clock);
#endif

/* GLOBAL OBJECTS */
#ifdef DISPLAY_CLASS
  #include "UITask.h"
  UITask ui_task(&board, &interface_manager);
#endif

StdRNG fast_rng;
SimpleMeshTables tables;
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store
   #ifdef DISPLAY_CLASS
      , &ui_task
   #endif
);

#ifdef C6L_OLED_TEST
static unsigned long c6l_oled_last_update = 0;
static uint8_t c6l_oled_page = 0;
static bool c6l_button_was_pressed = false;
static uint32_t c6l_oled_last_activity_ms = 0;
static bool c6l_oled_sleeping = false;
static uint32_t c6l_logo_started_ms = 0;
static bool c6l_boot_logo_done = false;
static constexpr uint32_t C6L_BOOT_LOGO_MS = 3000;
static constexpr uint32_t C6L_PAIRING_TIMEOUT_MS = 30000;
static SerialBLEInterface::PairingDisplayEvent c6l_pairing = {};
static bool c6l_pairing_visible = false;
static bool c6l_pairing_was_sleeping = false;
static uint32_t c6l_pairing_shown_ms = 0;
static uint32_t c6l_rx_baseline = 0;
static uint32_t c6l_tx_baseline = 0;
static bool c6l_led_event_active = false;
static bool c6l_pending_rx = false;
static bool c6l_pending_tx = false;
static bool c6l_led_event_is_rx = false;
static uint32_t c6l_led_event_until = 0;
static constexpr uint32_t C6L_OLED_TIMEOUT_MS = 60000;
static constexpr uint8_t C6L_CLOCK_PAGE = 6;
static constexpr uint8_t C6L_PAGE_COUNT = 7;
static constexpr uint32_t C6L_MIN_VALID_TIMESTAMP = 1735689600;  // 1 Jan 2025 UTC

// Persisted IDs: never reorder these profiles.
static const struct { const char* name; const char* info; const char* tz; } c6l_timezones[] = {
  {"UTC", "UTC+00:00", "UTC0"},
  {"EUROPE", "AUTO DST", "CET-1CEST,M3.5.0/2,M10.5.0/3"},
  {"US EAST", "AUTO DST", "EST5EDT,M3.2.0/2,M11.1.0/2"},
  {"US WEST", "AUTO DST", "PST8PDT,M3.2.0/2,M11.1.0/2"},
  {"INDIA", "UTC+05:30", "IST-5:30"}
};
static constexpr uint8_t C6L_TZ_COUNT = sizeof(c6l_timezones) / sizeof(c6l_timezones[0]);
static bool c6l_timezone_mode = false;
static bool c6l_timezone_error = false;
static uint8_t c6l_timezone_choice = 0;
static uint32_t c6l_timezone_activity = 0;
static bool c6l_button_consumed = false;
static bool c6l_clock_press = false;
static uint32_t c6l_clock_pressed_ms = 0;

static uint8_t c6l_validTimezone(uint8_t id) {
  return id < C6L_TZ_COUNT ? id : 0;
}

static bool c6l_applyTimezone(uint8_t id) {
  if (setenv("TZ", c6l_timezones[c6l_validTimezone(id)].tz, 1) != 0) {
    Serial.println("ERROR: C6L timezone setup failed");
    return false;
  }
  tzset();
  return true;
}

static void c6l_setBleLed() {
  if (bluetooth_interface.isConnected()) {
    neopixelWrite(P_LORA_TX_NEOPIXEL_LED, 0, 0, 12);
  } else {
    neopixelWrite(P_LORA_TX_NEOPIXEL_LED, 10, 10, 10);
  }
}

static void c6l_updateLed() {
  const uint32_t now = millis();
  const uint32_t packets_recv = radio_driver.getPacketsRecv();
  const uint32_t packets_sent = radio_driver.getPacketsSent();

  if (packets_recv != c6l_rx_baseline) {
    c6l_rx_baseline = packets_recv;
    c6l_pending_rx = true;
  }
  if (packets_sent != c6l_tx_baseline) {
    c6l_tx_baseline = packets_sent;
    c6l_pending_tx = true;
  }

  if (c6l_led_event_active && static_cast<int32_t>(now - c6l_led_event_until) >= 0) {
    c6l_led_event_active = false;
  }

  if (!c6l_led_event_active) {
    if (c6l_pending_rx) {
      c6l_pending_rx = false;
      c6l_led_event_is_rx = true;
      c6l_led_event_active = true;
      c6l_led_event_until = now + 150;
    } else if (c6l_pending_tx) {
      c6l_pending_tx = false;
      c6l_led_event_is_rx = false;
      c6l_led_event_active = true;
      c6l_led_event_until = now + 150;
    }
  }

  if (c6l_led_event_active) {
    if (c6l_led_event_is_rx) {
      neopixelWrite(P_LORA_TX_NEOPIXEL_LED, 32, 0, 0);
    } else {
      neopixelWrite(P_LORA_TX_NEOPIXEL_LED, 32, 24, 0);
    }
  } else {
    c6l_setBleLed();
  }
}

static void updateC6LOledStatus() {
  char line[24];
  const NodePrefs* prefs = the_mesh.getNodePrefs();
  const char* ble_status = !bluetooth_interface.isEnabled() ? "OFF" :
                           bluetooth_interface.isConnected() ? "CON" : "ON";

  display.startFrame();
  display.setTextSize(1);
  display.setColor(SSD1306_WHITE);
  if (c6l_pairing_visible) {
    display.setCursor((64 - display.getTextWidth("PAIRING")) / 2, 0);
    display.print("PAIRING");
    if (c6l_pairing.pass_key <= 999999) {
      snprintf(line, sizeof(line), "%06lu",
               static_cast<unsigned long>(c6l_pairing.pass_key));
      // Six 9x18 seven-segment digits, spaced by 2 pixels: 64 pixels total.
      // Segment order: top, upper right, lower right, bottom, lower left,
      // upper left, middle. Draw through the existing OLED driver.
      static constexpr uint8_t segments[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
      };
      for (uint8_t i = 0; i < 6; ++i) {
        const int x = i * 11;
        const int y = 22;
        const uint8_t mask = segments[line[i] - '0'];
        if (mask & 0x01) display.fillRect(x + 2, y, 5, 2);
        if (mask & 0x02) display.fillRect(x + 7, y + 2, 2, 6);
        if (mask & 0x04) display.fillRect(x + 7, y + 10, 2, 6);
        if (mask & 0x08) display.fillRect(x + 2, y + 16, 5, 2);
        if (mask & 0x10) display.fillRect(x, y + 10, 2, 6);
        if (mask & 0x20) display.fillRect(x, y + 2, 2, 6);
        if (mask & 0x40) display.fillRect(x + 2, y + 8, 5, 2);
      }
    } else {
      display.setCursor((64 - display.getTextWidth("PIN ERROR")) / 2, 24);
      display.print("PIN ERROR");
    }
  } else if (c6l_timezone_mode) {
    const char* rows[] = {"TIMEZONE", c6l_timezones[c6l_timezone_choice].name,
                         c6l_timezone_error ? "TZ ERROR" : c6l_timezones[c6l_timezone_choice].info};
    for (uint8_t i = 0; i < 3; ++i) {
      display.setCursor((64 - display.getTextWidth(rows[i])) / 2, i * 16);
      display.print(rows[i]);
    }
  } else if (c6l_oled_page == 0) {
    const char* mesh_text = "MESH";
    const char* core_text = "CORE";
    const char* poland_text = "C6L UI";
    const int mesh_x = (64 - display.getTextWidth(mesh_text)) / 2;
    const int core_x = (64 - display.getTextWidth(core_text)) / 2;
    const int poland_x = (64 - display.getTextWidth(poland_text)) / 2;
    display.setCursor(mesh_x, 0);
    display.print(mesh_text);
    display.setCursor(mesh_x + 1, 0);
    display.print(mesh_text);
    display.setCursor(core_x, 14);
    display.print(core_text);
    display.setCursor(core_x + 1, 14);
    display.print(core_text);
    display.setCursor(poland_x, 34);
    display.print(poland_text);
  } else if (c6l_oled_page == 1) {
    display.setCursor(0, 0);
    display.print("MeshCore");
    display.setCursor(0, 12);
    snprintf(line, sizeof(line), "B:%s", ble_status);
    display.print(line);
    display.setCursor(0, 24);
    if (prefs != NULL && prefs->freq > 0) {
      snprintf(line, sizeof(line), "L:%.1f", prefs->freq);
    } else {
      snprintf(line, sizeof(line), "L:---");
    }
    display.print(line);
    display.setCursor(0, 36);
    snprintf(line, sizeof(line), "R:%lu T:%lu",
             static_cast<unsigned long>(radio_driver.getPacketsRecv()),
             static_cast<unsigned long>(radio_driver.getPacketsSent()));
    display.print(line);
  } else if (c6l_oled_page == 2) {
    display.setCursor(0, 0);
    display.print("RADIO");
    display.setCursor(0, 12);
    if (prefs != NULL) {
      snprintf(line, sizeof(line), "F:%.1f", prefs->freq);
    } else {
      snprintf(line, sizeof(line), "F:---");
    }
    display.print(line);
    display.setCursor(0, 24);
    if (prefs != NULL) {
      snprintf(line, sizeof(line), "BW:%.0f SF:%u", prefs->bw, prefs->sf);
    } else {
      snprintf(line, sizeof(line), "BW:--- SF:-");
    }
    display.print(line);
    display.setCursor(0, 36);
    if (prefs != NULL) {
      snprintf(line, sizeof(line), "P:%d CR:%u", prefs->tx_power_dbm, prefs->cr);
    } else {
      snprintf(line, sizeof(line), "P:-- CR:-");
    }
    display.print(line);
  } else if (c6l_oled_page == 3) {
    display.setCursor(0, 0);
    display.print("SIGNAL");
    display.setCursor(0, 12);
    if (radio_driver.getPacketsRecv() == 0) {
      display.print("RSSI:---");
      display.setCursor(0, 24);
      display.print("SNR:---");
    } else {
      snprintf(line, sizeof(line), "RSSI:%d",
               static_cast<int>(radio_driver.getLastRSSI()));
      display.print(line);
      display.setCursor(0, 24);
      snprintf(line, sizeof(line), "SNR:%.1f", radio_driver.getLastSNR());
      display.print(line);
    }
    display.setCursor(0, 36);
    snprintf(line, sizeof(line), "RX:%lu",
             static_cast<unsigned long>(radio_driver.getPacketsRecv()));
    display.print(line);
  } else if (c6l_oled_page == 4) {
    const uint32_t packets_recv = radio_driver.getPacketsRecv();
    display.setCursor(0, 0);
    display.print("RF MON");
    display.setCursor(0, 9);
    snprintf(line, sizeof(line), "NF %d", radio_driver.getNoiseFloor());
    display.print(line);
    display.setCursor(0, 18);
    if (packets_recv == 0) {
      display.print("R ---");
      display.setCursor(0, 27);
      display.print("S ---");
    } else {
      snprintf(line, sizeof(line), "R %d",
               static_cast<int>(radio_driver.getLastRSSI()));
      display.print(line);
      display.setCursor(0, 27);
      snprintf(line, sizeof(line), "S %+.1f", radio_driver.getLastSNR());
      display.print(line);
    }
    display.setCursor(0, 36);
    snprintf(line, sizeof(line), "RX %lu", static_cast<unsigned long>(packets_recv));
    if (display.getTextWidth(line) > 64) {
      if (packets_recv < 1000000UL) {
        snprintf(line, sizeof(line), "RX %.1fk", packets_recv / 1000.0);
      } else {
        snprintf(line, sizeof(line), "RX %.1fM", packets_recv / 1000000.0);
      }
    }
    display.print(line);
  } else if (c6l_oled_page == 5) {
    const uint32_t uptime = millis() / 1000;
    display.setCursor(0, 0);
    display.print("DEVICE");
    display.setCursor(0, 12);
    display.print("C6L");
    display.setCursor(0, 24);
    snprintf(line, sizeof(line), "UP:%02lu:%02lu",
             static_cast<unsigned long>((uptime / 3600) % 100),
             static_cast<unsigned long>((uptime / 60) % 60));
    display.print(line);
    display.setCursor(0, 36);
    snprintf(line, sizeof(line), "B:%s", ble_status);
    display.print(line);
  } else {
    const uint32_t timestamp = rtc_clock.getCurrentTime();
    const time_t t = static_cast<time_t>(timestamp);
    struct tm now = {};
    if (timestamp < C6L_MIN_VALID_TIMESTAMP || localtime_r(&t, &now) == NULL) {
      display.setCursor(0, 12);
      display.print("NO TIME");
      display.setCursor(0, 28);
      display.print("SYNC BLE");
    } else {
        char clock_line[6];
        char date_line[9];
      snprintf(clock_line, sizeof(clock_line), "%02d:%02d", now.tm_hour, now.tm_min);
      snprintf(date_line, sizeof(date_line), "%02d.%02d.%02d",
               now.tm_mday, now.tm_mon + 1, (now.tm_year + 1900) % 100);
      display.setCursor((64 - display.getTextWidth(clock_line)) / 2, 4);
      display.print(clock_line);
      display.setCursor((64 - display.getTextWidth(date_line)) / 2, 28);
      display.print(date_line);
    }
  }
  display.endFrame();
}
#endif

/* END GLOBAL OBJECTS */

void halt() {
  while (1) ;
}

/* WIFI RECONNECT TRACKERS */
#if defined(ESP32) && defined(WIFI_SSID)
  bool wifi_needs_reconnect = false;
  unsigned long last_wifi_reconnect_attempt = 0;
#endif

void setup() {
  Serial.begin(115200);
  board.begin();

#ifdef C6L_OLED_TEST
  c6l_buzzer.begin();
  c6l_buzzer.startup();
#endif

#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.begin();
#endif

#ifdef C6L_OLED_TEST
  if (display.begin()) {
    updateC6LOledStatus();
    c6l_logo_started_ms = millis();
    c6l_oled_last_update = c6l_logo_started_ms;
    c6l_oled_last_activity_ms = millis();
    c6l_oled_sleeping = false;
  }
#elif defined(DISPLAY_CLASS)
  DisplayDriver* disp = NULL;
  if (display.begin()) {
    disp = &display;
    disp->startFrame();
  #ifdef ST7789
    disp->setTextSize(2);
  #endif
    disp->drawTextCentered(disp->width() / 2, 28, "Loading...");
    disp->endFrame();
  }
#endif

  if (!radio_init()) { halt(); }

  fast_rng.begin(radio_driver.getRngSeed());

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  #if defined(QSPIFLASH)
    if (!QSPIFlash.begin()) {
      // debug output might not be available at this point, might be too early. maybe should fall back to InternalFS here?
      MESH_DEBUG_PRINTLN("CustomLFS_QSPIFlash: failed to initialize");
    } else {
      MESH_DEBUG_PRINTLN("CustomLFS_QSPIFlash: initialized successfully");
    }
  #else
  #if defined(EXTRAFS)
      ExtraFS.begin();
  #endif
  #endif
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );
#elif defined(ESP32)
  SPIFFS.begin(true);
  store.begin();
  the_mesh.begin(
    #ifdef DISPLAY_CLASS
        disp != NULL
    #else
        false
    #endif
  );
#else
  #error "need to define filesystem"
#endif

// add bluetooth interface
#ifdef C6L_OLED_TEST
  if (!c6l_applyTimezone(the_mesh.getNodePrefs()->c6l_timezone)) {
    c6l_applyTimezone(0);  // Best-effort UTC fallback; errors are logged.
  }
#endif
#if defined(BLE_PIN_CODE)
  bluetooth_interface.begin(BLE_NAME_PREFIX, the_mesh.getNodePrefs()->node_name, the_mesh.getBLEPin());
  interface_manager.addInterface(InterfaceType::Bluetooth, &bluetooth_interface);
#endif

// add wifi interface
#ifdef WIFI_SSID
  board.setInhibitSleep(true);   // prevent sleep when WiFi is active
  WiFi.setAutoReconnect(true);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
      if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
          WIFI_DEBUG_PRINTLN("WiFi disconnected. Flagging for reconnect...");
          wifi_needs_reconnect = true;
      } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
          WIFI_DEBUG_PRINTLN("WiFi connected successfully!");
          wifi_needs_reconnect = false;
      }
  });

  WiFi.begin(WIFI_SSID, WIFI_PWD);
  wifi_interface.begin(TCP_PORT);
  interface_manager.addInterface(InterfaceType::WiFi, &wifi_interface);
#endif

// add usb interface
#if defined(ENABLE_USB_INTERFACE)
  usb_serial_interface.begin(Serial);
  interface_manager.addInterface(InterfaceType::USB, &usb_serial_interface);
#endif

// add ethernet interface
#if defined(ETHERNET_ENABLED)
  ethernet_interface.begin();
  interface_manager.addInterface(InterfaceType::Ethernet, &ethernet_interface);
#endif

// add hardware serial interface
#if defined(SERIAL_RX)
  companion_serial.setPins(SERIAL_RX, SERIAL_TX);
  companion_serial.begin(115200);
  hardware_serial_interface.begin(companion_serial);
  interface_manager.addInterface(InterfaceType::HardwareSerial, &hardware_serial_interface);
#endif

  the_mesh.startInterface(interface_manager);
  sensors.begin();

#if ENV_INCLUDE_GPS == 1
  the_mesh.applyGpsPrefs();
#endif

#ifdef DISPLAY_CLASS
  ui_task.begin(disp, &sensors, the_mesh.getNodePrefs());  // still want to pass this in as dependency, as prefs might be moved
#endif

  board.onBootComplete();
#ifdef C6L_OLED_TEST
  c6l_rx_baseline = radio_driver.getPacketsRecv();
  c6l_tx_baseline = radio_driver.getPacketsSent();
  c6l_setBleLed();
#endif
}

void loop() {
  the_mesh.loop();
  interface_manager.loop();
  sensors.loop();
#ifdef C6L_OLED_TEST
  c6l_updateLed();
  if (c6l_buzzer.isPlaying()) c6l_buzzer.loop();
  const bool pairing_was_visible = c6l_pairing_visible;
  const bool pairing_changed = bluetooth_interface.readPairingDisplayEvent(c6l_pairing);
  const uint32_t pairing_now = millis();
  if (c6l_pairing.active) {
    c6l_timezone_mode = false;  // Discard any unconfirmed selection before handling P0.
    c6l_clock_press = false;
    c6l_button_consumed = true;
  }
  if (c6l_pairing.active &&
      (uint32_t)(pairing_now - c6l_pairing.timestamp_ms) >= C6L_PAIRING_TIMEOUT_MS) {
    c6l_pairing.active = false;  // UI timeout only; do not change BLE state.
  }
  if (!c6l_boot_logo_done &&
      (uint32_t)(pairing_now - c6l_logo_started_ms) >= C6L_BOOT_LOGO_MS) {
    c6l_boot_logo_done = true;
  }
  if (c6l_boot_logo_done && c6l_pairing.active && !c6l_pairing_visible) {
    c6l_pairing_was_sleeping = c6l_oled_sleeping;
    c6l_pairing_shown_ms = pairing_now;
    c6l_pairing_visible = true;
    if (c6l_oled_sleeping) display.wake();
    c6l_oled_sleeping = false;
  } else if (c6l_pairing_visible && !c6l_pairing.active) {
    c6l_pairing_visible = false;
    // Pause the normal auto-off countdown while the overlay is visible.
    c6l_oled_last_activity_ms += (uint32_t)(pairing_now - c6l_pairing_shown_ms);
    c6l_oled_sleeping = c6l_pairing_was_sleeping;
    updateC6LOledStatus();  // Restore the unchanged normal page, removing the PIN.
    if (c6l_oled_sleeping) display.sleep();
    c6l_oled_last_update = pairing_now;
  }
  if (c6l_pairing_visible && (!pairing_was_visible || pairing_changed)) {
    updateC6LOledStatus();
    c6l_oled_last_update = pairing_now;
  }
  const bool button_pressed = board.isButtonPressed();
  const uint32_t button_now = millis();
  bool ui_changed = false;
  if (!c6l_boot_logo_done || c6l_pairing.active || c6l_pairing_visible || pairing_was_visible) {
    c6l_clock_press = false;
    c6l_button_consumed = true;
  } else if (c6l_button_consumed) {
    if (!button_pressed) c6l_button_consumed = false;
  } else {
    if (button_pressed && !c6l_button_was_pressed) {
      c6l_oled_last_activity_ms = button_now;
      c6l_timezone_activity = button_now;
      if (c6l_oled_sleeping) {
        display.wake();
        c6l_oled_sleeping = false;
        c6l_button_consumed = true;
        ui_changed = true;
      } else if (c6l_oled_page == C6L_CLOCK_PAGE) {
        c6l_clock_press = true;
        c6l_clock_pressed_ms = button_now;
      } else {
        c6l_oled_page = (c6l_oled_page + 1) % C6L_PAGE_COUNT;
        ui_changed = true;
      }
    }
    if (c6l_clock_press) {
      const uint32_t duration = button_now - c6l_clock_pressed_ms;
      if (duration >= 3000) {
        c6l_clock_press = false;
        c6l_button_consumed = true;
        c6l_timezone_activity = button_now;
        if (!c6l_timezone_mode) {
          c6l_timezone_choice = c6l_validTimezone(the_mesh.getNodePrefs()->c6l_timezone);
          c6l_timezone_error = false;
          c6l_timezone_mode = true;
        } else {
          NodePrefs* prefs = the_mesh.getNodePrefs();
          if (c6l_timezone_choice == prefs->c6l_timezone && !c6l_timezone_error) {
            c6l_timezone_mode = false;
          } else if (c6l_applyTimezone(c6l_timezone_choice)) {
            prefs->c6l_timezone = c6l_timezone_choice;
            const bool saved = the_mesh.savePrefs();
            c6l_timezone_error = !saved;
            c6l_timezone_mode = !saved;
          } else {
            c6l_timezone_error = true;
          }
        }
        ui_changed = true;
      } else if (!button_pressed) {
        c6l_clock_press = false;
        c6l_timezone_activity = button_now;
        if (c6l_timezone_mode) {
          c6l_timezone_choice = (c6l_timezone_choice + 1) % C6L_TZ_COUNT;
          c6l_timezone_error = false;
        } else {
          c6l_oled_page = 0;
        }
        ui_changed = true;
      }
    }
  }
  if (c6l_timezone_mode && !button_pressed && !c6l_clock_press &&
      (uint32_t)(button_now - c6l_timezone_activity) >= 30000) {
    c6l_timezone_mode = false;
    ui_changed = true;
  }
  if (ui_changed) {
      updateC6LOledStatus();
      c6l_oled_last_update = button_now;
  }
  c6l_button_was_pressed = button_pressed;
  const uint32_t now = millis();
  if (!c6l_oled_sleeping && !c6l_pairing_visible && c6l_oled_page != C6L_CLOCK_PAGE &&
      (uint32_t)(now - c6l_oled_last_activity_ms) >= C6L_OLED_TIMEOUT_MS) {
    display.sleep();
    c6l_oled_sleeping = true;
  }
  const uint32_t refresh_interval = !c6l_pairing_visible && c6l_oled_page == C6L_CLOCK_PAGE ? 1000 : 750;
  if (!c6l_oled_sleeping && c6l_boot_logo_done && (c6l_pairing_visible || c6l_oled_page != 0) &&
      now - c6l_oled_last_update >= refresh_interval) {
    updateC6LOledStatus();
    c6l_oled_last_update = now;
  }


#endif
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.loop();
#endif

  if (!the_mesh.hasPendingWork()) {
#if defined(NRF52_PLATFORM)
    board.sleep(0); // nrf ignores seconds param, sleeps whenever possible
#endif
  }

#if defined(ESP32) && defined(WIFI_SSID)
  // Safely attempt to reconnect every 10 seconds if flagged
  if (wifi_needs_reconnect && (millis() - last_wifi_reconnect_attempt > 10000)) {
    WIFI_DEBUG_PRINTLN("Attempting manual WiFi reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();
    last_wifi_reconnect_attempt = millis();
  }
#endif
}
