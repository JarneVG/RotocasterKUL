
void handlePresetInfoInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  if (millis() - lastButtonPressMillis >= debounceTime) {  // Only check after debounce

    // Check falling edge of CLK (scroll wheel rotation)
    if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
      processButtonScroll();

      if (editingName) {
        editedPresetInfoFlag = true;  // set flag to write settings to flash
        handleEditingNameMode();
      } else if (editingValue) {
        // While editing, use the scroll wheel to change the value of the selected setting
        editedPresetInfoFlag = true;                  // set flag to write settings to flash
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          presetInfoSettingValues[editingIndex]++;
          if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
            presetInfoSettingValues[editingIndex + 1] = presetInfoSettingValues[editingIndex];
          }
        } else {  // Counterclockwise
          presetInfoSettingValues[editingIndex]--;
          if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
            presetInfoSettingValues[editingIndex + 1] = presetInfoSettingValues[editingIndex];
          }
        }

        // Ensure value stays within min/max limits
        presetInfoSettingValues[editingIndex] = constrain(
          presetInfoSettingValues[editingIndex],
          minSettingValues[editingIndex],
          maxSettingValues[editingIndex]);

        if ((editingIndex == 0) || (editingIndex == 2) || (editingIndex == 6)) {
          presetInfoSettingValues[editingIndex + 1] = constrain(
            presetInfoSettingValues[editingIndex + 1],
            minSettingValues[editingIndex + 1],
            maxSettingValues[editingIndex + 1]);
        }
      } else {
        // Adjust the presetInfoPointer to navigate settings
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          if (presetInfoPointer != totalPresetInfoOptions - 1) {
            presetInfoPointer = (presetInfoPointer + 1) % totalPresetInfoOptions;
          }
        } else {  // Counterclockwise
          if (presetInfoPointer != 0) {
            presetInfoPointer = (presetInfoPointer - 1 + totalPresetInfoOptions) % totalPresetInfoOptions;
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
          presetInfoPointer = 2;  // Focus next field
        }
        editingChar = !editingChar;
      } else if (editingValue) {
        editingValue = false;
        editingIndex = -1;
      } else {
        if (presetInfoPointer == 0) {
          changeToState(PRESET_SELECT);
          presetInfoPointer = 1;
        } else if (presetInfoPointer == 1) {
          editingName = true;
          currentCharIndex = 0;
          editingChar = false;
        } else if (presetInfoPointer > 1 && presetInfoPointer < totalPresetInfoOptions - 3) {
          editingValue = true;
          editingIndex = presetInfoPointer - 2;                        // Adjusted index
        } else if (presetInfoPointer == totalPresetInfoOptions - 3) {  // Save
          // Save new values
          transferPresetInfoDataToFlash();
          presetInfoPointer = 1;
          changeToState(PRESET_SELECT);
        } else if (presetInfoPointer == totalPresetInfoOptions - 2) {  // Delete
          presetInfoPointer = 0;
          presetSelectPointer--;
          changeToState(CONFIRM_DELETE);
        } else if (presetInfoPointer == totalPresetInfoOptions - 1) {  // Start
          // Data will be transferred from info to operation in the confirm state
          if (editedPresetInfoFlag) {
            transferPresetInfoDataToFlash();  // save data
          }
          confirmQuickStartPointer = 0;  // Use same pointer as in quickstart for easier coding
          presetInfoPointer = 0;
          changeToState(CONFIRM_PRESET_INFO_START);
        }
      }
    }
  }
  previousStateEncoderA = currentStateEncoderA;
}


// void handleEditingNameMode() {
//   if (editingChar) {
//     if (digitalRead(inputEncoderB) == LOW) {
//       stringInput[currentCharIndex] = nextChar(stringInput[currentCharIndex]);
//     } else {
//       stringInput[currentCharIndex] = prevChar(stringInput[currentCharIndex]);
//     }
//   } else {
//     if (digitalRead(inputEncoderB) == LOW) {
//       currentCharIndex = (currentCharIndex + 1) % presetNamesLength;
//     } else {
//       currentCharIndex = (currentCharIndex - 1 + presetNamesLength) % presetNamesLength;
//     }
//   }
// }


void drawPresetInfoScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;
    u8g2.firstPage();

    do {
      // Draw Back Button
      if (presetInfoPointer == 0) {
        drawBackButtonAndHLine(1);
      } else {
        drawBackButtonAndHLine(0);
      }

      u8g2.setFont(u8g2_font_9x18_tf);
      u8g2.setCursor(25, 10);
      u8g2.print("Preset Info");

      u8g2.setFont(u8g2_font_6x10_tf);

      if (presetInfoPointer > visibleItems) {
        startIndex = presetInfoPointer - visibleItems;
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
      nameBoxWidth = nameBoxWidth - 6*numOfTrailingSpaces;
      if (presetInfoPointer == 1) {
        u8g2.drawRFrame(NAME_START_POS_X - 2, yPos - 9, nameBoxWidth, 11, 3);
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
      for (int i = 0; i < totalPresetInfoOptions - 3; i++) {
        int visibleIndex = startIndex + i;
        if (visibleIndex >= totalPresetInfoOptions - 3) break;

        yPos = 25 + (row * 10);
        u8g2.setCursor(0, yPos);
        u8g2.print(settingNames[visibleIndex]);

        int boxWidth = 15;
        if (abs(presetInfoSettingValues[visibleIndex]) >= 10) boxWidth += 5;
        if (abs(presetInfoSettingValues[visibleIndex]) >= 100) boxWidth += 5;
        if (presetInfoSettingValues[visibleIndex] < 0) boxWidth += 7;

        if (presetInfoPointer == visibleIndex + 2) {
          u8g2.drawRFrame(100, yPos - 9, boxWidth, 11, 3);
          if (editingValue && editingIndex == visibleIndex) {
            u8g2.drawRBox(100, yPos - 9, boxWidth, 11, 3);
            u8g2.setDrawColor(0);
          }
        }

        u8g2.setCursor(105, yPos);
        u8g2.print(presetInfoSettingValues[visibleIndex]);
        u8g2.setDrawColor(1);
        row++;
      }

      // Draw Start Button (last two options)
      if (presetInfoPointer == totalPresetInfoOptions - 4) {
        // Cover some weird stuff that's happening
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 57, 128, 64 - 57);
        u8g2.setDrawColor(1);

        drawSaveDeleteStartPreset(60, 0);
      }
      if (presetInfoPointer >= totalPresetInfoOptions - 3) {
        // Cover some weird stuff that's happening
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 27, 128, 64 - 27);
        u8g2.setDrawColor(1);

        yPos = 25 + (1 * 10);
        u8g2.setCursor(0, yPos);
        u8g2.print(settingNames[6]);
        u8g2.setCursor(105, yPos);
        u8g2.print(presetInfoSettingValues[6]);

        yPos += 10;
        u8g2.setCursor(0, yPos);
        u8g2.print(settingNames[7]);
        u8g2.setCursor(105, yPos);
        u8g2.print(presetInfoSettingValues[7]);

        drawSaveDeleteStartPreset(48, presetInfoPointer);
      }

    } while (u8g2.nextPage());
  }
}


void drawSaveDeleteStartPreset(uint8_t yPos, uint8_t currentPointer) {
  int beginOffset = 6;
  int centerOffsets = 5;
  int textOffsetY = 11;
  int saveWidth = u8g2.getStrWidth(String("Save").c_str());
  int deleteWidth = u8g2.getStrWidth(String("Delete").c_str());
  int startWidth = u8g2.getStrWidth(String("Start").c_str());
  int deleteStartX = beginOffset + saveWidth + 6 + centerOffsets;
  int saveStartX = deleteStartX + deleteWidth + 6 + centerOffsets;

  u8g2.drawRFrame(beginOffset, yPos, saveWidth + 6, 15, 3);
  u8g2.drawRFrame(deleteStartX, yPos, deleteWidth + 6, 15, 3);
  u8g2.drawRFrame(saveStartX, yPos, startWidth + 6, 15, 3);

  if (currentPointer == totalPresetInfoOptions - 3) {
    u8g2.drawRBox(beginOffset, yPos, saveWidth + 6, 15, 3);

    u8g2.setDrawColor(0);
  }
  u8g2.setCursor(beginOffset + 3, yPos + textOffsetY);
  u8g2.print("Save");
  u8g2.setDrawColor(1);

  if (currentPointer == totalPresetInfoOptions - 2) {
    u8g2.drawRBox(deleteStartX, yPos, deleteWidth + 6, 15, 3);
    u8g2.setDrawColor(0);
  }
  u8g2.setCursor(deleteStartX + 3, yPos + textOffsetY);
  u8g2.print("Delete");
  u8g2.setDrawColor(1);

  if (currentPointer == totalPresetInfoOptions - 1) {
    u8g2.drawRBox(saveStartX, yPos, startWidth + 6, 15, 3);
    u8g2.setDrawColor(0);
  }
  u8g2.setCursor(saveStartX + 3, yPos + textOffsetY);
  u8g2.print("Start");
  u8g2.setDrawColor(1);
}


void transferPresetInfoDataToFlash() {

  strncpy(newPresetAddData.name, String(stringInput).c_str(), sizeof(newPresetAddData.name) - 1);
  newPresetAddData.name[sizeof(newPresetAddData.name) - 1] = '\0';  // Ensure null termination

  // Set the rest of the preset values as needed
  newPresetAddData.innerSpeedStart = presetInfoSettingValues[0];
  newPresetAddData.innerSpeedEnd = presetInfoSettingValues[1];
  newPresetAddData.outerSpeedStart = presetInfoSettingValues[2];
  newPresetAddData.outerSpeedEnd = presetInfoSettingValues[3];
  newPresetAddData.durationHr = presetInfoSettingValues[4];
  newPresetAddData.durationMin = presetInfoSettingValues[5];
  newPresetAddData.tempStart = presetInfoSettingValues[6];
  newPresetAddData.tempEnd = presetInfoSettingValues[7];

  updatePreset(newPresetAddData, presetSelectPointer - 2);
}


