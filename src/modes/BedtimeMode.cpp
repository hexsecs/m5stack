#include "BedtimeMode.h"

#include "../Platform.h"
#include <math.h>

#include "../Config.h"
#include "../Sound.h"

namespace {

struct Step {
  const char* line1;
  const char* line2;
  IconFn icon;
  uint32_t accent;
  const char* hint;  // small grown-up note, may be null
};

// The routine, in order. Up to 13 steps fit in Checklist's bitmask.
const Step kSteps[] = {
    {"TOYS", "AWAY", iconToys, 0x4DA3FF, nullptr},
    {"BRUSH", "TEETH", iconTeeth, 0x2EC4B6, nullptr},
    {"POTTY", "TIME", iconPotty, 0x8BD346, nullptr},
    {"BATH", "TIME", iconBath, 0xB38CFF, "hold side = skip"},
    {"PAJAMA", "POWER!", iconPajamas, 0x6FA8FF, nullptr},
    {"SIP OF", "WATER", iconWater, 0x3DD6D0, nullptr},
    {"READ 2", "BOOKS", iconBooks, 0xC4A1FF, nullptr},
    {"HUGS &", "KISSES", iconHugs, 0xFF6B9A, nullptr},
    {"TUCK IN", "STUFFIE", iconStuffie, 0xFF9F1C, nullptr},
    {"LIGHTS", "OUT", iconLightsOut, 0xFFD43B, nullptr},
};
constexpr uint8_t kStepCount = sizeof(kSteps) / sizeof(kSteps[0]);
static_assert(kStepCount <= 13, "Checklist supports at most 13 steps");

constexpr uint32_t kCheerMs = 1300;
constexpr uint32_t kAllDoneMs = 9000;
constexpr uint32_t kAllDoneMinMs = 3000;  // before a press can move things on
constexpr uint32_t kSuperMs = 7000;
constexpr uint32_t kGoodnightMs = 25000;
constexpr uint32_t kTapMaxMs = 700;      // B shorter than this = back
constexpr uint32_t kRestartHoldMs = 3000;  // B longer than this = start over
constexpr uint32_t kResumeWindowS = 3 * 3600;  // forget a half-done routine after 3h

MELODY(kStepDone, {N_C5, 90}, {N_E5, 90}, {N_G5, 90}, {N_C6, 240});
MELODY(kTwinkle, {N_C5, 280}, {N_C5, 280}, {N_G5, 280}, {N_G5, 280}, {N_A5, 280}, {N_A5, 280},
       {N_G5, 560}, {N_F5, 280}, {N_F5, 280}, {N_E5, 280}, {N_E5, 280}, {N_D5, 280}, {N_D5, 280},
       {N_C5, 560});
MELODY(kFanfare, {N_G4, 120}, {N_C5, 120}, {N_E5, 120}, {N_G5, 240}, {N_E5, 120}, {N_G5, 480},
       {REST, 120}, {N_C6, 700});
MELODY(kLullaby, {N_E5, 300}, {N_E5, 300}, {N_G5, 800}, {N_E5, 300}, {N_E5, 300}, {N_G5, 800},
       {N_E5, 300}, {N_G5, 300}, {N_C6, 600}, {N_B5, 500}, {N_A5, 500}, {N_A5, 400}, {N_G5, 900});
MELODY(kBack, {N_E5, 80}, {N_C5, 140});
MELODY(kSkip, {N_C5, 70}, {N_C5, 70});
MELODY(kRestart, {N_G5, 90}, {N_E5, 90}, {N_C5, 200});

constexpr uint32_t kBurstColors[] = {0xFFD43B, 0xFF6B9A, 0x4DA3FF, 0x8BD346, 0xFF9F1C, 0xC4A1FF};

// Survive deep sleep (but not a full power-off).
RTC_DATA_ATTR uint32_t rtcSnapshot = 0;
RTC_DATA_ATTR uint32_t rtcSleptAt = 0;
RTC_DATA_ATTR bool rtcHasSnapshot = false;

// Seconds-ish timestamp from the RTC chip. Not a true epoch (months are
// treated as 31 days) but monotonic, which is all the resume check needs.
uint32_t clockSeconds() {
  if (!M5.Rtc.isEnabled()) return 0;
  auto dt = M5.Rtc.getDateTime();
  uint32_t y = dt.date.year >= 2000 ? dt.date.year - 2000 : 0;
  return ((((y * 12 + dt.date.month) * 31 + dt.date.date) * 24 + dt.time.hours) * 60 +
          dt.time.minutes) * 60 + dt.time.seconds;
}

uint8_t loadStars() {
  Preferences p;
  p.begin("bedtime", true);
  uint8_t s = p.getUChar("stars", 0);
  p.end();
  return s;
}

void saveStars(uint8_t s) {
  Preferences p;
  p.begin("bedtime", false);
  p.putUChar("stars", s);
  p.end();
}

float ease(float t) { return t < 0 ? 0 : t > 1 ? 1 : 1 - (1 - t) * (1 - t); }

void drawPanel(M5Canvas& g, uint32_t accent) {
  g.fillRoundRect(6, 6, 108, 108, 14, col::PAPER);
  for (int i = 0; i < 3; ++i) g.drawRoundRect(6 + i, 6 + i, 108 - 2 * i, 108 - 2 * i, 14 - i, accent);
}

}  // namespace

