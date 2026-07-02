#include <SPI.h>
#include <RF24.h>

#define ESTOP 20
#define ROTATOR_STEP 14
#define ROTATOR_DIR 15
#define TILT_STEP 5
#define TILT_DIR 4
#define CLAW_STEP 3
#define CLAW_DIR 2
#define SWING_STEP 22
#define SWING_DIR 23
#define BOTTOMRIGHTAXISX A0
#define BOTTOMRIGHTAXISY A1
#define UPPERRIGHTAXISX A3
#define UPPERRIGHTAXISY A2
#define BOTTOMLEFTAXISX A4
#define BOTTOMLEFTAXISY A5
#define UPPERLEFTAXISX A6
#define UPPERLEFTAXISY A7
#define ARPWM 44
#define ALPWM 12
#define BRPWM 8
#define BLPWM 9
#define CRPWM 7
#define CLPWM 6
#define DRPWM 10
#define DLPWM 11

RF24 radio(48,49);
const byte address[6] = "Bagger";

volatile unsigned long lastInterrupt = 0;
volatile bool eStop = false;

int stepDelay = 500;

long lastRead = 0;
int bottomRightX;
int bottomRightY;
int upperRightX;
int upperRightY;
int bottomLeftX;
int bottomLeftY;
int upperLeftX;
int upperLeftY;

int maxJoyValue = 1023;
int minJoyValue = 0;
int joyMiddle = maxJoyValue / 2;
int deadzone = 50;

int maxSpeed = 200;
int minSpeed = 0;
int rampStep = 3;

int currentSpeedA = 0;
int currentSpeedB = 0;
int currentSpeedC = 0;
int currentSpeedD = 0;

struct JoyData
{
  uint16_t bottomRightX;
  uint16_t bottomRightY;
  uint16_t upperRightX;
  uint16_t upperRightY;
  uint16_t bottomLeftX;
  uint16_t bottomLeftY;
  uint16_t upperLeftX;
  uint16_t upperLeftY;
};

struct Stepper {
  int stepPin;
  int dirPin;
  int dir;
  unsigned long lastStepTime;
  int stepDelay;
  bool stepState;
};

Stepper rotator = {ROTATOR_STEP, ROTATOR_DIR, HIGH, 0, stepDelay, false};
Stepper tilt    = {TILT_STEP, TILT_DIR, HIGH, 0, stepDelay, false};
Stepper claw    = {CLAW_STEP, CLAW_DIR, HIGH, 0, stepDelay, false};
Stepper swing   = {SWING_STEP, SWING_DIR, HIGH, 0, stepDelay, false};

void isr_stop() {
  unsigned long now = millis();
  if (now - lastInterrupt > 200) {
    eStop = !eStop;
    lastInterrupt = now;
  }
}

void radioSetup() {
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1,address);
  radio.startListening();
}

void setup() {
  pinMode(ESTOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ESTOP), isr_stop, FALLING);
  pinMode(ROTATOR_STEP, OUTPUT);
  pinMode(ROTATOR_DIR, OUTPUT);
  pinMode(TILT_STEP, OUTPUT);
  pinMode(TILT_DIR, OUTPUT);
  pinMode(CLAW_STEP, OUTPUT);
  pinMode(CLAW_DIR, OUTPUT);
  pinMode(SWING_STEP, OUTPUT);
  pinMode(SWING_DIR, OUTPUT);
  pinMode(ARPWM, OUTPUT);
  pinMode(ALPWM, OUTPUT);
  pinMode(BRPWM, OUTPUT);
  pinMode(BLPWM, OUTPUT);
  pinMode(CRPWM, OUTPUT);
  pinMode(CLPWM, OUTPUT);
  pinMode(DRPWM, OUTPUT);
  pinMode(DLPWM, OUTPUT);
  Serial.begin(9600);

  //radioSetup();
}

void updateStepper(Stepper &s) {
  if (eStop) return;
  if (s.stepDelay == 0) return;
  unsigned long now = micros();
  if (now - s.lastStepTime >= s.stepDelay) {
    s.lastStepTime = now;
    digitalWrite(s.dirPin, s.dir);
    s.stepState = !s.stepState;
    digitalWrite(s.stepPin, s.stepState);
  }
}

int moveActuator(int joyValue, int currentSpeed, int RPWM, int LPWM) {
  if(eStop) return;
  int targetSpeed = 0;
  if (joyValue > joyMiddle + deadzone) {
    targetSpeed = map(joyValue, joyMiddle + deadzone, maxJoyValue, minSpeed, maxSpeed);
  } 
  else if (joyValue < joyMiddle - deadzone) {
    targetSpeed = -map(joyValue, minJoyValue, joyMiddle - deadzone, maxSpeed, minSpeed);
  }

  // Soft-Start
  if (currentSpeed < targetSpeed) {
    currentSpeed += rampStep;
  } else if (currentSpeed > targetSpeed) {
    currentSpeed -= rampStep;
  }

  currentSpeed = constrain(currentSpeed, -maxSpeed, maxSpeed);

  if (abs(currentSpeed) < rampStep) {
    currentSpeed = 0;
  }

  // Motorsteuerung
  if (currentSpeed > 0) {
    analogWrite(RPWM, currentSpeed);
    analogWrite(LPWM, 0);
  } 
  else if (currentSpeed < 0) {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, -currentSpeed);
  } 
  else {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
  }

  return currentSpeed;
}

