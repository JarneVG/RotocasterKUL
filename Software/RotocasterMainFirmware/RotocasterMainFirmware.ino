#include <Wire.h>
#include <U8g2lib.h>
#include "bitmaps.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "SmoothLED.h"
#include <ezBuzzer.h> // ezBuzzer library
#include "thermistor.h"
//#include "RP2040_PWM.h"
#include <hardware/irq.h>
#include <hardware/timer.h>




const unsigned int totalHomeOptions = 4;  // Total selectable options (0 = none, 1 = quick start, 2 = preset select)
const unsigned int totalQuickStartOptions = 10;
unsigned int totalPresetSelectOptions = 7;  // cannot be const uint since SD contents is variable
const unsigned int totalPresetAddOptions = 11;
const unsigned int totalPresetInfoOptions = 13;  // Same as presetAdd but needs delete and start operation
const unsigned int totalOperationOptions = 5;
const unsigned int totalWarningOptions = 5;
const unsigned int totalConfirmQuickStartOptions = 2;
const unsigned int totalConfirmPresetOptions = 2;
unsigned int totalPreheatOptions = 3;

int homePointer = 0;        // 0 = none, 1 = Quick Start, 2 = Select Preset
int quickStartPointer = 0;  // 0 = none, 1 = Quick Start, 2 = Select Preset
int presetSelectPointer = 0;
int presetAddPointer = 0;
int presetInfoPointer = 0;
int operationPointer = 0;
int warningPointer = 0;
int confirmQuickStartPointer = 0;
int confirmPresetPointer = 0;
int preheatPointer = 0;
int sensorScreenPointer = 0;

//void drawSensorScreen();

// --------------------------- FLASH ----------------------------------//


#define XIP_BASE 0x10000000
#define FLASH_SECTOR_SIZE 4096
#define FLASH_STORAGE_OFFSET ((1024) * 1024)  // 1.0 MB offset
#define MAX_PRESETS 50                        // Maximum number of presets

const char *adjectives[] = {
  "Squishy", "Flexi", "Soft", "Bendy", "Elastic",
  "Stretch", "Malle", "Comply", "Jelly", "Pliable",
  "Rubbery", "Wobbly", "Cushy", "Yieldy", "Squirmy",
  "Bouncy", "Spongy", "Supple", "Mushy", "Floppy"
};

const char *subjects[] = {
  "Gripper", "Bot", "Hand", "Tendon", "Pad",
  "Arm", "Claw", "Finger", "Limb", "Joint",
  "Digit", "Glove", "Fist", "Wrist", "Elbow",
  "Sensor", "Trigger", "Muscle", "Valve", "Link"
};



struct PresetData {
  char name[16];
  int16_t innerSpeedStart;
  int16_t innerSpeedEnd;
  int16_t outerSpeedStart;
  int16_t outerSpeedEnd;
  int16_t durationHr;
  int16_t durationMin;
  int16_t tempStart;
  int16_t tempEnd;
};

PresetData newPresetAddData;
PresetData lastUsedPresetData;

struct Settings {
  uint8_t version;
  uint8_t brightness;
  uint8_t volume;
};

struct FlashStorage {
  Settings settings;
  uint8_t presetCount;
  PresetData presets[MAX_PRESETS];
};

FlashStorage flashData;

void __not_in_flash_func(writeFlashData)() {
  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
  restore_interrupts(ints);

  ints = save_and_disable_interrupts();
  flash_range_program(FLASH_STORAGE_OFFSET, (const uint8_t *)&flashData, sizeof(FlashStorage));
  restore_interrupts(ints);
}

