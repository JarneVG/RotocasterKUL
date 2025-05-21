#define freqThreshold 0

void initializeSteppers(uint8_t microStep1 = 2, uint8_t microStep2 = 2) {
  // Initialize EN and DIR pins.
  pinMode(EN_PIN_MOTOR_1, OUTPUT);
  pinMode(DIR_PIN_MOTOR_1, OUTPUT);
  pinMode(EN_PIN_MOTOR_2, OUTPUT);
  pinMode(DIR_PIN_MOTOR_2, OUTPUT);




  SPI.begin();
  pinMode(MISO, INPUT_PULLUP);

  driver_1.begin();           // Initiate pins and registeries
  driver_1.rms_current(MAX_MILLIAMPS_MOTOR_1);  // Set stepper current to 600mA
  driver_1.stealthChop(1);    // Enable extremely quiet stepping
  driver_1.microsteps(MICROSTEP_1);

  driver_2.begin();           // Initiate pins and registeries
  driver_2.rms_current(MAX_MILLIAMPS_MOTOR_2);  // Set stepper current to 600mA
  driver_2.stealthChop(1);    // Enable extremely quiet stepping
  driver_2.microsteps(MICROSTEP_2);






  // Optionally, initialize them to a known state.
  digitalWrite(EN_PIN_MOTOR_1, HIGH);  // Disabled initially.
  digitalWrite(EN_PIN_MOTOR_2, HIGH);  // Disabled initially.
  setStepperDirections(CW, CW);
}


void setFrequencySteppers(float freq1, float freq2) {
  bool dir1 = CW;
  bool dir2 = CW;
  if (freq1 < 0) {
    dir1 = CCW;
    freq1 = -freq1;
  }
  if (freq2 < 0) {
    dir2 = CCW;
    freq2 = -freq2;
  }
  setStepperDirections(dir1, dir2);
  Serial.print(6000);
  Serial.print("\t");

  Serial.print(freq1);
  Serial.print("\t");
  Serial.println(freq2);
  freq_motor_1 = freq1;
  period_motor_1_us = 1e6 / freq1;
  freq_motor_2 = freq2;
  period_motor_2_us = 1e6 / freq2;
  // PWM1->setPWM(STEP_PIN_MOTOR_1, freq1, 50.0);
  // PWM2->setPWM(STEP_PIN_MOTOR_2, freq2, 50.0);
}

void enableSteppers(int en1 = 1, int en2 = 1) {
  // Zero is on, 1 is off
  digitalWrite(EN_PIN_MOTOR_1, !en1);
  digitalWrite(EN_PIN_MOTOR_2, !en2);
}

void disableSteppers() {
  digitalWrite(EN_PIN_MOTOR_1, HIGH);
  digitalWrite(EN_PIN_MOTOR_2, HIGH);
}

void setStepperDirections(int dir1, int dir2) {
  digitalWrite(DIR_PIN_MOTOR_1, dir1);
  digitalWrite(DIR_PIN_MOTOR_2, dir2);
}


#define MOTOR_STEPS_PER_ROT 180
#define BELT_TRANSMISSION_RATIO 3

// Motor1 maps to inner frame, motor2 maps to outer frame
void calculateStepFrequencies(float &stepFrequency1, float &stepFrequency2, float desiredOuterFrameDPS, float desiredInnerFrameDPS, uint8_t microStep1, uint8_t microStep2) {
  // Conver
  float desiredOuterFrameHz = desiredOuterFrameDPS / 360;
  float desiredInnerFrameHz = desiredInnerFrameDPS / 360;

  // Serial.println(desiredOuterFrameRPM);
  // Serial.println(desiredInnerFrameRPM);
  // Serial.println();

  float desiredOuterMotorShaftHz = desiredOuterFrameHz * BELT_TRANSMISSION_RATIO;
  //float desiredInnerMotorShaftHz = (desiredInnerFrameHz)*BELT_TRANSMISSION_RATIO;

  float desiredInnerMotorShaftHz = (-desiredOuterFrameHz + desiredInnerFrameHz) * BELT_TRANSMISSION_RATIO;


  // stepFrequency1 = constrain(stepFrequency1, 0, 1000);
  // stepFrequency2 = constrain(stepFrequency2, 0, 1000);

  stepFrequency2 = (desiredOuterMotorShaftHz * MOTOR_STEPS_PER_ROT * MICROSTEP_2);
  stepFrequency1 = (desiredInnerMotorShaftHz * MOTOR_STEPS_PER_ROT * MICROSTEP_1);
}



/*
void updateStepSignal() {

  static unsigned long lastStep1 = 0;
  static unsigned long lastStep2 = 0;
  static bool stepState1 = false;
  static bool stepState2 = false;

  unsigned long now = micros();



  if (bitBangStepperActive1) {

    if (now - lastStep1 >= microsInterval1) {
      lastStep1 = now;
      stepState1 = !stepState1;
      digitalWriteFast(STEP_PIN_MOTOR_1, stepState1);
    }
  }
  if (bitBangStepperActive2) {
    if (now - lastStep2 >= microsInterval2) {
      lastStep2 = now;
      stepState2 = !stepState2;
      digitalWriteFast(STEP_PIN_MOTOR_2, stepState2);
    }
  }
}
*/
