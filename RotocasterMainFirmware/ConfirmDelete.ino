void handleConfirmDeleteInput() {
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

      if (confirmQuickStartPointer == 0) {  // Go back to info screen
        changeToState(PRESET_INFO);
        presetInfoPointer = totalPresetInfoOptions - 2;
      }
      if (confirmQuickStartPointer == 1) {  // Delete the preset
        deletePreset(presetSelectPointer - 1);
        readFlashData();
        changeToState(PRESET_SELECT);
        presetInfoPointer = 1;
        confirmQuickStartPointer = 0;
      }
    }
    //Serial.println(confirmQuickStartPointer);
  }
}









void drawConfirmDeleteScreen() {
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


      u8g2.setCursor(78, 55);
      u8g2.print("Delete");

      u8g2.setDrawColor(1);
      u8g2.drawRFrame(0, 0, 128, 40, 5);
      u8g2.drawRFrame(1, 1, 126, 38, 5);
      u8g2.drawRFrame(2, 2, 124, 36, 5);

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(6, 13);
      u8g2.print("Are you sure?");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(6, 23);
      u8g2.print("This will delete");
      u8g2.setCursor(6, 33);
      u8g2.print("the preset.");

    } while (u8g2.nextPage());
  }
}