void readFlashData() {
  FlashStorage *dataRead = (FlashStorage *)(XIP_BASE + FLASH_STORAGE_OFFSET);
  if (dataRead->settings.version == 0xFF) {  // Uninitialized flash check
    Serial.println("Flash is empty. Initializing default values.");
    flashData.settings = { 1, 50, 75 };
    flashData.presetCount = 0;
    writeFlashData();
  } else {
    flashData = *dataRead;
  }

  Serial.println("Stored settings:");
  Serial.print("Version: ");
  Serial.println(flashData.settings.version);
  Serial.print("Brightness: ");
  Serial.println(flashData.settings.brightness);
  Serial.print("Volume: ");
  Serial.println(flashData.settings.volume);
  Serial.print("Preset count: ");
  Serial.println(flashData.presetCount);

  for (uint8_t i = 0; i < flashData.presetCount; i++) {
    Serial.print("Preset ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(flashData.presets[i].name);
  }
}

bool addPreset(PresetData newPreset) {
  if (flashData.presetCount >= MAX_PRESETS) {
    Serial.println("Preset storage full!");
    return false;
  }
  flashData.presets[flashData.presetCount] = newPreset;
  flashData.presetCount++;
  writeFlashData();
  return true;
}


bool updatePreset(PresetData newPreset, uint8_t index) {
  if (index >= flashData.presetCount) {
    Serial.println("Invalid index: no preset exists at that position!");
    return false;
  }

  flashData.presets[index] = newPreset;
  writeFlashData();
  Serial.print("Updated preset at index ");
  Serial.println(index);
  return true;
}


bool saveLastUsedPreset(PresetData newPreset) {
  int8_t lastUsedIndex = -1;

  // Look for a preset containing "LAST USED" in its name
  for (uint8_t i = 0; i < flashData.presetCount; i++) {
    if (strstr(flashData.presets[i].name, "LAST USED") != nullptr) {
      lastUsedIndex = i;
      break;
    }
  }

  // If found, update the preset at that index
  if (lastUsedIndex != -1) {
    flashData.presets[lastUsedIndex] = newPreset;

    // Move it to the front if it's not already there
    if (lastUsedIndex != 0) {
      PresetData temp = flashData.presets[lastUsedIndex];
      // Shift presets from 0 to lastUsedIndex - 1 one step to the right
      for (int i = lastUsedIndex; i > 0; i--) {
        flashData.presets[i] = flashData.presets[i - 1];
      }
      flashData.presets[0] = temp;
    }

    writeFlashData();
    Serial.println("Updated and moved LAST USED preset to front.");
    return true;
  }

  // If not found, check if we can add a new one
  if (flashData.presetCount >= MAX_PRESETS) {
    Serial.println("Preset storage full!");
    return false;
  }

  // Shift all presets to make space at index 0
  for (int i = flashData.presetCount; i > 0; i--) {
    flashData.presets[i] = flashData.presets[i - 1];
  }

  flashData.presets[0] = newPreset;
  flashData.presetCount++;
  totalPresetSelectOptions++;
  writeFlashData();
  Serial.println("Added LAST USED preset at the front.");
  return true;
}


void generateRandomName(char *buffer, size_t length) {
  snprintf(buffer, length, "%s %s", adjectives[random(0, 20)], subjects[random(0, 20)]);
}

// --------------------------- FLASH END ----------------------------------//





// Timer variables
uint32_t lastStateTransitionMillis = 0;
uint32_t lastButtonPressMillis = 0;
uint32_t operationStartMillis = 0;
uint32_t totalOperationMillis = 0;  // Sum of hours and minutes
uint32_t lastRefreshMillis = 0;     // Used for animations on operation screen
uint32_t currentMillis = 0;


// Not using enum because this caused compile errors
#define HOME 0
#define PREHEAT 1
#define PRESET_SELECT 2
#define PRESET_ADD 3
#define PRESET_INFO 4
#define OPERATION 5
#define CONFIRM_QUICK_START 6
#define CONFIRM_PRESET_INFO_START 7
#define CONFIRM_PRESET 8
#define CONFIRM_DELETE 9
#define QUICK_START 10
#define WARNING 11
#define FINISHED_OPERATION 12
#define SENSOR_SCREEN 13


uint8_t CURRENT_SCREEN_STATE = HOME;

enum OperationScreenExtraStates_t {
  OPERATION_NORMAL,
  OPERATION_BACK,
  OPERATION_MOTOR_1,
  OPERATION_MOTOR_2,
  OPERATION_TEMPERATURE,
  OPERATION_INFO,
  OPERATION_TIME
};

OperationScreenExtraStates_t operationScreenExtraState = OPERATION_NORMAL;

// Rotary Encoder Inputs
#define encoderSelectButton 2
#define inputEncoderA 1  // CLK
#define inputEncoderB 0  // DT
#define LED_STRIP_PIN 3
#define SCREEN_CS_PIN 4
#define BUZZER_PIN 5

#define EN_PIN_MOTOR_1 11
#define DIR_PIN_MOTOR_1 13
#define STEP_PIN_MOTOR_1 14
#define CS_PIN_MOTOR_1 15

#define EN_PIN_MOTOR_2 20
#define DIR_PIN_MOTOR_2 10
#define STEP_PIN_MOTOR_2 9
#define CS_PIN_MOTOR_2 8

#define HEATER_PIN 22
#define FAN_PIN 21
#define THERMISTOR_PIN 26

#define LIMIT_SWITCH_PIN 27






int currentStateEncoderA;
int previousStateEncoderA;


bool transitionComplete = false;
bool screenRefreshFlag = true;               // 0 means no refresh is needed, 1 means refresh is needed
volatile bool encoderButtonPressed = false;  // Flag set in ISR
bool preheatEnabledFlag = false;
bool almostExitingOperationFlag = false;
bool finishedOperationFlag = false;
bool editedPresetInfoFlag = false;  // to know wether or not to write to flash when starting operation

bool doorState = false;  // false is closed, true is open
bool heaterFanState = 0;



int preheatTargetTemp = 20;
int tempSelectionValue = 20;
float thermistorTempValue = 21.2;
float averagedThermistorTempValue = thermistorTempValue;
const float thermistorAlpha = 0.1;
uint32_t lastHeaterCalculationMillis = 0xFFFFFFFF;  // start with a giant number, only change it to something reasonable once heater has been turned on

THERMISTOR thermistor(THERMISTOR_PIN,  // Analog pin
                      10000,           // Nominal resistance at 25 ºC
                      3600,            // thermistor's beta coefficient
                      10000);          // Value of the series resistor





// Steppers

void enableSteppers(int, int);
void initializeSteppers(uint8_t, uint8_t);



// RP2040_PWM *PWM1;
// RP2040_PWM *PWM2;


#include <TMC2130Stepper.h>

#define CW 1
#define CCW 0


#define R_SENSE 0.11f  // Match to your driver \
                       // SilentStepStick series use 0.11 \
                       // UltiMachine Einsy and Archim2 boards use 0.2 \
                       // Panucatt BSD2660 uses 0.1 \
                       // Watterott TMC5160 uses 0.075


TMC2130Stepper driver_1 = TMC2130Stepper(EN_PIN_MOTOR_1, DIR_PIN_MOTOR_1, STEP_PIN_MOTOR_1, CS_PIN_MOTOR_1);
TMC2130Stepper driver_2 = TMC2130Stepper(EN_PIN_MOTOR_2, DIR_PIN_MOTOR_2, STEP_PIN_MOTOR_2, CS_PIN_MOTOR_2);



#define MICROSTEP_1 64
#define MICROSTEP_2 64

#define MAX_MILLIAMPS_MOTOR_1 600
#define MAX_MILLIAMPS_MOTOR_2 600

#define maxFrameFreqRPM 30
float stepFrequency1;
float stepFrequency2;

unsigned long microsInterval1;
unsigned long microsInterval2;

volatile bool bitBangStepperActive1;
volatile bool bitBangStepperActive2;





void handleEncoderSelectInterrupt() {
  if (transitionComplete) {       // only after a certain time after the previous interrupt
    encoderButtonPressed = true;  // Set flag when interrupt fires
  }
}




// Scrolling
int startIndex = 0;          // Defines which variable to display first
const int visibleItems = 4;  // Maximum number of visible items on the screen


#define presetNamesLength 16  // 15 alphanumerical, 1 check
const char allowedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789";
const int numAllowedChars = sizeof(allowedChars) - 1;  // subtract the null terminator

// ----- Global Variables ----- //
char stringInput[presetNamesLength] = "NAME           ";  // initial 15-character string (plus '\0')
int currentCharIndex = 0;                                 // Currently selected character position (0 to 9)
bool editingChar = false;                                 // false: moving the cursor; true: editing the character
bool editingName = false;                                 // Needs extra flag because of extra layer of input









#define progressBarTotalLength 124
#define progressBarHeight 2
uint8_t progressBarCurrentPixels = 0;
uint8_t bitmapAnimationCounter = 0;
#define bitmapWidth 19
#define bitmapHeight 19
#define bitmapStartX 1
#define bitmapSpacerX 25
#define bitmapStartY 17

uint32_t remainingHours, remainingMinutes, remainingSeconds;








bool editingValue = false;  // Whether you're editing a value (see quickStart, preheat, ...)
int editingIndex = -1;      // The index of the setting you're currently editing



#define numberOfSettings 8  // Define size at compile time


int16_t quickStartInnerSpeedStart = 1;
int16_t quickStartInnerSpeedEnd = 1;
int16_t quickStartOuterSpeedStart = 4;
int16_t quickStartOuterSpeedEnd = 4;
int16_t quickStartDurationHr = 0;
int16_t quickStartDurationMin = 30;
int16_t quickStartTempStart = 20;
int16_t quickStartTempEnd = 20;

int16_t presetInfoInnerSpeedStart = 30;
int16_t presetInfoInnerSpeedEnd = 30;
int16_t presetInfoOuterSpeedStart = 25;
int16_t presetInfoOuterSpeedEnd = 25;
int16_t presetInfoDurationHr = 0;
int16_t presetInfoDurationMin = 1;
int16_t presetInfoTempStart = 20;
int16_t presetInfoTempEnd = 20;

int16_t presetAddInnerSpeedStart = 30;
int16_t presetAddInnerSpeedEnd = 30;
int16_t presetAddOuterSpeedStart = 25;
int16_t presetAddOuterSpeedEnd = 25;
int16_t presetAddDurationHr = 0;
int16_t presetAddDurationMin = 1;
int16_t presetAddTempStart = 20;
int16_t presetAddTempEnd = 20;


// Define settings names
const char *settingNames[] = {
  "Inner DPS Start",
  "Inner DPS End",
  "Outer DPS Start",
  "Outer DPS End",
  "Duration hr",
  "Duration min",
  "Temp Start",
  "Temp End",
  " ",
  " "
};


// Define sensor names
const char *sensorNames[] = {
  "Temp raw: ",
  "Temp av.: ",
  "Door: ",
  "M1 error: ",
  "M1 step EN: ",
  "M1 stall det: ",
  "M2 error: ",
  "M2 step EN: ",
  "M2 stall det: ",
  "On-time hr: ",
  "On-time min: ",
  "On-time sec: ",
};

String sensorValues[] = {
  "20C",
  "21C",
  "NYI",
  "NYI",
  "NYI",
  "NYI",
  "NYI",
  "NYI",
  "NYI",
  "1",
  "2",
  "3",
};

// String(round(currentMillis/(1000*60*60))),
// String(round(currentMillis/(1000*60))),
// String(round(currentMillis/1000)),

const uint8_t totalSensorScreenOptions = sizeof(sensorNames) / sizeof(sensorNames[0]);


// Corresponding values
int16_t quickStartSettingValues[] = {
  quickStartInnerSpeedStart,
  quickStartInnerSpeedEnd,
  quickStartOuterSpeedStart,
  quickStartOuterSpeedEnd,
  quickStartDurationHr,
  quickStartDurationMin,
  quickStartTempStart,
  quickStartTempEnd
};

// Corresponding values
int16_t presetInfoSettingValues[] = {
  presetInfoInnerSpeedStart,
  presetInfoInnerSpeedEnd,
  presetInfoOuterSpeedStart,
  presetInfoOuterSpeedEnd,
  presetInfoDurationHr,
  presetInfoDurationMin,
  presetInfoTempStart,
  presetInfoTempEnd
};

// Corresponding values
int16_t presetAddSettingValues[] = {
  presetAddInnerSpeedStart,
  presetAddInnerSpeedEnd,
  presetAddOuterSpeedStart,
  presetAddOuterSpeedEnd,
  presetAddDurationHr,
  presetAddDurationMin,
  presetAddTempStart,
  presetAddTempEnd
};



int16_t operationSettingValues[8] = { 0 };  // Initialize array to all zeros until confirmed by the user (either quick start or presets)
float currentTargetInnerSpeed, currentTargetOuterSpeed, currentTargetTemperature;
float limitedInnerSpeedSetpoint, limitedOuterSpeedSetpoint, limitedTargetTemperatureSetpoint;
float maxInnerSpeedRate = 6.0;  // max speed change per second
float maxOuterSpeedRate = 6.0;  // max speed change per second
float maxTemperatureRate = 80;  // max temperature change per second

unsigned long previousRateLimitMillis = 0;

int16_t minSettingValues[] = {
  -180,
  -180,
  -180,
  -180,
  0,
  0,
  20,
  20
};

int16_t maxSettingValues[] = {
  180,
  180,
  180,
  180,
  48,
  59,
  80,
  80
};





//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* cs=*/SCREEN_CS_PIN, /* reset=*/U8X8_PIN_NONE);


#define ledRampupTime 1000
#define ledHoldTime 60 * 1000
#define ledRampDownTime 500

SmoothLED smoothLed(LED_STRIP_PIN, ledRampupTime, ledHoldTime, ledRampDownTime);
ezBuzzer buzzer(BUZZER_PIN); // attach to buzzer pin

unsigned long lastScreenRefreshMillis = 0;
const unsigned long automaticScreenRefreshInterval = 1000;  // Refresh every 2000 ms (2 second)




// Frequency control
volatile float freq_motor_1 = 0;
volatile float freq_motor_2 = 0;
volatile uint32_t last_toggle_motor_1 = 0;
volatile uint32_t last_toggle_motor_2 = 0;
volatile uint32_t period_motor_1_us = 1000000;
volatile uint32_t period_motor_2_us = 1000000;
volatile bool step_state_motor_1 = false;
volatile bool step_state_motor_2 = false;

repeating_timer_t heartbeat_timer;

// Timer callback: toggles both motors if it's time
bool timerCallback(repeating_timer_t *rt) {
uint32_t now = time_us_32();

  // Motor 1
  if (freq_motor_1 > 0 && (uint32_t)(now - last_toggle_motor_1) >= period_motor_1_us / 2) {
    last_toggle_motor_1 = now;
    step_state_motor_1 = !step_state_motor_1;
    digitalWriteFast(STEP_PIN_MOTOR_1, step_state_motor_1);
  }

  // Motor 2
  if (freq_motor_2 > 0 && (uint32_t)(now - last_toggle_motor_2) >= period_motor_2_us / 2) {
    last_toggle_motor_2 = now;
    step_state_motor_2 = !step_state_motor_2;
    digitalWriteFast(STEP_PIN_MOTOR_2, step_state_motor_2);
  }

  return true;  // Keep repeating
}



void setup() {
  Serial.begin(9600);
  delay(500);

  u8g2.begin();
  homePointer = 2;
  pinMode(encoderSelectButton, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_PIN, INPUT);
  pinMode(inputEncoderA, INPUT);
  pinMode(inputEncoderB, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(encoderSelectButton), handleEncoderSelectInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_PIN), handleLimitSwitchInterrupt, CHANGE);

  handleLimitSwitchInterrupt();
  // PWM1 = new RP2040_PWM(STEP_PIN_MOTOR_1, 0, 50.0);  // pin, freq, duty (%)
  // PWM2 = new RP2040_PWM(STEP_PIN_MOTOR_2, 0, 50.0);  // different pin, freq, duty

  // Start with some default frequency
  add_repeating_timer_us(-10, timerCallback, nullptr, &heartbeat_timer);



  Serial.println("Reading data from flash...");
  readFlashData();
  totalPresetSelectOptions = flashData.presetCount + 2;  // One for the back button, one for the + button


  analogWrite(HEATER_PIN, 0);
  analogWrite(FAN_PIN, 0);

  initializeSteppers(MICROSTEP_1, MICROSTEP_2);

  disableSteppers();

  smoothLed.begin();

  previousStateEncoderA = digitalRead(inputEncoderA);  // Initialize state
}

