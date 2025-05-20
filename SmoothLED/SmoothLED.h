#ifndef SmoothLED_H
#define SmoothLED_H

#include <Arduino.h>

class SmoothLED {
  public:
    SmoothLED(uint8_t pwmPin,
            uint32_t rampUpTime, uint32_t holdTime, uint32_t rampDownTime);

    void begin();
    void update();
    void trigger();  // Call this to restart or re-enter the cycle

void setRampTimes(uint32_t newRampUp, uint32_t newHold, uint32_t newRampDown);



  private:
    uint8_t pwmPin;

    uint32_t rampUpTime;
    uint32_t holdTime;
    uint32_t rampDownTime;

    bool triggerFlag = false;
    uint32_t ledStartMillis = millis();

    float getRampValue(float target, uint32_t startMillis, uint32_t totalTime, uint32_t currentMillis, int motionLaw, bool rampUp);
};

#endif
