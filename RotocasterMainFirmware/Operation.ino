void handleOperationInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check the buttons after a certain debounce time

    // Check only on the falling edge of CLK
    if (operationScreenExtraState == OPERATION_NORMAL || operationScreenExtraState == OPERATION_BACK) {
      if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
        processButtonScroll();

        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          if (operationPointer != totalOperationOptions - 1) {
            operationPointer = (operationPointer + 1) % totalOperationOptions;
          }
          if (operationScreenExtraState == OPERATION_BACK) {
            operationPointer = 1;
          }
        } else {
          if (operationPointer != 0) {  // Counterclockwise
            operationPointer = (operationPointer - 1 + totalOperationOptions) % totalOperationOptions;
          }
        }
      }
    }

    if ((encoderButtonPressed) && (transitionComplete)) {  // Select option
      processButtonPress();

      if (operationScreenExtraState == OPERATION_NORMAL) {
        if (operationPointer == 0) {
          operationScreenExtraState = OPERATION_BACK;
          return;
        }
        if (operationPointer == 1) {
          operationScreenExtraState = OPERATION_MOTOR_1;
          return;
        }
        if (operationPointer == 2) {
          operationScreenExtraState = OPERATION_MOTOR_2;
          return;
        }
        if (operationPointer == 3) {
          operationScreenExtraState = OPERATION_TEMPERATURE;
          return;
        }
        if (operationPointer == 4) {
          operationScreenExtraState = OPERATION_INFO;
          return;
        }

      } else if (operationScreenExtraState == OPERATION_BACK) {
        operationScreenExtraState = OPERATION_NORMAL;
        if (operationPointer == 0) {
          operationScreenExtraState = OPERATION_NORMAL;
        } else if (operationPointer == 1) {
          // reset the rate limited values
          limitedInnerSpeedSetpoint = 0;
          limitedOuterSpeedSetpoint = 0;
          if (!preheatEnabledFlag) {
            limitedTargetTemperatureSetpoint = thermistorTempValue;
          }

          operationPointer = 0;
          changeToState(HOME);
        }
      } else if (operationScreenExtraState == OPERATION_MOTOR_1) {
        operationScreenExtraState = OPERATION_NORMAL;
      } else if (operationScreenExtraState == OPERATION_MOTOR_2) {
        operationScreenExtraState = OPERATION_NORMAL;
      } else if (operationScreenExtraState == OPERATION_TEMPERATURE) {
        operationScreenExtraState = OPERATION_NORMAL;

      } else if (operationScreenExtraState == OPERATION_INFO) {
        operationScreenExtraState = OPERATION_NORMAL;
      }
      processButtonPress();
    }
  }

  uint32_t remainingMillis = totalOperationMillis - (currentMillis - operationStartMillis);

  if (remainingMillis < 500) {
    int length = sizeof(noteDurations) / sizeof(int);
    buzzer.playMelody(melody, noteDurations, length);
    changeToState(FINISHED_OPERATION);
    screenRefreshFlag = true;  // Ensure it gets drawn at least once
  }
}





void drawOperationScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;

    u8g2.firstPage();
    do {
      // Back button
      if (operationPointer == 0) {
        drawBackButtonAndHLine(1);
      } else {
        drawBackButtonAndHLine(0);
      }

      u8g2.setFont(u8g2_font_9x18_tf);
      u8g2.setCursor(28, 10);
      u8g2.print("Operation");
      u8g2.setFont(u8g2_font_6x10_tf);


      // Draw Progress Bar
      drawProgressBar(progressBarCurrentPixels);

      // Draw time left
      drawTimeLeft(operationPointer);

      // Draw Animations
      drawAnimations(bitmapAnimationCounter, operationPointer, operationScreenExtraState != OPERATION_NORMAL);


      // Draw PresetInfo
      u8g2.drawRFrame(75, 16, 53, 21, 3);
      if (operationPointer == 4) {
        u8g2.drawRBox(75, 16, 53, 21, 3);
        u8g2.setDrawColor(0);
      } else {
        u8g2.setDrawColor(1);
      }

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(84, 26);
      u8g2.print("Preset");
      u8g2.setCursor(89, 34);
      u8g2.print("Info");
      u8g2.setDrawColor(1);



      u8g2.setFont(u8g2_font_6x10_tf);
      drawTextCenterOutline("In.", 10, 46);
      drawTextCenterOutline("Out.", 35, 46);
      String temperature = String((int)round(averagedThermistorTempValue)) + "/";
      drawTextCenterOutline(temperature, 60, 46);  // Draw actual temperature here

      drawTextCenterOutline(String((int)limitedInnerSpeedSetpoint), 10, 56);
      drawTextCenterOutline(String((int)limitedOuterSpeedSetpoint), 35, 56);
      drawTextCenterOutline(String((int)limitedTargetTemperatureSetpoint) + "C", 60, 56);


      if (operationScreenExtraState == OPERATION_BACK) {
        drawBackButtonAndHLine(1);
        const int baseWidth = 107;
        const int baseHeight = 46;
        const int centerX = 64;
        const int centerY = 30;
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth + 2, baseHeight + 2, 5, 1);
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth - 4, baseHeight - 4, 5, 0);
        u8g2.setDrawColor(1);  // White text
        u8g2.setCursor(16, 19);
        u8g2.print("Go back and");
        u8g2.setCursor(16, 29);
        u8g2.print("stop operation?");


        const int offCenterX = 25;
        const int offCenterY = 10;

        drawCenteredRoundedRectangle(centerX - offCenterX, centerY + offCenterY, baseWidth / 2 - 6, baseHeight / 2 - 7, 5, 0);
        drawCenteredRoundedRectangle(centerX + offCenterX, centerY + offCenterY, baseWidth / 2 - 6, baseHeight / 2 - 7, 5, 0);

        if (operationPointer == 0) {  // "No"
          drawCenteredRoundedRectangle(centerX - offCenterX, centerY + offCenterY, baseWidth / 2 - 6, baseHeight / 2 - 7, 5, 2);
          u8g2.setDrawColor(0);  // Inverted text
          drawTextCenterOutline("No", centerX - offCenterX, centerY + offCenterY + 3);
          u8g2.setDrawColor(1);
          drawTextCenterOutline("Yes", centerX + offCenterX, centerY + offCenterY + 3);


        } else if (operationPointer == 1) {  // "Yes"
          drawCenteredRoundedRectangle(centerX + offCenterX, centerY + offCenterY, baseWidth / 2 - 6, baseHeight / 2 - 7, 5, 2);
          u8g2.setDrawColor(0);  // Inverted text
          drawTextCenterOutline("Yes", centerX + offCenterX, centerY + offCenterY + 3);
          u8g2.setDrawColor(1);
          drawTextCenterOutline("No", centerX - offCenterX, centerY + offCenterY + 3);
        }
      } else if (operationScreenExtraState == OPERATION_MOTOR_1) {
        const int baseWidth = 110;
        const int baseHeight = 58;
        const int centerX = 64;
        const int centerY = 27;
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth + 2, baseHeight + 2, 5, 1);
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth - 4, baseHeight - 4, 5, 0);
        drawCloseButton(108, 8, 10, 10);
        drawTextCenterOutlineAndUnderline("Info Motor 1", 64, 11);

        const int startInfoTextY = 22;
        const int startInfoTextX = 14;

        u8g2.setCursor(startInfoTextX, startInfoTextY);
        u8g2.print("DPS frame: ");
        u8g2.print((int)limitedInnerSpeedSetpoint);

        u8g2.setCursor(startInfoTextX, startInfoTextY + 10);
        u8g2.print("Errors: ");
        u8g2.print("NYI");

        u8g2.setCursor(startInfoTextX, startInfoTextY + 20);
        u8g2.print("Microsteps: ");
        u8g2.print(MICROSTEP_1);

      } else if (operationScreenExtraState == OPERATION_MOTOR_2) {
        const int baseWidth = 110;
        const int baseHeight = 58;
        const int centerX = 64;
        const int centerY = 27;
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth + 2, baseHeight + 2, 5, 1);
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth - 4, baseHeight - 4, 5, 0);
        drawCloseButton(108, 8, 10, 10);
        drawTextCenterOutlineAndUnderline("Info Motor 2", 64, 11);

        const int startInfoTextY = 22;
        const int startInfoTextX = 14;

        u8g2.setCursor(startInfoTextX, startInfoTextY);
        u8g2.print("DPS frame: ");
        u8g2.print((int)limitedOuterSpeedSetpoint);

        u8g2.setCursor(startInfoTextX, startInfoTextY + 10);
        u8g2.print("Errors: ");
        u8g2.print("NYI");

        u8g2.setCursor(startInfoTextX, startInfoTextY + 20);
        u8g2.print("Microsteps: ");
        u8g2.print(MICROSTEP_2);

      } else if (operationScreenExtraState == OPERATION_TEMPERATURE) {
        const int baseWidth = 110;
        const int baseHeight = 58;
        const int centerX = 64;
        const int centerY = 27;
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth + 2, baseHeight + 2, 5, 1);
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth - 4, baseHeight - 4, 5, 0);
        drawCloseButton(108, 8, 10, 10);
        drawTextCenterOutlineAndUnderline("Info Temp", 64, 11);

        const int startInfoTextY = 22;
        const int startInfoTextX = 14;

        u8g2.setCursor(startInfoTextX, startInfoTextY);
        u8g2.print("Fan: ");
        u8g2.print(heaterFanState * 100);
        u8g2.print("%");

        u8g2.setCursor(startInfoTextX, startInfoTextY + 10);
        u8g2.print("PTC heater: ");
        u8g2.print((int)(pidOutput / windowSize * 100));
        u8g2.print("%");

        u8g2.setCursor(startInfoTextX, startInfoTextY + 20);
        u8g2.print("Temperature: ");
        u8g2.print((int)(round(averagedThermistorTempValue)));
        u8g2.print("C");

      } else if (operationScreenExtraState == OPERATION_INFO) {
        const int baseWidth = 114;
        const int baseHeight = 59;
        const int centerX = 64;
        const int centerY = 27;
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth + 2, baseHeight + 2, 5, 1);
        drawCenteredRoundedRectangle(centerX, centerY, baseWidth - 4, baseHeight - 4, 5, 0);
        drawCloseButton(110, 8, 10, 10);

        const int startInfoTextX = 12;
        const int startInfoTextY = 11;

        drawUnderlinedText(startInfoTextX, startInfoTextY, stringInput);

        u8g2.setCursor(startInfoTextX, startInfoTextY + 10);
        u8g2.print("Time: ");
        u8g2.print(operationSettingValues[4]);
        u8g2.print("h, ");
        u8g2.print(operationSettingValues[5]);
        u8g2.print("min");


        u8g2.setCursor(startInfoTextX, startInfoTextY + 20);
        u8g2.print("M inn: ");
        if (operationSettingValues[0] == operationSettingValues[1]) {
          u8g2.print(operationSettingValues[0]);
        } else {
          u8g2.print(operationSettingValues[0]);
          u8g2.print("-");
          u8g2.print(operationSettingValues[1]);
        }
        u8g2.print(" dps");


        u8g2.setCursor(startInfoTextX, startInfoTextY + 30);
        u8g2.print("M out: ");
        if (operationSettingValues[2] == operationSettingValues[3]) {
          u8g2.print(operationSettingValues[2]);
        } else {
          u8g2.print(operationSettingValues[2]);
          u8g2.print("-");
          u8g2.print(operationSettingValues[3]);
        }
        u8g2.print(" dps");

        u8g2.setCursor(startInfoTextX, startInfoTextY + 40);
        u8g2.print("Temp: ");
        if (operationSettingValues[6] == operationSettingValues[7]) {
          u8g2.print(operationSettingValues[6]);
        } else {
          u8g2.print(operationSettingValues[6]);
          u8g2.print("-");
          u8g2.print(operationSettingValues[7]);
        }
        u8g2.print("C");
      }
    } while (u8g2.nextPage());
  }
}

