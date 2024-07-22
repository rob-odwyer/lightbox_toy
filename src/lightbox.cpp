#include <Arduino.h>
#include <FastLED.h>
#include <AbleButtons.h>
#include "snakes.h"

using Button = AblePullupClickerButton;
using ButtonList = AblePullupClickerButtonList;

void handleMovement(uint8_t currentX, uint8_t currentY, uint8_t targetX, uint8_t targetY);
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
// This is an index into the tail array indicating the current head
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

  uint8_t currentX = tail[headIndex].x;
  uint8_t currentY = tail[headIndex].y;
  uint8_t targetX = currentX;
  uint8_t targetY = currentY;
  bool moved = false;

  // Process button input and move head
  if (left.resetClicked())
  {
    if (targetX > 0)
    {
      moved = true;
      targetX = targetX - 1;
    }
  }
  if (right.resetClicked())
  {
    if (targetX < MATRIX_WIDTH - 1)
    {
      moved = true;
      targetX = targetX + 1;
    }
  }
  if (up.resetClicked())
  {
    if (targetY < MATRIX_HEIGHT - 1)
    {
      moved = true;
      targetY = targetY + 1;
    }
  }
  if (down.resetClicked())
  {
    if (targetY > 0)
    {
      moved = true;
      targetY = targetY - 1;
    }
  }

  if (moved)
  {
    handleMovement(currentX, currentY, targetX, targetY);
  }

  if (headIndex == (NUM_LEDS - 1))
  {
    // Moved off the last square, cue victory sequence
    headIndex = shrinkTailAnimation(tail, headIndex, leds, rainbow);

    // Skip rendering the last frame twice
    return;
  }

  FastLED.clear();
  renderTail(tail, headIndex, leds, rainbow);
  FastLED.show();
}

void handleMovement(uint8_t currentX, uint8_t currentY, uint8_t targetX, uint8_t targetY)
{
  uint64_t targetIndex = XY(targetX, targetY);

  // Check for moving back on neck
  if (headIndex > 0)
  {
    if (targetX == tail[headIndex - 1].x && targetY == tail[headIndex - 1].y)
    {
      uint64_t currentIndex = XY(currentX, currentY);

      // Mark the currently occupied square as unvisited
      visited &= ~(1ULL << currentIndex);

      // Retract the head to the previous position
      headIndex--;

      return;
    }
  }

  // Check if the square has been visited already and block movement if so
  if (visited & (1ULL << targetIndex))
  {
    return;
  }

  // Handle creating new tail segments by advancing the head
  // This shouldn't be allowed to wrap around, but it makes it more obvious when a bug has happened.
  headIndex = (headIndex + 1) % NUM_LEDS;
  tail[headIndex].x = targetX;
  tail[headIndex].y = targetY;

  // Mark the square as visited
  visited |= (1ULL << targetIndex);

  return;
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
  int dialValue = map(analogRead(DIAL_INPUT_PIN), 0, 1023, 0, 255);

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
      leds[XY(headX, headY)] = ColorFromPalette(palette, (map(tailIndex, 0, NUM_LEDS, 255, 0) + dialValue) % 255, 255);
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