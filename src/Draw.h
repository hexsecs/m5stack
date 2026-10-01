#pragma once
// Drawing helpers and the picture icons. Everything is built from simple
// shapes so no image assets are needed. Icons are drawn centred on (cx, cy)
// and fit inside roughly a 100x100 box.
//
// Colours are always passed as uint32_t 0xRRGGBB (M5GFX treats a plain int as
// RGB565, so don't pass bare literals).

#include <M5Unified.h>

namespace col {
constexpr uint32_t NAVY = 0x0B1A4A;
constexpr uint32_t PAPER = 0xFFF8EC;
constexpr uint32_t WHITE = 0xFFFFFF;
constexpr uint32_t INK = 0x1B1B2F;
constexpr uint32_t YELLOW = 0xFFD43B;
constexpr uint32_t DIM = 0x45507A;
constexpr uint32_t GREEN = 0x22C55E;
}  // namespace col

void fillStar(M5Canvas& g, int cx, int cy, int r, uint32_t color);
void drawStarOutline(M5Canvas& g, int cx, int cy, int r, uint32_t color);
void drawCheck(M5Canvas& g, int cx, int cy, int size, uint32_t color);
void drawSmile(M5Canvas& g, int cx, int cy, int r, uint32_t color);

using IconFn = void (*)(M5Canvas& g, int cx, int cy);

void iconToys(M5Canvas& g, int cx, int cy);
void iconTeeth(M5Canvas& g, int cx, int cy);
void iconPotty(M5Canvas& g, int cx, int cy);
void iconBath(M5Canvas& g, int cx, int cy);
void iconPajamas(M5Canvas& g, int cx, int cy);
void iconWater(M5Canvas& g, int cx, int cy);
void iconBooks(M5Canvas& g, int cx, int cy);
void iconHugs(M5Canvas& g, int cx, int cy);
void iconStuffie(M5Canvas& g, int cx, int cy);
void iconLightsOut(M5Canvas& g, int cx, int cy);

// A sleepy crescent moon; `cutColor` must match the background behind it.
void drawMoon(M5Canvas& g, int cx, int cy, int r, uint32_t cutColor);