void refreshOperationAnimationCalculations(uint32_t samplePeriod) {
  if (currentMillis - lastRefreshMillis > samplePeriod) {
    // Set refresh flag
    screenRefreshFlag = true;
    lastRefreshMillis = currentMillis;

    // calculate progress bar
    float progressBarPercentage = (float)(currentMillis - operationStartMillis) / (float)totalOperationMillis;
    progressBarCurrentPixels = (uint8_t)(progressBarPercentage * progressBarTotalLength);
    progressBarCurrentPixels = constrain(progressBarCurrentPixels, 0, progressBarTotalLength);



    // Calculate millis left
    uint32_t remainingMillis = totalOperationMillis - (currentMillis - operationStartMillis);
    remainingHours = remainingMillis / (1000 * 60 * 60);                    // Convert millis to hours
    remainingMinutes = (remainingMillis % (1000 * 60 * 60)) / (1000 * 60);  // Convert remaining millis to minutes
    remainingSeconds = (remainingMillis % (1000 * 60)) / 1000;              // Convert remaining millis to seconds


    // Calculate which picture to animate
    bitmapAnimationCounter = (bitmapAnimationCounter + 1) % 3;
  }
}


void drawProgressBar(uint8_t progressPixels) {
  u8g2.drawHLine(0, 63, 128);                                          // Horizontal line
  u8g2.drawHLine(0, 60 - progressBarHeight, 128);                      // Horizontal line
  u8g2.drawVLine(0, 61 - progressBarHeight, 3 + progressBarHeight);    // Vertical line
  u8g2.drawVLine(127, 61 - progressBarHeight, 3 + progressBarHeight);  // Vertical line

  u8g2.drawBox(2, 62 - progressBarHeight, progressPixels, progressBarHeight);
  u8g2.drawHLine(2, 62 - progressBarHeight, progressPixels);  // Horizontal line
}