BedtimeMode::BedtimeMode() : list_(kStepCount), week_(7) {}

void BedtimeMode::enter() {
  week_ = StarChart(7, loadStars());
  list_.reset();
  if (rtcHasSnapshot) {
    uint32_t now = clockSeconds();
    bool fresh = now == 0 || (now >= rtcSleptAt && now - rtcSleptAt < kResumeWindowS);
    if (fresh) list_.restore(rtcSnapshot);
    if (list_.finished()) list_.reset();
  }
  rtcHasSnapshot = false;
  waitForARelease_ = true;
  bDownAt_ = 0;
  setState(State::Step, millis());
}

void BedtimeMode::onSleep() {
  rtcSnapshot = list_.save();
  rtcSleptAt = clockSeconds();
  rtcHasSnapshot = true;
}

bool BedtimeMode::allowIdleSleep() const { return state_ == State::Step; }

void BedtimeMode::setState(State s, uint32_t now) {
  state_ = s;
  stateAt_ = now;
}

void BedtimeMode::finishRoutine(uint32_t now) {
  earnedPrize_ = week_.addStar();
  saveStars(week_.stars());
  sound.play(kTwinkle, MELODY_LEN(kTwinkle));
  setState(State::AllDone, now);
}

void BedtimeMode::handleGrownUpButton(uint32_t now) {
  if (M5.BtnB.wasPressed()) {
    bDownAt_ = now;
    bRestartFired_ = false;
  }
  if (!bDownAt_) return;

  if (M5.BtnB.isPressed() && !bRestartFired_ && now - bDownAt_ >= kRestartHoldMs) {
    list_.reset();
    sound.play(kRestart, MELODY_LEN(kRestart));
    flashText_ = "start over";
    flashUntil_ = now + 1200;
    bRestartFired_ = true;
  }

  if (M5.BtnB.wasReleased()) {
    uint32_t held = now - bDownAt_;
    bDownAt_ = 0;
    if (bRestartFired_) return;
    if (held < kTapMaxMs) {
      list_.back();
      sound.play(kBack, MELODY_LEN(kBack));
      flashText_ = "back";
    } else {
      list_.skip();
      sound.play(kSkip, MELODY_LEN(kSkip));
      flashText_ = "skipped";
      if (list_.finished()) finishRoutine(now);
    }
    flashUntil_ = now + 900;
  }
}

void BedtimeMode::update(M5Canvas& g, uint32_t now) {
  if (waitForARelease_ && !M5.BtnA.isPressed()) waitForARelease_ = false;
  bool aPressed = !waitForARelease_ && M5.BtnA.wasPressed();
  uint32_t elapsed = now - stateAt_;

  switch (state_) {
    case State::Step:
      if (aPressed && !list_.finished()) {
        cheerStep_ = list_.current();
        list_.complete();
        sound.play(kStepDone, MELODY_LEN(kStepDone));
        setState(State::Cheer, now);
        drawCheer(g, now);
        return;
      }
      handleGrownUpButton(now);
      if (state_ == State::Step) drawStep(g, now);
      else drawAllDone(g, now);
      return;

    case State::Cheer:
      if (elapsed >= kCheerMs) {
        if (list_.finished()) finishRoutine(now);
        else setState(State::Step, now);
      }
      drawCheer(g, now);
      return;

    case State::AllDone:
      if (elapsed >= kAllDoneMs || (aPressed && elapsed >= kAllDoneMinMs)) {
        if (earnedPrize_) {
          sound.play(kFanfare, MELODY_LEN(kFanfare));
          setState(State::SuperSleeper, now);
        } else {
          M5.Speaker.setVolume(kLullabyVolume);
          sound.play(kLullaby, MELODY_LEN(kLullaby));
          setState(State::Goodnight, now);
        }
      }
      drawAllDone(g, now);
      return;

    case State::SuperSleeper:
      if (elapsed >= kSuperMs) {
        week_.startNewWeekIfComplete();
        saveStars(week_.stars());
        earnedPrize_ = false;
        M5.Speaker.setVolume(kLullabyVolume);
        sound.play(kLullaby, MELODY_LEN(kLullaby));
        setState(State::Goodnight, now);
      }
      drawSuperSleeper(g, now);
      return;

    case State::Goodnight: {
      float t = (float)elapsed / kGoodnightMs;
      M5.Display.setBrightness((uint8_t)(kBrightness * (t >= 1 ? 0 : 1 - t)));
      if (elapsed >= kGoodnightMs) {
        list_.reset();  // fresh routine tomorrow
        setState(State::Asleep, now);
      }
      drawGoodnight(g, now);
      return;
    }

    case State::Asleep:
      return;
  }
}

