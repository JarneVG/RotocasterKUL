
void handleHomeInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  // Check only on the falling edge of CLK
  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check the buttons after a certain debounce time

    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
        homePointer = (homePointer + 1) % totalHomeOptions;
      } else {  // Counterclockwise
        homePointer = (homePointer - 1 + totalHomeOptions) % totalHomeOptions;
      }
    }


    if ((encoderButtonPressed) && (transitionComplete)) {  // Select option
      processButtonPress();

      if (homePointer == 0) {  // Quick start
        changeToState(SENSOR_SCREEN);
        sensorScreenPointer = 0;
      }

      if (homePointer == 1) {  // Quick start
        changeToState(PREHEAT);
        preheatPointer = 1;  // zero is for the back button
      }

      if (homePointer == 2) {  // Quick start
        changeToState(QUICK_START);
        quickStartPointer = 1;  // zero is for the back button
      }
      if (homePointer == 3) {  // Preset select
        changeToState(PRESET_SELECT);
        if (flashData.presetCount == 0) {
          presetSelectPointer = 1;
        } else {
          presetSelectPointer = 2;  //zero is for back button, one is for + button
        }
      }
    }
    //Serial.println(homePointer);
  }
}


void drawHomeScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_6x10_tf);
      // Always draw the frames




      // Sensor information box
      u8g2.setDrawColor(1);
      if (homePointer % totalHomeOptions == 0) {
        u8g2.drawRBox(0, 0, 62, 30, 5);
        u8g2.setDrawColor(0);  // Inverted text
      } else {
        u8g2.setDrawColor(1);
      }

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(20, 12);
      u8g2.print((int)averagedThermistorTempValue);
      u8g2.write(0xB0);  // Unicode for degree symbol
      u8g2.print("C");
      u8g2.setCursor(15, 23);
      if (doorState) {
        drawTextCenterOutline("Open", 32, 23);
      } else {
        drawTextCenterOutline("Closed", 32, 23);
      }


      // Preheat
      u8g2.setDrawColor(1);
      if (homePointer % totalHomeOptions == 1) {
        u8g2.drawRBox(66, 0, 62, 30, 5);
        u8g2.setDrawColor(0);  // Inverted text
      } else {
        u8g2.setDrawColor(1);
      }

      u8g2.setFont(u8g2_font_6x10_tf);

      if (preheatEnabledFlag) {
        u8g2.setCursor(68, 13);
        u8g2.print("Preheating");
        u8g2.setCursor(70, 23);
        u8g2.print((int)averagedThermistorTempValue);
        u8g2.print("C");
        u8g2.print(" / ");
        u8g2.print(preheatTargetTemp);
        u8g2.print("C");

#define TOTAL_HEAT_LENGTH 58
#define HEAT_STRIP_X_START 69
#define HEAT_STRIP_Y_START 28

        float actual_length = TOTAL_HEAT_LENGTH * pidOutput / windowSize;
        u8g2.drawHLine(HEAT_STRIP_X_START, HEAT_STRIP_Y_START, actual_length);
      } else {

        u8g2.setCursor(79, 12);
        u8g2.print("Enable");
        u8g2.setCursor(77, 23);
        u8g2.print("Preheat");
      }







      // Quick start
      if (homePointer % totalHomeOptions == 2) {
        u8g2.drawRBox(0, 32, 62, 30, 5);  // Fill box
        u8g2.setDrawColor(0);             // Inverted text
      } else {
        u8g2.setDrawColor(1);
      }
      u8g2.setCursor(18, 45);
      u8g2.print("Quick");
      u8g2.setCursor(18, 55);
      u8g2.print("Start");



      // Select Preset Box
      if (homePointer % totalHomeOptions == 3) {
        u8g2.drawRBox(66, 32, 62, 30, 5);  // Fill box
        u8g2.setDrawColor(0);              // Inverted text
      } else {
        u8g2.setDrawColor(1);
      }


      u8g2.setCursor(80, 45);
      u8g2.print("Select");
      u8g2.setCursor(80, 55);
      u8g2.print("Preset");
      u8g2.setDrawColor(1);  // Inverted text


      u8g2.drawRFrame(0, 0, 62, 30, 5);    //Sensors
      u8g2.drawRFrame(66, 0, 62, 30, 5);   // Preheating
      u8g2.drawRFrame(0, 32, 62, 30, 5);   // Always draw the frame
      u8g2.drawRFrame(66, 32, 62, 30, 5);  // Always draw the frame

    } while (u8g2.nextPage());
  }
}