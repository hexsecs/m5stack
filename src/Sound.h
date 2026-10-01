#pragma once
// Tiny non-blocking melody player for the StickC buzzer.

#include "Platform.h"

struct Note {
  uint16_t freq;  // Hz, 0 = rest
  uint16_t ms;
};

#define MELODY(name, ...) static const Note name[] = {__VA_ARGS__}
#define MELODY_LEN(name) (sizeof(name) / sizeof(name[0]))

// Note frequencies (Hz).
enum : uint16_t {
  REST = 0,
  N_C4 = 262, N_D4 = 294, N_E4 = 330, N_F4 = 349, N_G4 = 392, N_A4 = 440, N_B4 = 494,
  N_C5 = 523, N_D5 = 587, N_E5 = 659, N_F5 = 698, N_G5 = 784, N_A5 = 880, N_B5 = 988,
  N_C6 = 1047, N_E6 = 1319, N_G6 = 1568,
};

class Sound {
 public:
  void play(const Note* notes, size_t count) {
    notes_ = notes;
    count_ = count;
    index_ = 0;
    nextAt_ = millis();
  }
  void stop() {
    notes_ = nullptr;
    M5.Speaker.stop();
  }
  bool playing() const { return notes_ != nullptr; }

  void update() {
    if (!notes_ || (int32_t)(millis() - nextAt_) < 0) return;
    if (index_ >= count_) {
      notes_ = nullptr;
      return;
    }
    const Note& n = notes_[index_++];
    // Leave a short gap so repeated notes are heard as separate notes.
    if (n.freq) M5.Speaker.tone(n.freq, n.ms > 30 ? n.ms - 20 : n.ms);
    nextAt_ += n.ms;
  }

 private:
  const Note* notes_ = nullptr;
  size_t count_ = 0;
  size_t index_ = 0;
  uint32_t nextAt_ = 0;
};

extern Sound sound;