void loop() {
  smoothLed.update();


  currentMillis = millis();

  // Check if it's time to refresh
  if ((currentMillis - lastStateTransitionMillis >= 50) && (!transitionComplete)) {
    encoderButtonPressed = false;  // reset the button flag
    transitionComplete = true;     // Reset timer
    screenRefreshFlag = true;      // Trigger screen refresh
  }

  if (doorState) {
    smoothLed.trigger();
  }


  checkSensorData();
  calculateRateLimitedValues();
  doSteppers();

  doHeaterControl();
  checkAndDoHeaterFan();


  switch (CURRENT_SCREEN_STATE) {
      // Serial.print(limitedInnerSpeedSetpoint);
      // Serial.print(", ");
      // Serial.println(limitedOuterSpeedSetpoint);
    case HOME:
      limitedInnerSpeedSetpoint = 0;
      limitedOuterSpeedSetpoint = 0;

      if (millis() - lastScreenRefreshMillis >= automaticScreenRefreshInterval) {
        lastScreenRefreshMillis = millis();  // Reset timer correctly
        screenRefreshFlag = true;            // Set flag to refresh screen
      }


      handleHomeInput();
      drawHomeScreen();
      break;

    case SENSOR_SCREEN:
      updateSensorArray();
      handleSensorScreenInput();
      drawSensorScreen();
      break;

    case PREHEAT:
      handlePreheatInput();
      drawPreheatScreen();

      break;

    case PRESET_SELECT:
      // Handle preset select screen
      handlePresetSelectInput();
      drawPresetSelectScreen();
      break;

    case PRESET_ADD:

      handlePresetAddInput();
      drawPresetAddScreen();
      break;

    case PRESET_INFO:
      handlePresetInfoInput();
      drawPresetInfoScreen();

      break;


    case OPERATION:
      handleOperationInput();
      drawOperationScreen();
      refreshOperationAnimationCalculations(333);  // resets the refresh flag every so often (important for animations)
      makeOperationCalculations();

      break;


    case QUICK_START:
      // Handle quick start screen
      handleQuickStartInput();
      drawQuickStartScreen();
      break;

    case CONFIRM_PRESET_INFO_START:
      handleConfirmPresetInfoStartInput();
      drawConfirmPresetInfoStartScreen();
      break;

    case CONFIRM_QUICK_START:
      handleConfirmQuickStartInput();
      drawConfirmQuickStartScreen();
      break;

    case CONFIRM_PRESET:
      handleConfirmPresetInput();
      drawConfirmPresetScreen();
      break;

    case CONFIRM_DELETE:
      handleConfirmDeleteInput();
      drawConfirmDeleteScreen();
      break;

    case WARNING:
      break;

    case FINISHED_OPERATION:
      buzzer.loop(); // must be called in loop

      handleFinishedOperationInput();
      drawFinishedOperationScreen();
      break;
  }
}