// ---------------------------------------------------------------- drawing

void BedtimeMode::drawTitle(M5Canvas& g, const char* l1, const char* l2, uint32_t color) {
  constexpr int kCx = 179, kMaxW = 112;
  g.setTextDatum(middle_center);
  g.setFont(&fonts::FreeSansBold12pt7b);
  if (g.textWidth(l1) > kMaxW || g.textWidth(l2) > kMaxW) g.setFont(&fonts::FreeSansBold9pt7b);
  g.setTextColor(color);
  g.drawString(l1, kCx, 42);
  g.drawString(l2, kCx, 72);
}

void BedtimeMode::drawProgress(M5Canvas& g, uint32_t now) {
  for (uint8_t i = 0; i < list_.count(); ++i) {
    int x = 12 + i * 24, y = 126;
    if (list_.isDone(i)) {
      fillStar(g, x, y, 7, col::YELLOW);
    } else if (list_.isSkipped(i)) {
      g.fillCircle(x, y, 2, col::DIM);
    } else if (i == list_.current() && state_ == State::Step) {
      int r = 7 + (int)lroundf(sinf(now / 180.0f));
      drawStarOutline(g, x, y, r, col::WHITE);
    } else {
      drawStarOutline(g, x, y, 6, col::DIM);
    }
  }
}

void BedtimeMode::drawStep(M5Canvas& g, uint32_t now) {
  g.fillScreen(col::NAVY);
  if (list_.finished()) return;  // only momentarily, while finishing up
  const Step& s = kSteps[list_.current()];

  drawPanel(g, s.accent);
  int bob = (int)lroundf(3 * sinf(now / 260.0f));  // gentle "press me" wiggle
  s.icon(g, 60, 60 + bob);
  drawTitle(g, s.line1, s.line2, s.accent);

  const char* small = s.hint;
  if (M5.BtnB.isPressed() && bDownAt_ && !bRestartFired_) {
    uint32_t held = now - bDownAt_;
    if (held >= kTapMaxMs) small = held < kRestartHoldMs - 800 ? "let go = skip" : "keep holding...";
  }
  if (small) {
    g.setFont(&fonts::Font2);
    g.setTextDatum(middle_center);
    g.setTextColor((uint32_t)0xA9B4D9);
    g.drawString(small, 179, 100);
  }

  drawProgress(g, now);

  if ((int32_t)(flashUntil_ - now) > 0 && flashText_) {
    g.fillRoundRect(140, 90, 80, 22, 11, col::WHITE);
    g.setFont(&fonts::Font2);
    g.setTextDatum(middle_center);
    g.setTextColor(col::NAVY);
    g.drawString(flashText_, 180, 101);
  }
}

void BedtimeMode::drawCheer(M5Canvas& g, uint32_t now) {
  float t = (float)(now - stateAt_) / kCheerMs;
  if (t > 1) t = 1;
  const Step& s = kSteps[cheerStep_];

  g.fillScreen(col::NAVY);
  drawPanel(g, s.accent);
  int hop = (int)lroundf(-fabsf(sinf(t * 3 * M_PI)) * 10 * (1 - t));
  s.icon(g, 60, 60 + hop);

  // Green tick badge pops in.
  int r = (int)(24 * ease(t * 4));
  if (r > 2) {
    g.fillCircle(92, 92, r + 3, col::WHITE);
    g.fillCircle(92, 92, r, col::GREEN);
    if (r > 12) drawCheck(g, 92, 92, r / 2, col::WHITE);
  }

  // Burst of stars flying out from the picture.
  for (int i = 0; i < 10; ++i) {
    float a = i * (2 * M_PI / 10) + t * 1.5f;
    float d = 20 + ease(t) * 120;
    int sr = (int)(8 * (1 - 0.6f * t));
    fillStar(g, 60 + (int)(cosf(a) * d), 60 + (int)(sinf(a) * d), sr, kBurstColors[i % 6]);
  }

  g.setTextDatum(middle_center);
  g.setFont(&fonts::FreeSansBold18pt7b);
  g.setTextColor(col::YELLOW);
  float pop = t < 0.25f ? ease(t * 4) : 1;
  g.setTextSize(0.6f + 0.4f * pop);
  g.drawString("YAY!", 179, 58);
  g.setTextSize(1);

  drawProgress(g, now);
}

