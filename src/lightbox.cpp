#include <Arduino.h>
#include <FastLED.h>
#include <AbleButtons.h>
#include "snakes.h"

using Button = AblePullupClickerButton;
using ButtonList = AblePullupClickerButtonList;

uint16_t XY(uint8_t x, uint8_t y);
void renderTail(TailSegment *tail, int8_t headIndex, CRGB *leds, CRGBPalette16 &palette);
int8_t shrinkTailAnimation(TailSegment *tail, int8_t headIndex, CRGB *leds, CRGBPalette16 &palette);

// Arduino pins used for IO
#define BUTTON_UP_PIN 14       // PC0, purple wire
#define BUTTON_DOWN_PIN 15     // PC1, gray wire
#define BUTTON_LEFT_PIN 16     // PC2, black wire
#define BUTTON_RIGHT_PIN 17    // PC3, white wire
#define BUTTON_INTERACT_PIN 18 // PC4, brown wire

#define LED_MATRIX_DATA_PIN 2 // PD2

#define DIAL_INPUT_PIN 5 // PC5

// Fixed parameters related to the LED matrix
#define COLOR_ORDER GRB
#define CHIPSET WS2812
#define BRIGHTNESS 32
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
CRGBArray<NUM_LEDS> leds;

Button up(BUTTON_UP_PIN);
Button down(BUTTON_DOWN_PIN);
Button left(BUTTON_LEFT_PIN);
Button right(BUTTON_RIGHT_PIN);
Button interact(BUTTON_INTERACT_PIN);

Button *btns[] = {
    &up, &down, &left, &right, &interact};
ButtonList btnList(btns); // List of button to track input from together.

TailSegment tail[NUM_LEDS];
uint8_t headIndex = 0;

// Keep track of visited squares in a 64-bit bitmask
uint64_t visited = 0;

CRGBPalette16 rainbow;

void setup()
{
  // initialize tail array
  for (int i = 1; i < NUM_LEDS; i++)
  {
    tail[i] = TailSegment{0, 0};
  }

  // initialize LED matrix
  FastLED.addLeds<CHIPSET, LED_MATRIX_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();

  // initialize rainbow palette
  rainbow = RainbowColors_p;

  // get random index into HILBERT_CURVE and copy shifted curve into tail array
  randomSeed(analogRead(DIAL_INPUT_PIN));
  int curveStartIndex = random(0, HILBERT_CURVE_SIZE);
  for (int i = 0; i < HILBERT_CURVE_SIZE; i++)
  {
    tail[i] = HILBERT_CURVE[(curveStartIndex + i) % HILBERT_CURVE_SIZE];
    headIndex = i;
  }

  headIndex = shrinkTailAnimation(tail, headIndex, leds, rainbow);

  // Initialize all buttons
  btnList.begin();
}

void loop()
{
  btnList.handle();

  uint8_t headX = tail[headIndex].x;
  uint8_t headY = tail[headIndex].y;
  bool moved = false;

  // Process button input and move head
  if (left.resetClicked())
  {
    if (headX > 0)
    {
      moved = true;
      headX = headX - 1;
    }
  }
  if (right.resetClicked())
  {
    if (headX < MATRIX_WIDTH - 1)
    {
      moved = true;
      headX = headX + 1;
    }
  }
  if (up.resetClicked())
  {
    if (headY < MATRIX_HEIGHT - 1)
    {
      moved = true;
      headY = headY + 1;
    }
  }
  if (down.resetClicked())
  {
    if (headY > 0)
    {
      moved = true;
      headY = headY - 1;
    }
  }

  // Check if the square has been visited already and block movement if so
  uint64_t targetIndex = XY(headX, headY);
  if (visited & (1ULL << targetIndex))
  {
    moved = false;
  }

  // Handle creating new tail segments by advancing the head
  if (moved)
  {
    headIndex = (headIndex + 1) % NUM_LEDS;
    if (headIndex == (NUM_LEDS - 1))
    {
      // Moved off the last square, cue victory sequence
      headIndex = shrinkTailAnimation(tail, headIndex, leds, rainbow);
      return;
    }
    tail[headIndex].x = headX;
    tail[headIndex].y = headY;

    // Mark the square as visited
    visited |= (1ULL << targetIndex);
  }

  FastLED.clear();
  renderTail(tail, headIndex, leds, rainbow);
  FastLED.show();
}

// This function will return the right 'led index number' for
// a given set of X and Y coordinates on your matrix.
uint16_t XY(uint8_t x, uint8_t y)
{
  if (x < 0 || y < 0 || x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT)
  {
    return 0;
  }
  return (x * MATRIX_HEIGHT) + y;
}

void renderTail(TailSegment *tail, int8_t headIndex, CRGB *leds, CRGBPalette16 &palette)
{
  for (int8_t tailIndex = headIndex; tailIndex >= 0; tailIndex--)
  {
    uint8_t headX = tail[tailIndex].x;
    uint8_t headY = tail[tailIndex].y;

    if (tailIndex == headIndex)
    {
      leds[XY(headX, headY)] = CRGB::White;
    }
    else
    {
      leds[XY(headX, headY)] = ColorFromPalette(palette, map(tailIndex, 0, NUM_LEDS, 255, 0), 255);
    }
  }
}

int8_t shrinkTailAnimation(TailSegment *tail, int8_t headIndex, CRGB *leds, CRGBPalette16 &palette)
{
  for (; headIndex > 0; headIndex--)
  {
    FastLED.clear();
    renderTail(tail, headIndex, leds, palette);
    FastLED.show();
    delay(50);
  }
  return headIndex;
}