#define debounceTime 2




void processButtonPress() {
  encoderButtonPressed = false;  // Clear flag after handling
  screenRefreshFlag = true;      // refresh screen on next loop with new information
  transitionComplete = false;
  lastButtonPressMillis = millis();
  smoothLed.trigger();
}

void processButtonScroll() {
  screenRefreshFlag = true;  // refresh screen on next loop with new information
  transitionComplete = false;
  lastButtonPressMillis = millis();
  smoothLed.trigger();
}


void handleLimitSwitchInterrupt() {
  smoothLed.trigger();
  doorState = digitalRead(LIMIT_SWITCH_PIN);

  if (doorState) {
    disableSteppers();
    limitedOuterSpeedSetpoint = 0;
    limitedInnerSpeedSetpoint = 0;
    // currentTargetInnerSpeed = 0;
    // currentTargetOuterSpeed = 0;

  } else if (CURRENT_SCREEN_STATE == OPERATION) {
    enableSteppers(1, 1);
    // float ricoInner = (float)(operationSettingValues[1] - operationSettingValues[0]) / totalOperationMillis;
    // float ricoOuter = (float)(operationSettingValues[3] - operationSettingValues[2]) / totalOperationMillis;
    // currentTargetInnerSpeed = operationSettingValues[0] + ricoInner * (currentMillis - operationStartMillis);
    // currentTargetOuterSpeed = operationSettingValues[2] + ricoOuter * (currentMillis - operationStartMillis);
  }

  processButtonScroll();
}

