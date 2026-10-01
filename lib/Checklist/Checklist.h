#pragma once
// Hardware-independent state for a step-by-step checklist plus a weekly
// "star chart". Kept free of Arduino/M5 headers so it can be unit tested on
// the host (`pio test -e native`).

#include <stdint.h>

class Checklist {
 public:
  explicit Checklist(uint8_t stepCount) : count_(stepCount) {}

  uint8_t count() const { return count_; }
  uint8_t current() const { return current_; }
  bool finished() const { return current_ >= count_; }
  bool isDone(uint8_t i) const { return i < count_ && (doneMask_ >> i) & 1u; }
  bool isSkipped(uint8_t i) const { return i < count_ && (skipMask_ >> i) & 1u; }

  // Child pressed the big button: mark this step done and advance.
  void complete() { advance(true); }
  // Grown-up skipped an optional step (e.g. no bath tonight).
  void skip() { advance(false); }

  // Grown-up "oops": go back one step and un-mark it.
  void back() {
    if (current_ == 0) return;
    --current_;
    clearBit(current_);
  }

  void reset() {
    current_ = 0;
    doneMask_ = skipMask_ = 0;
  }

  // Snapshot for keeping progress across deep sleep.
  uint32_t save() const { return (uint32_t)current_ | (doneMask_ << 5) | ((uint32_t)skipMask_ << 18); }
  void restore(uint32_t s) {
    reset();
    uint8_t cur = s & 0x1F;
    if (cur > count_) return;  // stale/garbage snapshot: start over
    current_ = cur;
    doneMask_ = (s >> 5) & 0x1FFF;
    skipMask_ = (s >> 18) & 0x1FFF;
  }

 private:
  void advance(bool done) {
    if (finished()) return;
    clearBit(current_);
    if (done) doneMask_ |= 1u << current_;
    else skipMask_ |= 1u << current_;
    ++current_;
  }
  void clearBit(uint8_t i) {
    doneMask_ &= ~(1u << i);
    skipMask_ &= ~(1u << i);
  }

  uint8_t count_;
  uint8_t current_ = 0;
  uint16_t doneMask_ = 0;  // supports up to 13 steps
  uint16_t skipMask_ = 0;
};

// Weekly star chart: one star per finished night, a prize after `goal`.
class StarChart {
 public:
  explicit StarChart(uint8_t goal = 7, uint8_t stars = 0) : goal_(goal), stars_(stars > goal ? 0 : stars) {}

  uint8_t stars() const { return stars_; }
  uint8_t goal() const { return goal_; }

  // Adds tonight's star. Returns true when this star reaches the goal.
  bool addStar() {
    if (stars_ >= goal_) stars_ = 0;  // prize was already earned: new week
    ++stars_;
    return stars_ == goal_;
  }

  // After the prize has been celebrated, start a fresh week.
  void startNewWeekIfComplete() {
    if (stars_ >= goal_) stars_ = 0;
  }

 private:
  uint8_t goal_;
  uint8_t stars_;
};
