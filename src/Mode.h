#pragma once
// A "mode" is one toddler activity in the toolkit (bedtime checklist,
// toothbrush timer, ...). main.cpp owns the loop, the frame buffer and
// power management; a mode just reads the buttons and draws a frame.

#include <M5Unified.h>

class Mode {
 public:
  virtual ~Mode() = default;

  // Called when the mode becomes active (boot, wake-up or mode switch).
  virtual void enter() = 0;
  // Called once per frame after M5.update(). Draw the whole frame into `gfx`.
  virtual void update(M5Canvas& gfx, uint32_t now) = 0;

  // Called right before deep sleep so the mode can stash its progress.
  virtual void onSleep() {}
  // Return true to put the device to sleep right away (e.g. after "goodnight").
  virtual bool wantsSleep() const { return false; }
  // Return false while the mode must stay awake regardless of button activity.
  virtual bool allowIdleSleep() const { return true; }
};
