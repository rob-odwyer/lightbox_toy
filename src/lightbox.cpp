#include <Arduino.h>
#include <FastLED.h>
#include <AbleButtons.h>

using Button = AblePullupClickerButton;
using ButtonList = AblePullupClickerButtonList;

uint16_t XY(uint8_t x, uint8_t y);

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

struct TailSegment
{
  uint8_t x;
  uint8_t y;
};

TailSegment tail[NUM_LEDS];
uint8_t headIndex = 0;

CRGB color = CRGB::White;

void setup()
{
  // initialize LED matrix
  FastLED.addLeds<CHIPSET, LED_MATRIX_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);

  btnList.begin();

  FastLED.clear();
  FastLED.setBrightness(BRIGHTNESS);
  leds.fill_rainbow(0, 5);
  FastLED.show();
  delay(200);
  FastLED.clear();
  delay(200);

  // initialize tail array
  for (int i = 1; i < NUM_LEDS; i++)
  {
    tail[i] = TailSegment{0, 0};
  }
  tail[headIndex].x = 3;
  tail[headIndex].y = 3;
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
    moved = true;
    headX = (MATRIX_WIDTH + headX - 1) % MATRIX_WIDTH;
  }
  if (right.resetClicked())
  {
    moved = true;
    headX = (headX + 1) % MATRIX_WIDTH;
  }
  if (up.resetClicked())
  {
    moved = true;
    headY = (headY + 1) % MATRIX_HEIGHT;
  }
  if (down.resetClicked())
  {
    moved = true;
    headY = (MATRIX_HEIGHT + headY - 1) % MATRIX_HEIGHT;
  }

  // Handle creating new tail segments by advancing the head
  if (moved)
  {
    headIndex = (headIndex + 1) % NUM_LEDS;
    if (headIndex == 0)
    {
      // TODO: moved off the last square, cue victory sequence
    }
    tail[headIndex].x = headX;
    tail[headIndex].y = headY;
  }

  FastLED.clear();

  leds[XY(headX, headY)] = CRGB::White;

  // render the tail
  for (int8_t tailIndex = headIndex; tailIndex >= 0; tailIndex--)
  {
    headX = tail[tailIndex].x;
    headY = tail[tailIndex].y;

    leds[XY(headX, headY)] = tailIndex == headIndex ? CRGB::White : CRGB::Green;
  }

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