void drawTimeLeft(uint8_t pointer) {

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(92, 58 - progressBarHeight);
  String timeLeftString = String(remainingHours) + ":" + String(remainingMinutes) + ":" + String(remainingSeconds);
  int textWidth = u8g2.getStrWidth(timeLeftString.c_str());

  const int startTimeBoxX = 75;
  const int startTimeBoxY = 39;
  const int startTimeWidth = 53;
  const int startTimeHeight = 18;

  // Draw box for remaining time
  u8g2.drawRFrame(startTimeBoxX, startTimeBoxY, startTimeWidth, startTimeHeight, 3);
  if (operationPointer == 5) {
    u8g2.drawRBox(startTimeBoxX, startTimeBoxY, startTimeWidth, startTimeHeight, 3);
    u8g2.setDrawColor(0);
  } else {
    u8g2.setDrawColor(1);
  }

  drawTextCenterOutline(timeLeftString, startTimeBoxX + startTimeWidth / 2, startTimeBoxY + startTimeHeight / 2 + 3);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);
}


void drawAnimations(uint8_t animationCounter, uint8_t pointer, uint8_t disableFlag) {
  if (animationCounter == 0) {
    u8g2.drawXBMP(bitmapStartX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_30deg);
    u8g2.drawXBMP(bitmapStartX + bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_30deg);
    u8g2.drawXBMP(bitmapStartX + 2 * bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Fire21x21_right);
  } else if (animationCounter == 1) {
    u8g2.drawXBMP(bitmapStartX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_60deg);
    u8g2.drawXBMP(bitmapStartX + bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_60deg);
    u8g2.drawXBMP(bitmapStartX + 2 * bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Fire21x21_leftmiddle);
  } else if (animationCounter == 2) {
    u8g2.drawXBMP(bitmapStartX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_90deg);
    u8g2.drawXBMP(bitmapStartX + bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_90deg);
    u8g2.drawXBMP(bitmapStartX + 2 * bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Fire21x21_left);
  }


  if (!disableFlag) {
    // INVERT COLORS WHEN SELECTED
    if (pointer == 1) {  // 1st motor is selected
      u8g2.setDrawColor(0);
      if (animationCounter == 0) u8g2.drawXBMP(bitmapStartX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_30deg);
      else if (animationCounter == 1) u8g2.drawXBMP(bitmapStartX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_60deg);
      else if (animationCounter == 2) u8g2.drawXBMP(bitmapStartX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_90deg);
      u8g2.setDrawColor(1);
    }

    if (pointer == 2) {  // 2nd motor is selected
      u8g2.setDrawColor(0);
      if (animationCounter == 0) u8g2.drawXBMP(bitmapStartX + bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_30deg);
      else if (animationCounter == 1) u8g2.drawXBMP(bitmapStartX + bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_60deg);
      else if (animationCounter == 2) u8g2.drawXBMP(bitmapStartX + bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Motor22x22_90deg);
      u8g2.setDrawColor(1);
    }

    if (pointer == 3) {      // fire is selected
      u8g2.setDrawColor(0);  // inverse
      if (animationCounter == 0) u8g2.drawXBMP(bitmapStartX + 2 * bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Fire21x21_right);
      else if (animationCounter == 1) u8g2.drawXBMP(bitmapStartX + 2 * bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Fire21x21_leftmiddle);
      else if (animationCounter == 2) u8g2.drawXBMP(bitmapStartX + 2 * bitmapSpacerX, bitmapStartY, bitmapWidth, bitmapHeight, bitmap_Fire21x21_left);
      u8g2.setDrawColor(1);
    }
  }
  u8g2.drawRFrame(bitmapStartX - 1 + (1 - 1) * bitmapSpacerX, bitmapStartY - 1, 21, 21, 3);
  u8g2.drawRFrame(bitmapStartX - 1 + (2 - 1) * bitmapSpacerX, bitmapStartY - 1, 21, 21, 3);
  u8g2.drawRFrame(bitmapStartX - 1 + (3 - 1) * bitmapSpacerX, bitmapStartY - 1, 21, 21, 3);
}






