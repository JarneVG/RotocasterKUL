
void drawBackButtonAndHLine(bool selected) {
  u8g2.drawHLine(0, 14, 128);  // Horizontal line

  // Back button
  if (selected) {
    u8g2.drawRBox(0, 0, 20, 13, 3);  // Filled button
    u8g2.setDrawColor(0);            // Set drawing color to "inverse" (black text on white)
  } else {
    u8g2.drawRFrame(0, 0, 20, 13, 3);  // Outlined button
    u8g2.setDrawColor(1);              // Set drawing color to normal
  }

  // Arrow inside the button (always black)
  u8g2.drawLine(4, 6, 14, 6);  // straight line
  u8g2.drawLine(4, 6, 8, 3);   // top line
  u8g2.drawLine(4, 6, 8, 9);   // bottom line

  u8g2.setDrawColor(1);  // Restore default draw color
}






/*
void drawPlusSign() {
  u8g2.drawLine(113, 6, 121, 6);   // horizontal line
  u8g2.drawLine(117, 2, 117, 10);  // vertical line
}
*/

void drawPlusSign(uint8_t centerX, uint8_t centerY, uint8_t height, uint8_t width) {
  uint8_t n = round(centerY + height/2);
  uint8_t s = round(centerY - height/2);
  uint8_t w = round(centerX - width/2);
  uint8_t e = round(centerX + width/2);
  u8g2.drawLine(e, centerY, w, centerY);   // horizontal line
  u8g2.drawLine(centerX, n, centerX, s);  // vertical line
}



void drawCheckMark(uint8_t xStart, uint8_t yStart, uint8_t width, uint8_t height) {
  float startCheck = 0.4;  // percentage of the height where check starts
  float bottomCheck = 0.4;

  u8g2.drawLine(xStart, yStart - (int)(startCheck * height), xStart + (int)(bottomCheck * width), yStart);
  u8g2.drawLine(xStart + (int)(bottomCheck * width), yStart, xStart + width, yStart - height);
}




void drawTextRightOutline(const String &text, int xPosition, int yPosition) {
  int textWidth = u8g2.getStrWidth(text.c_str());

  // Calculate the starting position of the text (right-aligned)
  int startX = xPosition - textWidth;


  // Draw the main text (right-aligned)
  u8g2.drawStr(startX, yPosition, text.c_str());  // Text positioned based on calculated startX
}


void drawTextCenterOutline(const String &text, int xPosition, int yPosition) {
  int textWidth = u8g2.getStrWidth(text.c_str());

  // Calculate the starting position of the text (center-aligned)
  int startX = xPosition - (textWidth / 2);

  // Draw the main text (center-aligned)
  //u8g2.setDrawColor(1);  // Text color (white)
  u8g2.drawStr(startX, yPosition, text.c_str());  // Text positioned based on calculated startX
}

void drawTextCenterOutlineAndUnderline(const String &text, int xPosition, int yPosition) {
  int textWidth = u8g2.getStrWidth(text.c_str());

  // Calculate the starting position of the text (center-aligned)
  int startX = xPosition - (textWidth / 2);

  // Draw the main text (center-aligned)
  //u8g2.setDrawColor(1);  // Text color (white)
  drawUnderlinedText(startX, yPosition, text.c_str());  // Text positioned based on calculated startX
}

void drawUnderlinedText(int x, int y, const char* text) {
  u8g2.setCursor(x, y);
  u8g2.print(text);

  int textWidth = u8g2.getStrWidth(text);
  int fontAscent = u8g2.getAscent();
  int fontDescent = u8g2.getDescent();

  // The underline goes just below the text baseline
  int underlineY = y + 1; // You can adjust +1 to fine-tune
  int numOfTrailingSpaces = countTrailingSpaces(text);
  u8g2.drawLine(x, underlineY, x + textWidth-1-numOfTrailingSpaces*6, underlineY);
}

int countTrailingSpaces(const String& input) {
  int count = 0;
  for (int i = input.length() - 1; i >= 0; i--) {
    if (input.charAt(i) == ' ') {
      count++;
    } else {
      break;
    }
  }
  return count;
}

// ----- Helper Functions ----- //
char nextChar(char current) {
  // Find the current character in allowedChars; if not found, default to first element
  int index = 0;
  for (int i = 0; i < numAllowedChars; i++) {
    if (allowedChars[i] == current) {
      index = i;
      break;
    }
  }
  // Move to next character with wrap-around
  index = (index + 1) % numAllowedChars;
  Serial.println(allowedChars[index]);

  return allowedChars[index];
}

char prevChar(char current) {
  int index = 0;
  for (int i = 0; i < numAllowedChars; i++) {
    if (allowedChars[i] == current) {
      index = i;
      break;
    }
  }
  // Move to previous character with wrap-around
  index = (index - 1 + numAllowedChars) % numAllowedChars;
  Serial.println(allowedChars[index]);
  return allowedChars[index];
}



void drawDottedHLine(int x_start, int y, int length, int dot_spacing = 4, int dot_length = 2) {
  for (int x = x_start; x < x_start + length; x += dot_spacing) {
    u8g2.drawHLine(x, y, dot_length);  // draw a 2-pixel mini segment
  }
}




void drawCenteredRoundedRectangle(int cx, int cy, int width, int height, int radius, uint8_t mode) {
    // Compute top-left corner
    int x = cx - width / 2;
    int y = cy - height / 2;

    switch (mode) {
        case 0: // Normal outline, Frame
            u8g2.setDrawColor(1);
            u8g2.drawRFrame(x, y, width, height, radius);
            break;
        case 1: // Filled black box
            u8g2.setDrawColor(0);
            u8g2.drawRBox(x, y, width, height, radius);
            break;
        case 2: // Filled white box (background color inside)
            u8g2.setDrawColor(1);  // Draw box with foreground
            u8g2.drawRBox(x, y, width, height, radius);
            u8g2.setDrawColor(0);  // Reset to normal for any other drawing
            break;
        default:
            // Do nothing for invalid mode
            break;
    }
}


void drawCloseButton(uint8_t centerX, uint8_t centerY, uint8_t width, uint8_t height) {
  // Calculate top-left corner for the box
  uint8_t x = round(centerX - width / 2);
  uint8_t y = round(centerY - height / 2);

  // Step 1: Draw a filled white rounded box
  u8g2.setDrawColor(1); // 1 = White (on monochrome displays)
  u8g2.drawRBox(x, y, width, height, min(width, height) / 4);  // Rounded box

  // Step 2: Draw the black "X" symbol on top
  u8g2.setDrawColor(0); // 0 = Black (inverted on white background)

  uint8_t left   = x + 2;
  uint8_t right  = x + width - 3;
  uint8_t top    = y + 2;
  uint8_t bottom = y + height - 3;

  u8g2.drawLine(left, top, right, bottom);     // Diagonal
  u8g2.drawLine(left, bottom, right, top);     // Diagonal

  // Step 3: Reset draw color for future drawing
  u8g2.setDrawColor(1);
}


