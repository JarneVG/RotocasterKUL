

void handlePresetSelectInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check the buttons after a certain debounce time

    // Check only on the falling edge of CLK
    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
        if (presetSelectPointer != totalPresetSelectOptions - 1)
          presetSelectPointer = (presetSelectPointer + 1) % totalPresetSelectOptions;
      } else if (presetSelectPointer != 0) {  // Counterclockwise
        presetSelectPointer = (presetSelectPointer - 1 + totalPresetSelectOptions) % totalPresetSelectOptions;
      }
    }
    if ((encoderButtonPressed) && (transitionComplete)) {  // Select option
      processButtonPress();

      if (presetSelectPointer == 0) {  // Back to home
        changeToState(HOME);
      } else if (presetSelectPointer == 1) {  // Add new preset

        changeToState(PRESET_ADD);
        presetAddPointer = 1;
      } else {  // Get the correct data transferred to the presetInfoSettingValues buffer
        changeToState(PRESET_INFO);
        strcpy(stringInput, flashData.presets[presetSelectPointer - 2].name);
        presetInfoSettingValues[0] = flashData.presets[presetSelectPointer - 2].innerSpeedStart;
        presetInfoSettingValues[1] = flashData.presets[presetSelectPointer - 2].innerSpeedEnd;
        presetInfoSettingValues[2] = flashData.presets[presetSelectPointer - 2].outerSpeedStart;
        presetInfoSettingValues[3] = flashData.presets[presetSelectPointer - 2].outerSpeedEnd;
        presetInfoSettingValues[4] = flashData.presets[presetSelectPointer - 2].durationHr;
        presetInfoSettingValues[5] = flashData.presets[presetSelectPointer - 2].durationMin;
        presetInfoSettingValues[6] = flashData.presets[presetSelectPointer - 2].tempStart;
        presetInfoSettingValues[7] = flashData.presets[presetSelectPointer - 2].tempEnd;
      }
    }
  }
}






void drawPresetSelectScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;

    u8g2.firstPage();
    do {
      // Back button
      if (presetSelectPointer == 0) {
        drawBackButtonAndHLine(1);
      } else {
        drawBackButtonAndHLine(0);
      }

      // Plus button
      if (presetSelectPointer == 1) {
        u8g2.drawRBox(108, 0, 20, 13, 3);
        u8g2.setDrawColor(0);
        drawPlusSign(117, 6, 8, 8);
        u8g2.setDrawColor(1);
      } else {
        u8g2.setDrawColor(1);
        u8g2.drawRFrame(108, 0, 20, 13, 5);
        drawPlusSign(117, 6, 8, 8);
      }

      u8g2.setFont(u8g2_font_9x18_tf);
      u8g2.setCursor(33, 10);
      u8g2.print("Presets");
      u8g2.setFont(u8g2_font_6x10_tf);


      // Calculate the preset index (subtract 2 for the extra menu items)
      int selectedPresetIndex = presetSelectPointer - 2;

      // Adjust startIndex based on the preset index.
      if (selectedPresetIndex >= visibleItems) {
        startIndex = selectedPresetIndex - (visibleItems - 1);
      } else {
        startIndex = 0;
      }

      // Display the preset names from startIndex.
      for (int i = 0; i < totalPresetSelectOptions; i++) {
        int visibleIndex = startIndex + i;
        if (visibleIndex >= totalPresetSelectOptions) break;

        int yPos = 25 + (i * 10);
        u8g2.setCursor(3, yPos);
        u8g2.print(flashData.presets[visibleIndex].name);

        // Calculate the width of the preset name text (with extra padding).
        int textWidth = u8g2.getStrWidth(String(flashData.presets[visibleIndex].name).c_str()) + 6;
        int numOfTrailingSpaces = countTrailingSpaces(flashData.presets[visibleIndex].name);
        textWidth = textWidth - 6*numOfTrailingSpaces;
        // Highlight the selected preset.
        // Note: presetSelectPointer is offset by 2 relative to the presets array.
        if (presetSelectPointer == visibleIndex + 2) {
          u8g2.drawRFrame(0, yPos - 9, textWidth, 11, 3);
        }
      }

    } while (u8g2.nextPage());
  }
}