void changeToState(uint8_t state) {
  CURRENT_SCREEN_STATE = state;
  if (CURRENT_SCREEN_STATE == OPERATION) {
    enableSteppers(1, 1);
  }

  limitedInnerSpeedSetpoint = 0;
  limitedOuterSpeedSetpoint = 0;
  currentTargetInnerSpeed = 0;
  currentTargetOuterSpeed = 0;
  previousRateLimitMillis = millis();

  editedPresetInfoFlag = false;  // if exiting presetInfo, and didn't edit anything, set flag back to zero
  screenRefreshFlag = true;      // Trigger screen refresh
  lastStateTransitionMillis = millis();
}

void calculateRateLimitedValues() {
  float dt = (currentMillis - previousRateLimitMillis) / 1e3;
  previousRateLimitMillis = currentMillis;

  limitedInnerSpeedSetpoint = applyRateLimiter(currentTargetInnerSpeed, limitedInnerSpeedSetpoint, maxInnerSpeedRate, dt);
  limitedOuterSpeedSetpoint = applyRateLimiter(currentTargetOuterSpeed, limitedOuterSpeedSetpoint, maxOuterSpeedRate, dt);
  limitedTargetTemperatureSetpoint = applyRateLimiter(currentTargetTemperature, limitedTargetTemperatureSetpoint, maxTemperatureRate, dt);
}

