void handleQuickStartInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check after debounce

    // Check falling edge of CLK (scroll wheel rotation)
    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (editingValue) {
        // While editing, use the scroll wheel to change the value of the selected setting
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          quickStartSettingValues[editingIndex]++;
          if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
            quickStartSettingValues[editingIndex + 1] = quickStartSettingValues[editingIndex];
          }
        } else {  // Counterclockwise
          quickStartSettingValues[editingIndex]--;
          if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
            quickStartSettingValues[editingIndex + 1] = quickStartSettingValues[editingIndex];
          }
        }


        // Ensure value stays within min/max limits
        quickStartSettingValues[editingIndex] = constrain(
          quickStartSettingValues[editingIndex],
          minSettingValues[editingIndex],
          maxSettingValues[editingIndex]);

        if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
          quickStartSettingValues[editingIndex + 1] = constrain(
            quickStartSettingValues[editingIndex + 1],
            minSettingValues[editingIndex + 1],
            maxSettingValues[editingIndex + 1]);
        }
      } else {
        // Adjust the quickStartPointer to navigate settings
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          if (quickStartPointer != totalQuickStartOptions - 1) {
            quickStartPointer = (quickStartPointer + 1) % totalQuickStartOptions;
          }
        } else {  // Counterclockwise
          if (quickStartPointer != 0) {
            quickStartPointer = (quickStartPointer - 1 + totalQuickStartOptions) % totalQuickStartOptions;
          }
        }
      }
    }

    // Detect button press to toggle edit mode
    if (encoderButtonPressed && transitionComplete) {
      processButtonPress();

      if (editingValue) {
        // Save the edited value when clicking again
        editingValue = false;
        editingIndex = -1;

      } else {
        // Start editing the selected value
        if (quickStartPointer > 0 && quickStartPointer < totalQuickStartOptions - 1) {
          editingValue = true;
          editingIndex = quickStartPointer - 1;  // Adjust based on your array structure
        }
        if (quickStartPointer == 0) {
          changeToState(HOME);
        }
        if (quickStartPointer == totalQuickStartOptions - 1) {
          confirmQuickStartPointer = 0;
          changeToState(CONFIRM_QUICK_START);
        }
      }
    }
  }
}





void drawQuickStartScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;
    u8g2.firstPage();
    do {
      // Highlight the back button if selected
      if (quickStartPointer == 0) {
        drawBackButtonAndHLine(1);
      } else {
        drawBackButtonAndHLine(0);
      }

      // Set font for title
      u8g2.setFont(u8g2_font_9x18_tf);
      u8g2.setCursor(28, 10);
      u8g2.print("Quick Start");

      u8g2.setFont(u8g2_font_6x10_tf);

      // Adjust startIndex based on quickStartPointer
      if (quickStartPointer > visibleItems) {
        startIndex = quickStartPointer - visibleItems;
      } else {
        startIndex = 0;
      }

      // Display the settings from startIndex
      for (int i = 0; i < totalQuickStartOptions - 2; i++) {
        int visibleIndex = startIndex + i;
        if (visibleIndex >= totalQuickStartOptions - 2) break;

        int yPos = 25 + (i * 10);
        u8g2.setCursor(0, yPos);
        u8g2.print(settingNames[visibleIndex]);

        int boxWidth = 15;
        if (abs(quickStartSettingValues[visibleIndex]) >= 10) { boxWidth += 5; }
        if (abs(quickStartSettingValues[visibleIndex]) >= 100) { boxWidth += 5; }
        if (quickStartSettingValues[visibleIndex] < 0) { boxWidth += 7; }

        // Highlight selected row
        if (quickStartPointer == visibleIndex + 1) {
          u8g2.drawRFrame(100, yPos - 9, boxWidth, 11, 3);
          //u8g2.setDrawColor(0);  // Invert text color

          // If editing, show edit state
          if (editingValue && editingIndex == visibleIndex) {
            u8g2.drawRBox(100, yPos - 9, boxWidth, 11, 3);  // Editable state border
            u8g2.setDrawColor(0);                           // Invert text color
          }
        }

        // Display the setting value
        u8g2.setCursor(105, yPos);
        u8g2.print(quickStartSettingValues[visibleIndex]);

        u8g2.setDrawColor(1);  // Reset color after highlighting
      }


      // Draw start button
      if (quickStartPointer == totalQuickStartOptions - 2) {
        u8g2.drawRFrame(20, 59, 88, 15, 3);
        u8g2.setCursor(50, 70);
        u8g2.print("Start");
      }
      if (quickStartPointer >= totalQuickStartOptions - 1) {
        u8g2.drawRBox(20, 59 - 10, 88, 15, 3);
        u8g2.setDrawColor(0);
        u8g2.setCursor(50, 70 - 10);
        u8g2.print("Start");
        u8g2.setDrawColor(1);
      }

    } while (u8g2.nextPage());
  }
}
