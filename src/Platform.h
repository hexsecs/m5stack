#pragma once
// Small shim so the same code builds for the real device (Arduino/ESP32)
// and for the desktop simulator (M5Unified's SDL build, `pio run -e native_sim`).

#include <M5Unified.h>

#if defined(ARDUINO)

#include <Arduino.h>
#include <Preferences.h>
#include <esp_attr.h>

#else  // ---- desktop simulator

#include <map>
#include <string>

using lgfx::delay;
using lgfx::millis;

#define RTC_DATA_ATTR

// In-memory stand-in for ESP32 NVS storage (forgotten when the sim exits).
class Preferences {
 public:
  bool begin(const char* ns, bool /*readOnly*/ = false) {
    ns_ = ns;
    return true;
  }
  void end() {}
  uint8_t getUChar(const char* key, uint8_t def = 0) {
    auto it = store().find(ns_ + "/" + key);
    return it == store().end() ? def : it->second;
  }
  size_t putUChar(const char* key, uint8_t v) {
    store()[ns_ + "/" + key] = v;
    return 1;
  }

 private:
  static std::map<std::string, uint8_t>& store() {
    static std::map<std::string, uint8_t> s;
    return s;
  }
  std::string ns_;
};

#endif