void readJoyValues() {
  bottomRightX = analogRead(BOTTOMRIGHTAXISX);
  bottomRightY = analogRead(BOTTOMRIGHTAXISY);
  upperRightX = analogRead(UPPERRIGHTAXISX);
  upperRightY = analogRead(UPPERRIGHTAXISY);
  bottomLeftX = analogRead(BOTTOMLEFTAXISX);
  bottomLeftY = analogRead(BOTTOMLEFTAXISY);
  upperLeftX = analogRead(UPPERLEFTAXISX);
  upperLeftY = analogRead(UPPERLEFTAXISY);
}

void readJoyValuesRadio() {
  if(radio.available())
    {
      JoyData data;
      radio.read(&data,sizeof(data));
      bottomRightX = data.bottomRightX;
      bottomRightY = data.bottomRightY;
      upperRightX = data.upperRightX;
      upperRightY = data.upperRightY;
      bottomLeftX = data.bottomLeftX;
      bottomLeftY = data.bottomLeftY;
      upperLeftX = data.upperLeftX;
      upperLeftY = data.upperLeftY;
    }
}

int getSpeedFromJoystick(int joyValue) {
  int offset = joyValue - joyMiddle;

  if (abs(offset) < deadzone) {
    return 0;
  }

  if (offset > 0) {
    return map(offset, deadzone, 512, minSpeed, maxSpeed);
  } else {
    return map(offset, -deadzone, -512, minSpeed, -maxSpeed);
  }
}

void printJoyValues() {
  Serial.print("BRX-Axis: ");
  Serial.print(bottomRightX);
  Serial.print(" | BRY-Axis: ");
  Serial.print(bottomRightY);
  Serial.print(" | URX-Axis: ");
  Serial.print(upperRightX);
  Serial.print(" | URY-Axis: ");
  Serial.print(upperRightY);
  Serial.print(" | BLX-Axis: ");
  Serial.print(bottomLeftX);
  Serial.print(" | BLY-Axis: ");
  Serial.print(bottomLeftY);
  Serial.print(" | ULX-Axis: ");
  Serial.print(upperLeftX);
  Serial.print(" | ULY-Axis: ");
  Serial.print(upperLeftY);
  Serial.println(" ");
}

void loop() {
  if (millis() - lastRead > 10) {
    readJoyValues();
    //readJoyValuesRadio();
    //printJoyValues();
    lastRead = millis();
  }

  currentSpeedA = moveActuator(upperRightY, currentSpeedA, ARPWM, ALPWM);
  currentSpeedB = moveActuator(bottomLeftY, currentSpeedB, BRPWM, BLPWM);
  currentSpeedC = moveActuator(upperLeftY, currentSpeedC, CRPWM, CLPWM);
  currentSpeedD = moveActuator(upperRightX, currentSpeedD, DRPWM, DLPWM);

  if(bottomRightX > joyMiddle + deadzone) {
    rotator.dir = HIGH;
    rotator.stepDelay = stepDelay;
  } else if(bottomRightX < joyMiddle - deadzone) {
    rotator.dir = LOW;
    rotator.stepDelay = stepDelay;
  } else {
    rotator.stepDelay = 0;
  }

  if(bottomRightY > joyMiddle + deadzone) {
    tilt.dir = HIGH;
    tilt.stepDelay = stepDelay;
  } else if(bottomRightY < joyMiddle - deadzone) {
    tilt.dir = LOW;
    tilt.stepDelay = stepDelay;
  } else {
    tilt.stepDelay = 0;
  }

  if(bottomLeftX > joyMiddle + deadzone) {
    claw.dir = HIGH;
    claw.stepDelay = stepDelay;
  } else if(bottomLeftX < joyMiddle - deadzone) {
    claw.dir = LOW;
    claw.stepDelay = stepDelay;
  } else {
    claw.stepDelay = 0;
  }

  if(upperLeftX > joyMiddle + deadzone) {
    swing.dir = HIGH;
    swing.stepDelay = stepDelay;
  } else if(upperLeftX < joyMiddle - deadzone) {
    swing.dir = LOW;
    swing.stepDelay = stepDelay;
  } else {
    swing.stepDelay = 0;
  }

  updateStepper(rotator);
  updateStepper(tilt);
  updateStepper(claw);
  updateStepper(swing);
}
