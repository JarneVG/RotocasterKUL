#include "SmoothLED.h"

#define BUTTON_PIN 20
#define PWM_PIN    25

SmoothLED smoothLed(PWM_PIN, 1000, 2000, 500);

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  smoothLed.begin();
}

void loop() {
  // Manual control: detect press and trigger ramp LED
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    smoothLed.trigger();  // Re-trigger on button press
  }
  smoothLed.update();
}