void BedtimeMode::drawWeek(M5Canvas& g, int y, uint32_t now, bool animateNewest) {
  constexpr int kSpacing = 30;
  int x0 = 120 - (week_.goal() - 1) * kSpacing / 2;
  for (uint8_t i = 0; i < week_.goal(); ++i) {
    int x = x0 + i * kSpacing;
    if (i + 1 < week_.stars() || (i + 1 == week_.stars() && !animateNewest)) {
      fillStar(g, x, y, 12, col::YELLOW);
    } else if (i + 1 == week_.stars()) {
      // Tonight's star grows in, overshoots, then twinkles.
      float t = (float)(now - stateAt_) / 1500.0f;
      float k = t < 1 ? ease(t) * 1.4f : 1.0f + 0.12f * sinf(now / 150.0f);
      fillStar(g, x, y, (int)(12 * k), col::YELLOW);
    } else {
      drawStarOutline(g, x, y, 12, col::DIM);
    }
  }
}

void BedtimeMode::drawAllDone(M5Canvas& g, uint32_t now) {
  uint32_t elapsed = now - stateAt_;
  g.fillScreen(col::NAVY);

  // Fireworks: a new burst every 900ms at a pseudo-random spot.
  for (int b = 0; b < 3; ++b) {
    uint32_t age = (elapsed + b * 300) % 900;
    uint32_t seed = (elapsed + b * 300) / 900 * 7 + b * 13;
    int bx = 30 + (seed * 53) % 180, by = 20 + (seed * 29) % 50;
    float t = age / 900.0f;
    uint32_t c = kBurstColors[seed % 6];
    for (int i = 0; i < 8; ++i) {
      float a = i * (M_PI / 4);
      float d = ease(t) * 26;
      int r = (int)(4 * (1 - t)) + 1;
      g.fillCircle(bx + (int)(cosf(a) * d), by + (int)(sinf(a) * d), r, c);
    }
  }

  g.setTextDatum(middle_center);
  g.setFont(&fonts::FreeSansBold18pt7b);
  g.setTextColor(col::YELLOW);
  g.drawString("ALL DONE!", 120, 40 + (int)lroundf(4 * sinf(now / 200.0f)));

  drawWeek(g, 100, now, true);
}

void BedtimeMode::drawSuperSleeper(M5Canvas& g, uint32_t now) {
  constexpr int cx = 120, cy = 54;
  g.fillScreen(col::NAVY);
  // Spinning sun-rays behind the big star.
  float spin = now / 1500.0f;
  for (int i = 0; i < 12; ++i) {
    float a0 = spin + i * (2 * M_PI / 12), a1 = a0 + 0.22f;
    g.fillTriangle(cx, cy, cx + (int)(cosf(a0) * 160), cy + (int)(sinf(a0) * 160),
                   cx + (int)(cosf(a1) * 160), cy + (int)(sinf(a1) * 160), (uint32_t)0x1B2B6B);
  }
  int r = 44 + (int)lroundf(4 * sinf(now / 180.0f));
  fillStar(g, cx, cy, r, col::YELLOW);
  g.fillCircle(cx - 8, cy, 3, col::INK);
  g.fillCircle(cx + 8, cy, 3, col::INK);
  drawSmile(g, cx, cy + 2, 8, col::INK);

  g.fillRect(0, 108, 240, 27, col::NAVY);
  g.setTextDatum(middle_center);
  g.setFont(&fonts::FreeSansBold12pt7b);
  g.setTextColor(col::YELLOW);
  g.drawString("SUPER SLEEPER!", 120, 120);
}

void BedtimeMode::drawGoodnight(M5Canvas& g, uint32_t now) {
  constexpr uint32_t kSky = 0x050B24;
  static const int16_t kStars[][2] = {{14, 18}, {120, 14}, {150, 30}, {226, 20}, {200, 110},
                                      {130, 118}, {20, 116}, {110, 70}, {226, 76}, {170, 100}};
  g.fillScreen(kSky);
  for (int i = 0; i < 10; ++i) {
    float tw = sinf(now / 400.0f + i * 1.7f);
    fillStar(g, kStars[i][0], kStars[i][1], 3 + (int)lroundf(tw * 1.5f), (uint32_t)0xFFE58A);
  }
  drawMoon(g, 62, 66, 36, kSky);
  g.setTextDatum(middle_center);
  g.setFont(&fonts::FreeSansBold12pt7b);
  g.setTextColor((uint32_t)0xFFE58A);
  g.drawString("Good", 172, 54);
  g.drawString("night", 172, 80);
}
