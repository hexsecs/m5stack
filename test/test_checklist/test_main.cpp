#include <unity.h>

#include "Checklist.h"

void setUp() {}
void tearDown() {}

void test_complete_all_steps() {
  Checklist c(3);
  TEST_ASSERT_FALSE(c.finished());
  c.complete();
  c.complete();
  c.complete();
  TEST_ASSERT_TRUE(c.finished());
  TEST_ASSERT_TRUE(c.isDone(0) && c.isDone(1) && c.isDone(2));
  c.complete();  // extra presses are harmless
  TEST_ASSERT_EQUAL_UINT8(3, c.current());
}

void test_skip_and_back() {
  Checklist c(4);
  c.complete();
  c.skip();
  TEST_ASSERT_TRUE(c.isSkipped(1));
  TEST_ASSERT_FALSE(c.isDone(1));
  c.back();
  TEST_ASSERT_EQUAL_UINT8(1, c.current());
  TEST_ASSERT_FALSE(c.isSkipped(1));
  c.back();
  c.back();  // can't go below zero
  TEST_ASSERT_EQUAL_UINT8(0, c.current());
  TEST_ASSERT_FALSE(c.isDone(0));
}

void test_save_restore() {
  Checklist a(10);
  a.complete();
  a.complete();
  a.complete();
  a.skip();
  a.complete();
  Checklist b(10);
  b.restore(a.save());
  TEST_ASSERT_EQUAL_UINT8(5, b.current());
  TEST_ASSERT_TRUE(b.isDone(2));
  TEST_ASSERT_TRUE(b.isSkipped(3));
  TEST_ASSERT_TRUE(b.isDone(4));
  TEST_ASSERT_FALSE(b.isDone(5));
}

void test_restore_garbage_resets() {
  Checklist c(10);
  c.restore(0x1F);  // current=31 > count
  TEST_ASSERT_EQUAL_UINT8(0, c.current());
}

void test_star_chart_week() {
  StarChart s(7);
  for (int i = 0; i < 6; ++i) TEST_ASSERT_FALSE(s.addStar());
  TEST_ASSERT_TRUE(s.addStar());
  TEST_ASSERT_EQUAL_UINT8(7, s.stars());
  TEST_ASSERT_FALSE(s.addStar());  // new week starts automatically
  TEST_ASSERT_EQUAL_UINT8(1, s.stars());
}

void test_star_chart_bad_saved_value() {
  StarChart s(7, 200);
  TEST_ASSERT_EQUAL_UINT8(0, s.stars());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_complete_all_steps);
  RUN_TEST(test_skip_and_back);
  RUN_TEST(test_save_restore);
  RUN_TEST(test_restore_garbage_resets);
  RUN_TEST(test_star_chart_week);
  RUN_TEST(test_star_chart_bad_saved_value);
  return UNITY_END();
}
