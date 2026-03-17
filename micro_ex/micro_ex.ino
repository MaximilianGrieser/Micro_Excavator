#define ESTOP 2
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

void setup() {
  pinMode(ESTOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ESTOP), isr_stop, FALLING);
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
    Serial.print(bototmLeftY);
    Serial.print(" | ULX-Axis: ");
    Serial.print(upperLeftX);
    Serial.print(" | ULY-Axis: ");
    Serial.print(upperLeftY);
    Serial.println(" ");
  } else {
    Serial.println("Interrupt");
  }

  delay(100);
}

void isr_stop() {
  unsigned long now = millis();
  if (now - lastInterrupt > 200) {
    eStop = !eStop;
    lastInterrupt = now;
  }
}