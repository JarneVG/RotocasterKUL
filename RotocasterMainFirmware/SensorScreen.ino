void handleSensorScreenInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);
  // Check only on the falling edge of CLK
  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check the buttons after a certain debounce time

    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
        if (sensorScreenPointer != totalSensorScreenOptions) {
          sensorScreenPointer = (sensorScreenPointer + 1) % totalSensorScreenOptions;
        }
      } else {  // Counterclockwise
        if (sensorScreenPointer != 0) {
          sensorScreenPointer = (sensorScreenPointer - 1 + totalSensorScreenOptions) % totalSensorScreenOptions;
        }
      }
    }

    if ((encoderButtonPressed) && (transitionComplete)) {  // Select option
      processButtonPress();
      changeToState(HOME);
    }
  }
  //Serial.println(homePointer);
}


void drawSensorScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_6x10_tf);
      const int linesPerScreen = 5;
      const int startingPosX = 2;
      const int startingPosY = 25;

      for (uint8_t i = 0; i < linesPerScreen; i++) {
        int index = sensorScreenPointer + i;
        if (index < totalSensorScreenOptions) {
          u8g2.setCursor(startingPosX, startingPosY + i * 10);  // 10 pixels per line
          u8g2.print(sensorNames[index]);
          u8g2.println(sensorValues[index]);
        }
      }

      drawBackButtonAndHLine(1);
      u8g2.setFont(u8g2_font_9x18_tf);
      u8g2.setCursor(28, 10);
      u8g2.print("Information");

      u8g2.setFont(u8g2_font_6x10_tf);
    } while (u8g2.nextPage());
  }
}


void updateSensorArray() {
  sensorValues[0] = String(thermistorTempValue, 0) + "C";
  sensorValues[1] = String(averagedThermistorTempValue, 0) + "C";

  if (doorState) {
    sensorValues[2] = "Open";
  } else {
    sensorValues[2] = "Closed";
  }


  sensorValues[9] = String(round(currentMillis / (1000 * 60 * 60)), 0);
  sensorValues[10] = String(round(currentMillis / (1000 * 60) - round(currentMillis / (1000 * 60 * 60)) * 60), 0);
  sensorValues[11] = String((currentMillis / 1000 - round(currentMillis / (1000 * 60)) * 60 - round(currentMillis / (1000 * 60 * 60)) * 60 * 60), 0);
}