#define TOGGLE_POS_X 105
#define TOGGLE_POS_Y 19
#define TOGGLE_WIDTH 22
#define TOGGLE_HEIGHT 15

bool selectingTemperature = false;
int maxPreheatTemp = 60;
int minPreheatTemp = 25;
void handlePreheatInput() {
  currentStateEncoderA = digitalRead(inputEncoderA);

  if (millis() - lastButtonPressMillis >= debounceTime) {                  // Debounce check
    if (!selectingTemperature) {                                           // Only move pointer if NOT selecting temperature
      if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {  // Falling edge detection
        processButtonScroll();
        

        totalPreheatOptions = (preheatEnabledFlag) ? (3) : (2);

        
        if (digitalRead(inputEncoderB) == LOW) {  // Clockwise
          if (preheatPointer >= totalPreheatOptions - 1) {
            preheatPointer = totalPreheatOptions - 1;
          } else {
            preheatPointer = (preheatPointer + 1) % totalPreheatOptions;
          }
        } else if (preheatPointer != 0) {  // Counterclockwise (if not at the first option)
          preheatPointer = (preheatPointer - 1 + totalPreheatOptions) % totalPreheatOptions;
        }
      }
    }

    if (encoderButtonPressed && transitionComplete) {  // Select option
      processButtonPress();

      if (preheatPointer == 0) {
        changeToState(HOME);  // Back to home
      } else if (preheatPointer == 1) {
        preheatEnabledFlag = !preheatEnabledFlag;  // Toggle preheat
        tempSelectionValue = maxPreheatTemp;       // Reset temperature to minimum
        preheatTargetTemp = tempSelectionValue;    // Ensure target temperature follows the reset
      }

      else if ((preheatPointer == 2) && (preheatEnabledFlag == true)) {
        selectingTemperature = !selectingTemperature;  // Enter temperature selection mode
      }
    }

    if (selectingTemperature) {  // Adjust temperature
      if (previousStateEncoderA == HIGH && currentStateEncoderA == LOW) {
        processButtonScroll();

        if (digitalRead(inputEncoderB) == LOW) {  // Increase temperature
          tempSelectionValue++;
        } else {  // Decrease temperature
          tempSelectionValue--;
        }
        tempSelectionValue = constrain(tempSelectionValue, minPreheatTemp, maxPreheatTemp);
        preheatTargetTemp = tempSelectionValue;

        screenRefreshFlag = true;
      }
    }
  }
}


void drawPreheatScreen() {
  if (screenRefreshFlag) {
    screenRefreshFlag = false;
    u8g2.firstPage();
    do {
      drawBackButtonAndHLine(preheatPointer == 0);
      u8g2.setFont(u8g2_font_9x18_tf);

      u8g2.setCursor(34, 10);
      u8g2.print("Preheat");
      u8g2.setFont(u8g2_font_6x10_tf);

      // Highlight selection
      if (preheatEnabledFlag) {
        if (preheatPointer == 1) {
          u8g2.drawRBox(0, 19, 100, 15, 3);

          u8g2.setDrawColor(0);
          u8g2.setCursor(5, 30);
          u8g2.print("Disable preheat");
          u8g2.setDrawColor(1);
          drawSwitchButton(TOGGLE_POS_X, TOGGLE_POS_Y, TOGGLE_WIDTH, TOGGLE_HEIGHT, 1, 1);




        } else {
          u8g2.setCursor(5, 30);
          u8g2.print("Disable preheat");
          drawSwitchButton(TOGGLE_POS_X, TOGGLE_POS_Y, TOGGLE_WIDTH, TOGGLE_HEIGHT, 1, 1);
        }



        if (preheatPointer == 2) {
          if (selectingTemperature) {
            u8g2.drawRBox(103, 39, 25, 15, 3);
            u8g2.setCursor(5, 50);
            u8g2.print("Set Temperature: ");
            u8g2.setDrawColor(0);
            u8g2.print(String(preheatTargetTemp) + "C");
            u8g2.setDrawColor(1);

          } else {
            u8g2.drawRFrame(103, 39, 25, 15, 3);
            u8g2.setCursor(5, 50);
            u8g2.print("Set Temperature: " + String(preheatTargetTemp) + "C");
            //u8g2.setCursor(5, 60);
            //u8g2.print(String(tempSelectionValue) + "C");
          }

        } else {
          u8g2.setCursor(5, 50);
          u8g2.print("Set Temperature: " + String(preheatTargetTemp) + "C");
          //u8g2.setCursor(5, 60);
          //u8g2.print(String(tempSelectionValue) + "C");
        }
      }

      else {
        if (preheatPointer == 1) {
          u8g2.drawRBox(0, 19, 100, 15, 3);

          u8g2.setDrawColor(0);
          u8g2.setCursor(5, 30);
          u8g2.print("Enable preheat");
          u8g2.setDrawColor(1);
          drawSwitchButton(TOGGLE_POS_X, TOGGLE_POS_Y, TOGGLE_WIDTH, TOGGLE_HEIGHT, 0, 1);


        } else {
          u8g2.setCursor(5, 30);
          u8g2.print("Enable preheat");
          drawSwitchButton(TOGGLE_POS_X, TOGGLE_POS_Y, TOGGLE_WIDTH, TOGGLE_HEIGHT, 0, 1);
        }
      }


    } while (u8g2.nextPage());
  }
}




void drawSwitchButton(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool buttonOn, bool selected) {
  // Ensure h is large enough
  if (h < 4) return;


  uint8_t circlePosX = x + h / 2;
  if (buttonOn) {
    circlePosX = circlePosX + w - h;

    u8g2.drawRBox(x, y, w, h, h / 2);
    u8g2.setDrawColor(0);
    u8g2.drawCircle(circlePosX, y + h / 2, (h - 2) / 2);
    u8g2.setDrawColor(1);

  } else {
    u8g2.drawRFrame(x, y, w, h, h / 2);
  }




  // Draw circle at the correct position
  if (selected) {
    u8g2.drawDisc(circlePosX, y + h / 2, (h - 4) / 2);

  } else {
    u8g2.setDrawColor(0);
    u8g2.drawDisc(circlePosX, y + h / 2, (h - 2) / 2);
    u8g2.setDrawColor(1);
    u8g2.drawCircle(circlePosX, y + h / 2, (h - 4) / 2);
  }
}