void makeOperationCalculations() {
  // Calculate target speeds and temperatures based on settings
  float ricoInner = (float)(operationSettingValues[1] - operationSettingValues[0]) / totalOperationMillis;
  float ricoOuter = (float)(operationSettingValues[3] - operationSettingValues[2]) / totalOperationMillis;
  float ricoTemp = (float)(operationSettingValues[7] - operationSettingValues[6]) / totalOperationMillis;

  currentTargetInnerSpeed = operationSettingValues[0] + ricoInner * (currentMillis - operationStartMillis);
  currentTargetOuterSpeed = operationSettingValues[2] + ricoOuter * (currentMillis - operationStartMillis);
  currentTargetTemperature = operationSettingValues[6] + ricoTemp * (currentMillis - operationStartMillis);



#define rampDuration 3000
  // Change targetspeeds in rampup
  // currentTargetInnerSpeed = getRampupRampdownSpeed(currentTargetInnerSpeed, operationStartMillis, rampDuration, currentMillis, 2, 5);
  // currentTargetOuterSpeed = getRampupRampdownSpeed(currentTargetOuterSpeed, operationStartMillis, rampDuration, currentMillis, 1, 7);

  // // Change targetspeeds in rampdown

  // currentTargetInnerSpeed = getRampupRampdownSpeed(currentTargetInnerSpeed, operationStartMillis + totalOperationMillis - rampDuration, rampDuration, currentMillis, 2, 0);
  // currentTargetOuterSpeed = getRampupRampdownSpeed(currentTargetOuterSpeed, operationStartMillis + totalOperationMillis - rampDuration, rampDuration, currentMillis, 1, 0);
}



float getRampupRampdownSpeed(float targetSpeed, uint32_t startMillis, uint32_t totalRampupMillis, uint32_t currentMillis, int motionLaw, bool rampUp) {
  if ((currentMillis - startMillis > totalRampupMillis) && rampUp) {
    return targetSpeed;
  }

  if ((currentMillis < startMillis) && !rampUp) {
    return targetSpeed;
  }

  float tau = (float)(currentMillis - startMillis) / totalRampupMillis;
  float s;

  // Motion laws
  if (motionLaw == 0) {
    s = tau;
  } else if (motionLaw == 1) {
    s = (tau <= 0.5) ? 2 * pow(tau, 2) : -2 * pow(tau, 2) + 4 * tau - 1;
  } else if (motionLaw == 2) {
    s = 3 * pow(tau, 2) - 2 * pow(tau, 3);
  } else if (motionLaw == 5) {
    s = 6 * pow(tau, 5) - 15 * pow(tau, 4) + 10 * pow(tau, 3);
  } else if (motionLaw == 7) {
    s = -20 * pow(tau, 7) + 70 * pow(tau, 6) - 84 * pow(tau, 5) + 35 * pow(tau, 4);
  } else {
    s = tau;  // Default case
  }

  if (rampUp) {
    return s * targetSpeed;
  } else {
    return (1 - s) * targetSpeed;
  }

  return 0.0f;  // This should never be reached, but ensures all paths return a value
}