float applyRateLimiter(float input, float current, float maxRate, float dt) {
  float maxDelta = maxRate * dt;
  float delta = input - current;

  if (delta > maxDelta)
    delta = maxDelta;
  else if (delta < -maxDelta)
    delta = -maxDelta;

  return current + delta;
}

void doSteppers() {

  //updateStepSignal();

  static unsigned long lastUpdateTime = 0;
  // Run this block only every 100ms

  if (currentMillis - lastUpdateTime < 10) {
    return;
  }
  lastUpdateTime = currentMillis;

  // Serial.println();
  // Serial.println();
  // Serial.println(limitedOuterSpeedSetpoint);
  // Serial.println();
  // Serial.println();


  if (CURRENT_SCREEN_STATE != OPERATION) {
    disableSteppers();
    //return;
  }

  else if (doorState) {
    disableSteppers();

    //return;
  }
  // doorstate logic

  else {

    enableSteppers(1, 1);
    calculateStepFrequencies(stepFrequency1, stepFrequency2, limitedOuterSpeedSetpoint, limitedInnerSpeedSetpoint, MICROSTEP_1, MICROSTEP_2);
    setFrequencySteppers(stepFrequency1, stepFrequency2);
    //disableSteppers();

    // Serial.print(limitedOuterSpeedSetpoint);
    // Serial.print("\t");
    // Serial.print(stepFrequency1);
  }
}





void checkSensorData() {

  uint16_t rawTemp = thermistor.read();
  thermistorTempValue = rawTemp / 10;


  averagedThermistorTempValue = getSmoothValue(averagedThermistorTempValue, thermistorTempValue, thermistorAlpha);
}

float getSmoothValue(float previousSmoothValue, float currentRawValue, float alpha) {
  return alpha * currentRawValue + (1 - alpha) * previousSmoothValue;
}