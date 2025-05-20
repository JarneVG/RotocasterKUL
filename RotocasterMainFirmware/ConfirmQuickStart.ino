void handleConfirmQuickStartInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  // Check only on the falling edge of CLK
  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check the buttons after a certain debounce time

    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
        if (confirmQuickStartPointer != totalConfirmQuickStartOptions - 1) {
          confirmQuickStartPointer = (confirmQuickStartPointer + 1) % totalConfirmQuickStartOptions;
        }
      } else {  // Counterclockwise
        if (confirmQuickStartPointer != 0)
          confirmQuickStartPointer = (confirmQuickStartPointer - 1 + totalConfirmQuickStartOptions) % totalConfirmQuickStartOptions;
      }
    }


    if ((encoderButtonPressed) && (transitionComplete)) {  // Select option
      processButtonPress();

      if (confirmQuickStartPointer == 0) {  // Quick start
        changeToState(QUICK_START);
        quickStartPointer = 1;
      }
      if (confirmQuickStartPointer == 1) {  // Preset select

        // Store all quickstart settings in the actual settings string
        for (int i = 0; i < numberOfSettings; i++) {
          operationSettingValues[i] = quickStartSettingValues[i];
        }

        totalOperationMillis =(uint32_t)quickStartSettingValues[4] * 60 * 60 * 1000  // hours
                                      + (uint32_t)quickStartSettingValues[5] * 60 * 1000;    // minutes

        operationStartMillis = millis();
        previousRateLimitMillis = millis();
        currentMillis = millis();
        changeToState(OPERATION);
        transferLastUsedDataFromQuickStartToFlash();
      }
    }
    //Serial.println(confirmQuickStartPointer);
  }
}









void drawConfirmQuickStartScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_6x10_tf);

      // Quick Start Box
      u8g2.drawRFrame(0, 42, 62, 20, 5);   // Always draw the frame
      u8g2.drawRFrame(66, 42, 62, 20, 5);  // Always draw the frame

      if (confirmQuickStartPointer % totalConfirmQuickStartOptions == 0) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(1, 43, 60, 18);  // Fill box
        u8g2.setDrawColor(0);         // Inverted text
      } else {
        u8g2.setDrawColor(1);
      }

      u8g2.setCursor(10, 55);
      u8g2.print("Go Back");

      // Select Preset Box
      if (confirmQuickStartPointer % totalConfirmQuickStartOptions == 1) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(67, 43, 60, 18);  // Fill box
        u8g2.setDrawColor(0);          // Inverted text
      } else {
        u8g2.setDrawColor(1);
      }


      u8g2.setCursor(75, 55);
      u8g2.print("Continue");

      // Sensor information box (always white)
      u8g2.setDrawColor(1);

      u8g2.drawRFrame(0, 0, 128, 40, 5);

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(4, 13);
      u8g2.print("Continue?");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(4, 23);
      u8g2.print("This will take: ");
      u8g2.setCursor(4, 33);

      int hours = quickStartSettingValues[4];
      int mins = quickStartSettingValues[5];
      if (hours != 0) {
        u8g2.print(hours);
        u8g2.print("h and ");
      }
      u8g2.print(mins);
      u8g2.print("min");
    } while (u8g2.nextPage());
  }
}




void transferLastUsedDataFromQuickStartToFlash() {

  strncpy(lastUsedPresetData.name, String("LAST USED      ").c_str(), sizeof(lastUsedPresetData.name) - 1);
  lastUsedPresetData.name[sizeof(lastUsedPresetData.name) - 1] = '\0';  // Ensure null termination

  // Set the rest of the preset values as needed
  lastUsedPresetData.innerSpeedStart = quickStartSettingValues[0];
  lastUsedPresetData.innerSpeedEnd = quickStartSettingValues[1];
  lastUsedPresetData.outerSpeedStart = quickStartSettingValues[2];
  lastUsedPresetData.outerSpeedEnd = quickStartSettingValues[3];
  lastUsedPresetData.durationHr = quickStartSettingValues[4];
  lastUsedPresetData.durationMin = quickStartSettingValues[5];
  lastUsedPresetData.tempStart = quickStartSettingValues[6];
  lastUsedPresetData.tempEnd = quickStartSettingValues[7];

  //updatePreset(lastUsedPresetData, presetSelectPointer - 2);
  saveLastUsedPreset(lastUsedPresetData);
}