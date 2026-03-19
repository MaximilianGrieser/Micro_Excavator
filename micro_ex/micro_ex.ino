
#define ESTOP 1
#define ROTATOR_STEP 7
#define ROTATOR_DIR 6
#define TILT_STEP 5
#define TILT_DIR 4
#define CLAW_STEP 3
#define CLAW_DIR 2
#define BOTTOMRIGHTAXISX A0
#define BOTTOMRIGHTAXISY A1
#define UPPERRIGHTAXISX A2
#define UPPERRIGHTAXISY A3
#define BOTTOMLEFTAXISX A4
#define BOTTOMLEFTAXISY A5
#define UPPERLEFTAXISX A6
#define UPPERLEFTAXISY A7

volatile unsigned long lastInterrupt = 0;
volatile bool eStop = false;
volatile unsigned int stepDelay = 500;
volatile unsigned int maxJoyValue = 1024;
volatile unsigned int joyMiddle = maxJoyValue / 2;
volatile unsigned int deadzone = 50;

void setup() {
  pinMode(ESTOP, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(ESTOP), isr_stop, FALLING);
  pinMode(ROTATOR_STEP, OUTPUT);
  pinMode(ROTATOR_DIR, OUTPUT);
  pinMode(TILT_STEP, OUTPUT);
  pinMode(TILT_DIR, OUTPUT);
  pinMode(CLAW_STEP, OUTPUT);
  pinMode(CLAW_DIR, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (!eStop) {
    int bottomRightX = analogRead(BOTTOMRIGHTAXISX);
    int bottomRightY = analogRead(BOTTOMRIGHTAXISY);
    int upperRightX = analogRead(UPPERRIGHTAXISX);
    int upperRightY = analogRead(UPPERRIGHTAXISY);
    int bottomLeftX = analogRead(BOTTOMLEFTAXISX);
    int bototmLeftY = analogRead(BOTTOMLEFTAXISY);
    int upperLeftX = analogRead(UPPERLEFTAXISX);
    int upperLeftY = analogRead(UPPERLEFTAXISY);
    /*Serial.print("BRX-Axis: ");
    Serial.print(bottomRightX);
    Serial.print(" | BRY-Axis: ");
    Serial.print(bottomRightY);
    Serial.print(" | URX-Axis: ");
    Serial.print(upperRightX);
    Serial.print(" | URY-Axis: ");
    Serial.print(upperRightY);
    Serial.print(" | BLX-Axis: ");
    Serial.println(bottomLeftX);
    Serial.print(" | BLY-Axis: ");
    Serial.println(bototmLeftY);
    Serial.print(" | ULX-Axis: ");
    Serial.print(upperLeftX);
    Serial.print(" | ULY-Axis: ");
    Serial.print(upperLeftY);
    Serial.println(" ");*/

    if(bottomRightX > joyMiddle + deadzone) {
      digitalWrite(ROTATOR_DIR, HIGH);
      digitalWrite(ROTATOR_STEP, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(ROTATOR_STEP, LOW);
      delayMicroseconds(stepDelay);
    }
    if(bottomRightX < joyMiddle - deadzone) {
      digitalWrite(ROTATOR_DIR, LOW);
      digitalWrite(ROTATOR_STEP, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(ROTATOR_STEP, LOW);
      delayMicroseconds(stepDelay);
    }

    if(bottomRightY > joyMiddle + deadzone) {
      digitalWrite(TILT_DIR, HIGH);
      digitalWrite(TILT_STEP, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(TILT_STEP, LOW);
      delayMicroseconds(stepDelay);
    }
    if(bottomRightY < joyMiddle - deadzone) {
      digitalWrite(TILT_DIR, LOW);
      digitalWrite(TILT_STEP, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(TILT_STEP, LOW);
      delayMicroseconds(stepDelay);
    }

    if(bottomLeftX > joyMiddle + deadzone) {
      digitalWrite(CLAW_DIR, HIGH);
      digitalWrite(CLAW_STEP, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(CLAW_STEP, LOW);
      delayMicroseconds(stepDelay);
    }
    if(bottomLeftX < joyMiddle - deadzone) {
      digitalWrite(CLAW_DIR, LOW);
      digitalWrite(CLAW_STEP, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(CLAW_STEP, LOW);
      delayMicroseconds(stepDelay);
    }

  } else {
    Serial.println("Interrupt");
  }
}

void isr_stop() {
  unsigned long now = millis();
  if (now - lastInterrupt > 200) {
    eStop = !eStop;
    lastInterrupt = now;
  }
}
