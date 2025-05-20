#define NAME_START_POS_X 28


void handlePresetAddInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check after debounce

    // Check falling edge of CLK (scroll wheel rotation)
    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (editingName) {
        handleEditingNameMode();
      } else if (editingValue) {
        // While editing, use the scroll wheel to change the value of the selected setting
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          presetAddSettingValues[editingIndex]++;
          if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
            presetAddSettingValues[editingIndex + 1] = presetAddSettingValues[editingIndex];
          }
        } else {  // Counterclockwise
          presetAddSettingValues[editingIndex]--;
          if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
            presetAddSettingValues[editingIndex + 1] = presetAddSettingValues[editingIndex];
          }
        }

        // Ensure value stays within min/max limits
        presetAddSettingValues[editingIndex] = constrain(
          presetAddSettingValues[editingIndex],
          minSettingValues[editingIndex],
          maxSettingValues[editingIndex]);

        if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
          presetAddSettingValues[editingIndex + 1] = constrain(
            presetAddSettingValues[editingIndex + 1],
            minSettingValues[editingIndex + 1],
            maxSettingValues[editingIndex + 1]);
        }
      } else {
        // Adjust the presetAddPointer to navigate settings
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          if (presetAddPointer != totalPresetAddOptions - 1) {
            presetAddPointer = (presetAddPointer + 1) % totalPresetAddOptions;
          }
        } else {  // Counterclockwise
          if (presetAddPointer != 0) {
            presetAddPointer = (presetAddPointer - 1 + totalPresetAddOptions) % totalPresetAddOptions;
          }
        }
      }
    }

    // Detect button press to toggle edit mode
    if (encoderButtonPressed && transitionComplete) {
      processButtonPress();

      if (editingName) {
        if (currentCharIndex == presetNamesLength - 1) {
          editingName = false;
          presetAddPointer = 2;  // Focus next field
        }
        editingChar = !editingChar;
      } else if (editingValue) {
        editingValue = false;
        editingIndex = -1;
      } else {
        if (presetAddPointer == 0) {
          changeToState(PRESET_SELECT);
          lastStateTransitionMillis = currentMillis;
        } else if (presetAddPointer == 1) {
          editingName = true;
          currentCharIndex = 0;
          editingChar = false;
        } else if (presetAddPointer > 1 && presetAddPointer < totalPresetAddOptions - 1) {
          editingValue = true;
          editingIndex = presetAddPointer - 2;  // Adjusted index
        } else if (presetAddPointer == totalPresetAddOptions - 1) {
          // Add preset
          transferPresetAddDataToFlash();
          totalPresetSelectOptions = flashData.presetCount + 2;  // One for the back button, one for the + button

          presetSelectPointer = 2;
          changeToState(PRESET_SELECT);
          lastStateTransitionMillis = currentMillis;
        }
      }
    }
  }
  previousStateEncoderA = currentStateEncoderA;
}


void handleEditingNameMode() {
  if (editingChar) {
    if (digitalRead(inputEncoderB) == LOW) {
      stringInput[currentCharIndex] = nextChar(stringInput[currentCharIndex]);
    } else {
      stringInput[currentCharIndex] = prevChar(stringInput[currentCharIndex]);
    }
  } else {
    if (digitalRead(inputEncoderB) == LOW) {
      currentCharIndex = (currentCharIndex + 1) % presetNamesLength;
    } else {
      currentCharIndex = (currentCharIndex - 1 + presetNamesLength) % presetNamesLength;
    }
  }
}


void drawPresetAddScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;
    u8g2.firstPage();

    do {
      // Draw Back Button
      if (presetAddPointer == 0) {
        drawBackButtonAndHLine(1);
      } else {
        drawBackButtonAndHLine(0);
      }

      u8g2.setFont(u8g2_font_9x18_tf);
      u8g2.setCursor(28, 10);
      u8g2.print("Add Preset");

      u8g2.setFont(u8g2_font_6x10_tf);

      if (presetAddPointer > visibleItems) {
        startIndex = presetAddPointer - visibleItems;
        drawDottedHLine(0, 26, 128, 8, 4);  // Draw dotted line starting at (10,30) with length 100 and dot spacing 5
      } else {
        startIndex = 0;
      }

      int yPos;
      int row = 0;

      // Draw name field
      yPos = 24 + (row * 10);
      u8g2.setCursor(0, yPos);
      u8g2.print("Name");

      // Draw input box around name
      int nameBoxWidth = 6 * presetNamesLength;
      int numOfTrailingSpaces = countTrailingSpaces(stringInput);
      nameBoxWidth = nameBoxWidth - 6 * numOfTrailingSpaces;

      if (presetAddPointer == 1) {
        u8g2.drawRFrame(NAME_START_POS_X - 3, yPos - 9, nameBoxWidth, 11, 3);
        if (editingName) {
          nameBoxWidth = 6 * presetNamesLength + 5;
          u8g2.drawRBox(NAME_START_POS_X - 3, yPos - 9, nameBoxWidth, 11, 3);
          u8g2.setDrawColor(0);
          u8g2.drawHLine(NAME_START_POS_X + currentCharIndex * 6 - 1, yPos + 1, 8);
          if (editingChar) {
            u8g2.drawHLine(NAME_START_POS_X + currentCharIndex * 6 - 1, yPos - 10, 8);
            u8g2.drawHLine(NAME_START_POS_X + currentCharIndex * 6, yPos - 9, 6);
          }
          drawCheckMark(118, yPos - 1, 6, 7);
        }
      }

      u8g2.setCursor(NAME_START_POS_X, yPos);
      u8g2.print(stringInput);
      u8g2.setDrawColor(1);
      row++;

      // Draw all other settings (shifted down)
      for (int i = 0; i < totalPresetAddOptions - 3; i++) {
        int visibleIndex = startIndex + i;
        if (visibleIndex >= totalPresetAddOptions - 3) break;

        yPos = 25 + (row * 10);
        u8g2.setCursor(0, yPos);
        u8g2.print(settingNames[visibleIndex]);

        int boxWidth = 15;
        if (abs(presetAddSettingValues[visibleIndex]) >= 10) boxWidth += 5;
        if (abs(presetAddSettingValues[visibleIndex]) >= 100) boxWidth += 5;
        if (presetAddSettingValues[visibleIndex] < 0) boxWidth += 7;

        if (presetAddPointer == visibleIndex + 2) {
          u8g2.drawRFrame(100, yPos - 9, boxWidth, 11, 3);
          if (editingValue && editingIndex == visibleIndex) {
            u8g2.drawRBox(100, yPos - 9, boxWidth, 11, 3);
            u8g2.setDrawColor(0);
          }
        }

        u8g2.setCursor(105, yPos);
        u8g2.print(presetAddSettingValues[visibleIndex]);
        u8g2.setDrawColor(1);
        row++;
      }

      // Draw Start Button (last two options)
      if (presetAddPointer == totalPresetAddOptions - 2) {
        u8g2.drawRFrame(20, 59, 88, 15, 3);
        u8g2.setCursor(55, 70);
        u8g2.print("Add");
      }
      if (presetAddPointer >= totalPresetAddOptions - 1) {
        u8g2.drawRBox(20, 49, 88, 15, 3);
        u8g2.setDrawColor(0);
        u8g2.setCursor(55, 60);
        u8g2.print("Add");
        u8g2.setDrawColor(1);
      }

    } while (u8g2.nextPage());
  }
}

void drawConfirmScreen() {
  // Still empty
}


void transferPresetAddDataToFlash() {

  strncpy(newPresetAddData.name, String(stringInput).c_str(), sizeof(newPresetAddData.name) - 1);
  newPresetAddData.name[sizeof(newPresetAddData.name) - 1] = '\0';  // Ensure null termination

  // Set the rest of the preset values as needed
  newPresetAddData.innerSpeedStart = presetAddSettingValues[0];
  newPresetAddData.innerSpeedEnd = presetAddSettingValues[1];
  newPresetAddData.outerSpeedStart = presetAddSettingValues[2];
  newPresetAddData.outerSpeedEnd = presetAddSettingValues[3];
  newPresetAddData.durationHr = presetAddSettingValues[4];
  newPresetAddData.durationMin = presetAddSettingValues[5];
  newPresetAddData.tempStart = presetAddSettingValues[6];
  newPresetAddData.tempEnd = presetAddSettingValues[7];

  addPreset(newPresetAddData);
}
