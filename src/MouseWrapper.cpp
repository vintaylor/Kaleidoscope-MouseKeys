
// Mouse-related methods
//
//
#include "MouseWrapper.h"
#include "kaleidoscope/hid.h"

uint16_t MouseWrapper_::next_width;
uint16_t MouseWrapper_::next_height;
uint16_t MouseWrapper_::section_top;
uint16_t MouseWrapper_::section_left;
boolean MouseWrapper_::is_warping;

uint8_t MouseWrapper_::accelStep;

MouseWrapper_::MouseWrapper_(void) {
}

void MouseWrapper_::begin(void) {
  kaleidoscope::hid::initializeMouse();
  kaleidoscope::hid::initializeAbsoluteMouse();
}

void MouseWrapper_::pressButton(uint8_t button) {
  kaleidoscope::hid::pressMouseButtons(button);
  end_warping();
}

void MouseWrapper_::release_button(uint8_t button) {
  kaleidoscope::hid::releaseMouseButtons(button);
}

void MouseWrapper_::warp_jump(uint16_t left, uint16_t top, uint16_t height, uint16_t width) {
  uint16_t x_center = left + width / 2;
  uint16_t y_center = top + height / 2;
  kaleidoscope::hid::moveAbsoluteMouseTo(x_center, y_center, 0);
}

void MouseWrapper_::begin_warping() {
  section_left = WARP_ABS_LEFT;
  section_top = WARP_ABS_TOP;
  next_width = MAX_WARP_WIDTH;
  next_height = MAX_WARP_HEIGHT;
  is_warping = true;
}

void MouseWrapper_::end_warping() {
  is_warping = false;
}

void MouseWrapper_::warp(uint8_t warp_cmd) {
  if (is_warping == false) {
    begin_warping();
  }

  if (warp_cmd & WARP_END) {
    end_warping();
    return;
  }

  next_width = next_width / 2;
  next_height = next_height / 2;

  if (warp_cmd & WARP_UP) {
//    Serial.print(" - up ");
  } else if (warp_cmd & WARP_DOWN) {
//   Serial.print(" - down ");
    section_top  = section_top + next_height;
  }

  if (warp_cmd & WARP_LEFT) {
    //  Serial.print(" - left ");
  } else if (warp_cmd & WARP_RIGHT) {
    // Serial.print(" - right ");
    section_left  = section_left + next_width;
  }

  warp_jump(section_left, section_top, next_height, next_width);
}

// cubic wave function based on code from FastLED
// produces a shape similar to a sine curve from 0 to 255
// (slow growth at 0, fast growth in the middle, slow growth at 255)
// http://www.wolframalpha.com/input/?i=((3((x)**2)%2F256)+-+((2((x)(x)(x%2F256))%2F256)))+%2B+1
uint8_t MouseWrapper_::acceleration(uint8_t cycles) {
  uint16_t i = cycles;

  uint16_t ii = (i * i) >> 8;
  uint16_t iii = (ii * i) >> 8;

  i = ((3 * ii) - (2 * iii)) + 1;

  // Just in case (may go up to 256 at peak)
  if (i > 255) i = 255;

  return i;
}


void MouseWrapper_::move(int8_t x, int8_t y, uint8_t speedLimit) {
  int16_t moveX = 0;
  int16_t moveY = 0;
  static int8_t remainderX = 0;
  static int8_t remainderY = 0;
  if (x != 0) {
    moveX = remainderX + (x * acceleration(accelStep));
    if (moveX > (int16_t)speedLimit) moveX = speedLimit;
    else if (moveX < -(int16_t)speedLimit) moveX = -speedLimit;
  }
  if (y != 0) {
    moveY = remainderY + (y * acceleration(accelStep));
    if (moveY > (int16_t)speedLimit) moveY = speedLimit;
    else if (moveY < -(int16_t)speedLimit) moveY = -speedLimit;
  }

  end_warping();
  // move by whole pixels, not subpixels
  kaleidoscope::hid::moveMouse(moveX >> 4, moveY >> 4, 0);
  // save leftover subpixel movements for later
  remainderX = moveX & 0x0f;
  remainderY = moveY & 0x0f;
}

MouseWrapper_ MouseWrapper;
