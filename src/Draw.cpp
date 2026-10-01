#include "Draw.h"

#include <math.h>

namespace {
constexpr uint32_t kBrown = 0xC68642;
constexpr uint32_t kTan = 0xEED3A8;
constexpr uint32_t kDark = 0x3B2A20;
constexpr uint32_t kRed = 0xE53935;
constexpr uint32_t kPink = 0xFF7AA8;
constexpr uint32_t kBlue = 0x3B6FD8;
constexpr uint32_t kLightBlue = 0x9AD7FF;

void eyes(M5Canvas& g, int cx, int cy, int spread, int r = 3) {
  g.fillCircle(cx - spread, cy, r, col::INK);
  g.fillCircle(cx + spread, cy, r, col::INK);
}

// Closed, sleepy eye: a little downward curve.
void sleepyEye(M5Canvas& g, int cx, int cy, int r) {
  g.fillArc(cx, cy, r, r - 2, 20, 160, col::INK);
}
}  // namespace

void fillStar(M5Canvas& g, int cx, int cy, int r, uint32_t color) {
  int xs[10], ys[10];
  for (int i = 0; i < 10; ++i) {
    float a = -M_PI / 2 + i * M_PI / 5;
    float rr = (i & 1) ? r * 0.45f : r;
    xs[i] = cx + lroundf(cosf(a) * rr);
    ys[i] = cy + lroundf(sinf(a) * rr);
  }
  for (int i = 0; i < 10; ++i) {
    int j = (i + 1) % 10;
    g.fillTriangle(cx, cy, xs[i], ys[i], xs[j], ys[j], color);
  }
}

void drawStarOutline(M5Canvas& g, int cx, int cy, int r, uint32_t color) {
  int xs[10], ys[10];
  for (int i = 0; i < 10; ++i) {
    float a = -M_PI / 2 + i * M_PI / 5;
    float rr = (i & 1) ? r * 0.45f : r;
    xs[i] = cx + lroundf(cosf(a) * rr);
    ys[i] = cy + lroundf(sinf(a) * rr);
  }
  for (int i = 0; i < 10; ++i) {
    int j = (i + 1) % 10;
    g.drawLine(xs[i], ys[i], xs[j], ys[j], color);
  }
}

void drawCheck(M5Canvas& g, int cx, int cy, int s, uint32_t color) {
  // Two thick strokes: short down-right, long up-right.
  int x0 = cx - s, y0 = cy;
  int x1 = cx - s / 3, y1 = cy + s * 2 / 3;
  int x2 = cx + s, y2 = cy - s * 2 / 3;
  int t = s / 5 + 1;
  for (int k = -t; k <= t; ++k) {
    g.drawLine(x0, y0 + k, x1, y1 + k, color);
    g.drawLine(x1, y1 + k, x2, y2 + k, color);
  }
}

void drawSmile(M5Canvas& g, int cx, int cy, int r, uint32_t color) {
  g.fillArc(cx, cy, r, r - 2, 25, 155, color);
}

void iconToys(M5Canvas& g, int cx, int cy) {
  // Teddy peeking out of a toy tub, with blocks.
  int bx = cx - 12, by = cy - 14;
  g.fillCircle(bx - 14, by - 14, 8, kBrown);
  g.fillCircle(bx + 14, by - 14, 8, kBrown);
  g.fillCircle(bx, by, 19, kBrown);
  g.fillEllipse(bx, by + 6, 8, 6, kTan);
  g.fillEllipse(bx, by + 3, 3, 2, kDark);
  eyes(g, bx, by - 5, 7, 2);
  g.fillRect(cx + 10, cy - 24, 18, 18, (uint32_t)col::YELLOW);
  g.fillRect(cx + 22, cy - 14, 14, 14, kRed);
  // Tub
  g.fillRoundRect(cx - 42, cy - 2, 84, 38, 10, (uint32_t)0x6CC24A);
  g.fillRoundRect(cx - 46, cy - 6, 92, 11, 5, (uint32_t)0x4E9A2F);
  eyes(g, cx, cy + 14, 9);
  drawSmile(g, cx, cy + 14, 8, col::INK);
}

void iconTeeth(M5Canvas& g, int cx, int cy) {
  int tx = cx - 10;
  // Tooth: crown + two roots, outlined so it reads on the paper panel.
  g.fillRoundRect(tx - 31, cy - 35, 62, 46, 20, (uint32_t)0xBFD3E6);
  g.fillRoundRect(tx - 31, cy - 5, 24, 42, 11, (uint32_t)0xBFD3E6);
  g.fillRoundRect(tx + 7, cy - 5, 24, 42, 11, (uint32_t)0xBFD3E6);
  g.fillRoundRect(tx - 29, cy - 33, 58, 42, 18, col::WHITE);
  g.fillRoundRect(tx - 29, cy - 3, 20, 38, 9, col::WHITE);
  g.fillRoundRect(tx + 9, cy - 3, 20, 38, 9, col::WHITE);
  eyes(g, tx, cy - 16, 10);
  drawSmile(g, tx, cy - 10, 9, col::INK);
  fillStar(g, tx - 36, cy - 34, 6, col::YELLOW);
  // Toothbrush
  int bx = cx + 34;
  g.fillRoundRect(bx - 4, cy - 10, 9, 52, 4, kBlue);
  g.fillRoundRect(bx - 5, cy - 32, 11, 24, 4, kBlue);
  g.fillRect(bx - 12, cy - 30, 8, 18, kLightBlue);
  fillStar(g, bx + 2, cy - 42, 6, col::YELLOW);
}

