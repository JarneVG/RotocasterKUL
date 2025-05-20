
void handleFinishedOperationInput() {

  if (encoderButtonPressed) {
    changeToState(HOME);
    processButtonPress();
  }
}

uint32_t previousFinishedScreenRefreshMillis = 0;
const uint32_t finishedRefreshScreenTime = 1000;

void drawFinishedOperationScreen() {
  // refresh screen
  if (currentMillis - previousFinishedScreenRefreshMillis > finishedRefreshScreenTime){
    screenRefreshFlag = true;
    previousFinishedScreenRefreshMillis = currentMillis;
  }

  if (screenRefreshFlag) {
    screenRefreshFlag = false;

    u8g2.firstPage();
    do {
      const int baseWidth = 128;
      const int baseHeight = 64;
      const int centerX = 64;
      const int centerY = 32;
      drawCenteredRoundedRectangle(centerX, centerY, baseWidth + 2, baseHeight + 2, 5, 1);
      drawCenteredRoundedRectangle(centerX, centerY, baseWidth - 4, baseHeight - 4, 5, 0);
      u8g2.setDrawColor(1);  // White text
      u8g2.setCursor(8, 13);
      u8g2.print("Cast finished in:");
      u8g2.setCursor(8, 23);

      int hours = operationSettingValues[4];
      int mins = operationSettingValues[5];
      if (hours != 0) {
        u8g2.print(hours);
        u8g2.print("h and ");
      }
      u8g2.print(mins);
      u8g2.print("min");
      u8g2.setCursor(8, 36);
      u8g2.print("Temperature: ");
      u8g2.print((int)round(thermistorTempValue));  // rounded integer
      u8g2.print("C");

      if ((int)round(thermistorTempValue) > 40){
        u8g2.drawXBMP(106, 24, 15, 15, epd_bitmap_warning_sign);
      }

      const int offCenterY = 19;
      drawCenteredRoundedRectangle(centerX, centerY + offCenterY, baseWidth - 12, 15, 5, 2);  // white box
      u8g2.setDrawColor(0);                                                                   // black text
      drawTextCenterOutline("Go back home", centerX, centerY + offCenterY + 4);
      u8g2.setDrawColor(1);  // black text
    } while (u8g2.nextPage());
  }
}
