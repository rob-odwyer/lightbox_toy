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

uint8_t xPos = 3, yPos = 3;
CRGB color = CRGB::White;

void setup()
{
  FastLED.addLeds<CHIPSET, LED_MATRIX_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);

  btnList.begin();

  FastLED.clear();
  FastLED.setBrightness(BRIGHTNESS);
  leds.fill_rainbow(0, 5);
  FastLED.show();

  delay(500);
  FastLED.clear();

  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;

  leds[XY(3, 3)] = CRGB::White;
  leds[XY(4, 3)] = CRGB::Blue;
  leds[XY(3, 4)] = CRGB::Red;
  leds[XY(2, 3)] = CRGB::Green;
  leds[XY(3, 2)] = CRGB::Yellow;

  FastLED.show();
}

void loop()
{
  btnList.handle();

  if (btnList.anyClicked())
  {
    FastLED.clear();

    if (left.resetClicked())
    {
      color = CRGB::Green;
      xPos = (MATRIX_WIDTH + xPos - 1) % MATRIX_WIDTH;
    }
    if (right.resetClicked())
    {
      color = CRGB::Blue;
      xPos = (xPos + 1) % MATRIX_WIDTH;
    }
    if (up.resetClicked())
    {
      color = CRGB::Red;
      yPos = (yPos + 1) % MATRIX_HEIGHT;
    }
    if (down.resetClicked())
    {
      color = CRGB::Yellow;
      yPos = (MATRIX_HEIGHT + yPos - 1) % MATRIX_HEIGHT;
    }

    leds[XY(xPos, yPos)] = color;
    FastLED.show();
  }
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