void iconPotty(M5Canvas& g, int cx, int cy) {
  constexpr uint32_t kGreen = 0x7ED957, kDarkGreen = 0x4E9A2F;
  g.fillRoundRect(cx - 32, cy - 40, 64, 30, 10, kGreen);  // back
  g.fillRoundRect(cx - 36, cy - 12, 72, 42, 14, kGreen);  // bowl
  g.fillRoundRect(cx - 28, cy + 26, 56, 12, 5, kDarkGreen);  // base
  g.fillEllipse(cx, cy - 12, 38, 10, (uint32_t)0xB6EDAE);     // seat
  g.fillEllipse(cx, cy - 12, 24, 5, (uint32_t)0x2F6B22);      // hole
  eyes(g, cx, cy + 6, 10);
  drawSmile(g, cx, cy + 8, 9, col::INK);
}

void iconBath(M5Canvas& g, int cx, int cy) {
  constexpr uint32_t kDuck = 0xFFD43B, kWing = 0xF2B705, kBeak = 0xFF8C1A, kFoam = 0xD8F1FF;
  g.fillTriangle(cx - 44, cy - 6, cx - 30, cy + 14, cx - 20, cy + 2, kDuck);  // tail
  g.fillEllipse(cx - 4, cy + 12, 38, 22, kDuck);
  g.fillEllipse(cx - 8, cy + 8, 17, 10, kWing);
  g.fillCircle(cx + 18, cy - 18, 18, kDuck);
  g.fillTriangle(cx + 32, cy - 22, cx + 50, cy - 14, cx + 32, cy - 9, kBeak);
  g.fillCircle(cx + 22, cy - 24, 3, col::INK);
  // Bubbles
  g.fillCircle(cx - 36, cy + 32, 10, kFoam);
  g.fillCircle(cx - 18, cy + 37, 9, kFoam);
  g.fillCircle(cx + 2, cy + 36, 10, kFoam);
  g.fillCircle(cx + 24, cy + 34, 11, kFoam);
  g.fillCircle(cx + 42, cy + 26, 7, kFoam);
  g.drawCircle(cx - 40, cy - 34, 6, kLightBlue);
  g.drawCircle(cx - 26, cy - 42, 4, kLightBlue);
}

void iconPajamas(M5Canvas& g, int cx, int cy) {
  // Sleeves (each a quad made of two triangles), then body.
  g.fillTriangle(cx - 26, cy - 32, cx - 50, cy + 2, cx - 26, cy - 8, kBlue);
  g.fillTriangle(cx - 50, cy + 2, cx - 38, cy + 12, cx - 26, cy - 8, kBlue);
  g.fillTriangle(cx + 26, cy - 32, cx + 50, cy + 2, cx + 26, cy - 8, kBlue);
  g.fillTriangle(cx + 50, cy + 2, cx + 38, cy + 12, cx + 26, cy - 8, kBlue);
  g.fillRoundRect(cx - 27, cy - 34, 54, 72, 8, kBlue);
  g.fillTriangle(cx - 13, cy - 34, cx + 13, cy - 34, cx, cy - 18, (uint32_t)0xBFD7FF);
  for (int y : {-8, 6, 20}) g.fillCircle(cx, cy + y, 3, col::WHITE);
  fillStar(g, cx - 14, cy + 2, 7, col::YELLOW);
  fillStar(g, cx + 14, cy - 14, 6, col::YELLOW);
  fillStar(g, cx + 14, cy + 24, 7, col::YELLOW);
  fillStar(g, cx - 14, cy + 28, 5, col::YELLOW);
  fillStar(g, cx - 38, cy - 2, 4, col::YELLOW);
  fillStar(g, cx + 38, cy - 2, 4, col::YELLOW);
}

