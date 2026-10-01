// Toddler Toolkit for M5StickC Plus / Plus2.
//
// main owns the frame buffer, sound, mode switching and power. Each mode
// lives in src/modes/. The power button (short click) cycles modes once
// there is more than one.

#include "Platform.h"

#if defined(ARDUINO)
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#endif

#include "Config.h"
#include "Mode.h"
#include "Sound.h"
#include "modes/BedtimeMode.h"

Sound sound;

namespace {

M5Canvas canvas(&M5.Display);

BedtimeMode bedtime;
Mode* const kModes[] = {&bedtime};
constexpr size_t kModeCount = sizeof(kModes) / sizeof(kModes[0]);

RTC_DATA_ATTR uint8_t rtcModeIndex = 0;  // remembered across deep sleep
uint32_t lastActivity = 0;

constexpr uint32_t kFrameMs = 33;  // ~30 fps
#if defined(ARDUINO)
constexpr gpio_num_t kWakePin = GPIO_NUM_37;  // big front button (A)
constexpr gpio_num_t kPlus2HoldPin = GPIO_NUM_4;  // keeps a Plus2 powered
#endif

Mode& mode() { return *kModes[rtcModeIndex]; }

void wake();

void goToSleep() {
  mode().onSleep();
  sound.stop();
  M5.Display.setBrightness(0);
  M5.Display.sleep();
  M5.Display.waitDisplay();

#if !defined(ARDUINO)
  // Simulator: blank screen until button A, then "wake up" like the device.
  M5.Display.fillScreen(TFT_BLACK);
  do {
    M5.delay(20);
    M5.update();
  } while (!M5.BtnA.wasPressed());
  M5.Display.wakeup();
  wake();
  return;
#else

  // The Plus2 has no PMIC: GPIO4 must stay high or the battery is cut off.
  if (M5.getBoard() == m5::board_t::board_M5StickCPlus2) {
    gpio_hold_en(kPlus2HoldPin);
    gpio_deep_sleep_hold_en();
  }

  // Wait for the button to be let go, otherwise we would wake immediately.
  while (!gpio_get_level(kWakePin)) delay(10);
  esp_sleep_enable_ext0_wakeup(kWakePin, 0);
  esp_deep_sleep_start();
#endif
}

// Everything that happens on power-up and on every wake from deep sleep.
void wake() {
  M5.Display.setBrightness(kBrightness);
  M5.Speaker.setVolume(kVolume);
  if (rtcModeIndex >= kModeCount) rtcModeIndex = 0;
  mode().enter();
  lastActivity = millis();
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
#if defined(ARDUINO)
  if (M5.getBoard() == m5::board_t::board_M5StickCPlus2) {
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis(kPlus2HoldPin);
  }
  // Waking from ext0 leaves the pin in RTC-IO mode; hand it back to GPIO.
  rtc_gpio_deinit(kWakePin);
#endif

  M5.Display.setRotation(kScreenRotation);
  canvas.setColorDepth(16);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  wake();
}

void loop() {
  uint32_t frameStart = millis();
  M5.update();
  sound.update();

  if (M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnPWR.isPressed()) lastActivity = frameStart;

  if (kModeCount > 1 && M5.BtnPWR.wasClicked()) {
    sound.stop();
    rtcModeIndex = (rtcModeIndex + 1) % kModeCount;
    wake();
  }

  mode().update(canvas, frameStart);
  canvas.pushSprite(0, 0);

  if (mode().wantsSleep() ||
      (mode().allowIdleSleep() && frameStart - lastActivity > kIdleSleepMs && !sound.playing())) {
    goToSleep();
  }

  uint32_t spent = millis() - frameStart;
  if (spent < kFrameMs) delay(kFrameMs - spent);
}

#if !defined(ARDUINO)
// Desktop simulator entry point (`pio run -e native_sim -t exec`).
// Keys: Left = button A (big front), Down = button B (side), Up = power.
static int simMain(bool* running) {
  setup();
  while (*running) loop();
  return 0;
}
int main(int, char**) { return lgfx::Panel_sdl::main(simMain); }
#endif
