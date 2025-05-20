// BANG BANG constants
// #define KP 0.08817f   // Proportional gain
// #define KI 0.001757f  // Integral gain
// #define KD 0.0f       // Derivative gain

float lowerTempMargin = 3;     // Enable heater again when falling lowerTempMargin degrees under target
float higherTempMargin = 0.5;  // Disable heater again when falling lowerTempMargin degrees under target


// Control settings
#define HEATER_SAMPLE_TIME_MS 100  // Sample time in milliseconds
// #define HEATER_MAX_OUTPUT 1.0f     // Max controller output (normalized)
// #define HEATER_MIN_OUTPUT 0.0f     // Min controller output (normalized)

// float previousError = 0.0f;
// float integralTerm = 0.0f;
// --- State Variables ---
unsigned long lastHeaterControlTime = 0;
// PID constants
#define KP 0.04817f
#define KI 0.001757f
#define KD 5.0f

float previousError = 0.0f;
float integralTerm = 0.0f;



// --- Time-Proportional Output Control ---
unsigned long windowSize = 2000;    // 2 seconds control window
unsigned long windowStartTime = 0;  // when current window started
float pidOutput = 0;                // PID output [0.0 - 1.0]

// Fan control
#define FAN_ON_TIME (10 * 1000)
#define FAN_BLOW_TEMP 25

void doHeaterControl() {
  float temperatureSetpoint;
  if (CURRENT_SCREEN_STATE == OPERATION) {
    temperatureSetpoint = limitedTargetTemperatureSetpoint;
  } else if (preheatEnabledFlag) {
    temperatureSetpoint = preheatTargetTemp;
  } else {
    temperatureSetpoint = 21;
  }
  if (temperatureSetpoint <= 20) {
    pidOutput = 0;
    digitalWrite(HEATER_PIN, LOW);
    return;
  }

  unsigned long now = millis();
  unsigned long elapsedTime = now - lastHeaterControlTime;

  if (elapsedTime >= HEATER_SAMPLE_TIME_MS) {
    lastHeaterControlTime = now;
    lastHeaterCalculationMillis = now;

    float currentTemperature = thermistorTempValue;
    float error = temperatureSetpoint - currentTemperature;

    // Integral term with anti-windup
    integralTerm += error * (elapsedTime / 1000.0f);
    float maxIntegral = (1.0f) / KI;
    if (integralTerm > maxIntegral) integralTerm = maxIntegral;
    else if (integralTerm < -maxIntegral) integralTerm = -maxIntegral;

    float derivative = (error - previousError) / (elapsedTime / 1000.0f);

    // PID output normalized to [0, 1]
    float output = KP * error + KI * integralTerm + KD * derivative;
    if (output > 1.0f) output = 1.0f;
    if (output < 0.0f) output = 0.0f;

    pidOutput = output * windowSize;  // Convert normalized output to ms duration
    previousError = error;
    // Serial.println();
    // Serial.println();

    // Serial.print("Temp: ");
    // Serial.print(currentTemperature);
    // Serial.print(" °C | Output time: ");
    // Serial.print(pidOutput);
    // Serial.println(" ms");

    // Serial.println();
    // Serial.println();
  }

  if (pidOutput < 0.5) {
    digitalWrite(HEATER_PIN, 0);
  }

  else if (pidOutput >= 0.5) {
    digitalWrite(HEATER_PIN, 1);
  }

}


void checkAndDoHeaterFan() {
  uint32_t elapsedMillis = currentMillis - lastHeaterCalculationMillis;
  if ((elapsedMillis > FAN_ON_TIME) || (lastHeaterCalculationMillis == 0xFFFFFFFF)) {  // disable fan after this time
    analogWrite(FAN_PIN, 0);
    heaterFanState = 0;
  } else {
    analogWrite(FAN_PIN, 255);
    heaterFanState = 1;
  }

  if (thermistorTempValue > FAN_BLOW_TEMP) {
    analogWrite(FAN_PIN, 255);
  }
}