void iconWater(M5Canvas& g, int cx, int cy) {
  int x = cx - 14;
  g.fillRoundRect(x - 8, cy - 46, 16, 14, 4, (uint32_t)0x1E5BB8);  // spout
  g.fillRoundRect(x - 28, cy - 36, 56, 12, 4, (uint32_t)0x1E5BB8); // lid
  g.fillRoundRect(x - 24, cy - 26, 48, 64, 8, (uint32_t)0xA8DBFF); // cup
  g.fillRoundRect(x - 18, cy - 2, 36, 34, 6, (uint32_t)0x2F80ED);  // water
  eyes(g, x, cy + 8, 8, 2);
  drawSmile(g, x, cy + 11, 7, col::WHITE);
  // Happy water drop
  int dx = cx + 32, dy = cy - 8;
  g.fillTriangle(dx - 12, dy + 2, dx + 12, dy + 2, dx, dy - 22, (uint32_t)0x4FC3F7);
  g.fillCircle(dx, dy + 6, 13, (uint32_t)0x4FC3F7);
  eyes(g, dx, dy + 4, 5, 2);
  drawSmile(g, dx, dy + 6, 5, col::INK);
}

void iconBooks(M5Canvas& g, int cx, int cy) {
  auto book = [&](int x, int y, uint32_t cover) {
    g.fillRoundRect(x - 40, y - 11, 80, 22, 4, cover);
    g.fillRect(x - 32, y - 7, 68, 14, (uint32_t)0xFFF1C9);
    for (int k = -4; k <= 4; k += 4) g.drawFastHLine(x - 30, y + k, 64, (uint32_t)0xD9C9A3);
  };
  book(cx, cy + 26, 0x43A047);
  book(cx - 4, cy + 4, 0x8E6CCF);
  // "2" badge: two books tonight.
  g.fillCircle(cx + 20, cy - 30, 18, col::YELLOW);
  g.setFont(&fonts::FreeSansBold12pt7b);
  g.setTextDatum(middle_center);
  g.setTextColor(col::NAVY);
  g.drawString("2", cx + 20, cy - 29);
  fillStar(g, cx - 30, cy - 30, 7, (uint32_t)0x4DA3FF);
}

void iconHugs(M5Canvas& g, int cx, int cy) {
  auto heart = [&](int x, int y, int r, uint32_t color) {
    int o = r * 7 / 10;
    g.fillCircle(x - o, y - r / 3, r, color);
    g.fillCircle(x + o, y - r / 3, r, color);
    g.fillTriangle(x - o - r * 9 / 10, y + r / 6, x + o + r * 9 / 10, y + r / 6, x, y + r * 17 / 10, color);
  };
  heart(cx - 6, cy + 2, 22, kRed);
  eyes(g, cx - 6, cy - 4, 9);
  drawSmile(g, cx - 6, cy, 9, col::INK);
  heart(cx + 32, cy - 30, 9, kPink);
  heart(cx - 38, cy - 34, 6, kPink);
}

void iconStuffie(M5Canvas& g, int cx, int cy) {
  int hy = cy - 10;
  g.fillCircle(cx - 22, hy - 22, 11, kBrown);
  g.fillCircle(cx + 22, hy - 22, 11, kBrown);
  g.fillCircle(cx - 22, hy - 22, 5, kTan);
  g.fillCircle(cx + 22, hy - 22, 5, kTan);
  g.fillCircle(cx, hy, 28, kBrown);
  g.fillEllipse(cx, hy + 10, 13, 10, kTan);
  g.fillEllipse(cx, hy + 5, 5, 4, kDark);
  sleepyEye(g, cx - 11, hy - 8, 5);
  sleepyEye(g, cx + 11, hy - 8, 5);
  // Blanket with stars and paws on top
  g.fillRoundRect(cx - 46, cy + 14, 92, 34, 8, kBlue);
  for (int x : {-30, 0, 30}) fillStar(g, cx + x, cy + 34, 5, col::YELLOW);
  g.fillEllipse(cx - 16, cy + 16, 9, 6, kBrown);
  g.fillEllipse(cx + 16, cy + 16, 9, 6, kBrown);
}

void drawMoon(M5Canvas& g, int cx, int cy, int r, uint32_t cutColor) {
  g.fillCircle(cx, cy, r, col::YELLOW);
  g.fillCircle(cx + r / 2, cy - r / 3, r * 9 / 10, cutColor);
  sleepyEye(g, cx - r / 2, cy + r / 8, r / 6 + 2);
  g.fillCircle(cx - r * 7 / 10, cy + r / 2, r / 8 + 1, (uint32_t)0xFFB3A7);  // rosy cheek
}

void iconLightsOut(M5Canvas& g, int cx, int cy) {
  // Night-sky disc so the moon and stars glow even on the paper panel.
  g.fillCircle(cx, cy, 48, (uint32_t)0x1B2B6B);
  drawMoon(g, cx - 6, cy + 4, 30, 0x1B2B6B);
  fillStar(g, cx + 26, cy - 26, 7, col::YELLOW);
  fillStar(g, cx + 32, cy + 12, 5, col::YELLOW);
  fillStar(g, cx + 6, cy + 34, 4, col::YELLOW);
  fillStar(g, cx - 24, cy - 32, 4, col::YELLOW);
}
