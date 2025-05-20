#include "SmoothLED.h"

SmoothLED::SmoothLED(uint8_t pwmPin,
                 uint32_t rampUp, uint32_t hold, uint32_t rampDown)
  : pwmPin(pwmPin),
    rampUpTime(rampUp), holdTime(hold), rampDownTime(rampDown) {}

void SmoothLED::begin() {
  ledStartMillis = millis();
  pinMode(pwmPin, OUTPUT);
}

void SmoothLED::trigger() {
  triggerFlag = true;
}

void SmoothLED::setRampTimes(uint32_t newRampUp, uint32_t newHold, uint32_t newRampDown) {
  rampUpTime = newRampUp;
  holdTime = newHold;
  rampDownTime = newRampDown;
}


void SmoothLED::update() {
  uint32_t currentMillis = millis();
  uint32_t elapsed = currentMillis - ledStartMillis;
  float duty = 0;

  if (triggerFlag) {
    triggerFlag = false;

    // If in hold phase, extend hold
    if (elapsed >= rampUpTime && elapsed < (rampUpTime + holdTime + rampDownTime)) {
      ledStartMillis = currentMillis - rampUpTime;  // Stay in hold phase
    } else if (elapsed < rampUpTime){// continue as usual if ramping up
    }
    else {
      ledStartMillis = currentMillis;
    }
  }

  if (elapsed < rampUpTime) {
    duty = getRampValue(256.0, ledStartMillis, rampUpTime, currentMillis, 1, true);
  } else if (elapsed < (rampUpTime + holdTime)) {
    duty = 256.0;
  } else if (elapsed < (rampUpTime + holdTime + rampDownTime)) {
    duty = getRampValue(256.0, ledStartMillis + rampUpTime + holdTime, rampDownTime, currentMillis, 1, false);
  } else {
    duty = 0;
  }

  analogWrite(pwmPin, (int)duty);
}

float SmoothLED::getRampValue(float target, uint32_t start, uint32_t duration, uint32_t now, int law, bool rampUp) {
  if ((now - start > duration) && rampUp) return target;
  if ((now < start) && !rampUp) return target;

  float tau = (float)(now - start) / duration;
  tau = constrain(tau, 0.0, 1.0);

  float s;
  switch (law) {
    case 1:
      s = (tau <= 0.5f) ? 2 * pow(tau, 2) : -2 * pow(tau, 2) + 4 * tau - 1;
      break;
    case 2:
      s = 3 * pow(tau, 2) - 2 * pow(tau, 3);
      break;
    case 5:
      s = 6 * pow(tau, 5) - 15 * pow(tau, 4) + 10 * pow(tau, 3);
      break;
    case 7:
      s = -20 * pow(tau, 7) + 70 * pow(tau, 6) - 84 * pow(tau, 5) + 35 * pow(tau, 4);
      break;
    case 0:
    default:
      s = tau;
      break;
  }

  return rampUp ? s * target : (1 - s) * target;
}
