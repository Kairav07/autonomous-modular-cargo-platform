// =====================================================
// FINAL ESP32 ROBOT CODE
// AUTO LINE FOLLOW + MANUAL RC + ULTRASONIC STOP
// =====================================================


// ---------------- L298N MOTOR PINS ----------------

#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

#define ENA 33
#define ENB 32


// ---------------- LINE SENSOR PINS ----------------

#define IR1 34
#define IR2 35
#define IR3 36   // VP
#define IR4 39   // VN
#define IR5 13


// ---------------- HC-SR04 ----------------

#define TRIG 18
#define ECHO 19

#define STOP_DISTANCE 15


// =====================================================
// LINE SENSOR POLARITY
// =====================================================

// From your latest readings:
// white was appearing as 1.
//
// Therefore start with:
// BLACK = 0
//
// If later your physical test proves otherwise,
// change ONLY this one value.

#define BLACK 0


// =====================================================
// SPEED SETTINGS
// =====================================================

// AUTO mode - start slow and stable
int autoBaseSpeed = 95;
int autoMaxSpeed  = 155;

// proportional correction
float Kp = 22.0;

// MANUAL mode
int manualSpeed = 150;


// =====================================================
// MODE
// =====================================================

// false = manual
// true  = auto

bool autoMode = false;


// =====================================================
// MANUAL SAFETY TIMEOUT
// =====================================================

// Python sends a command repeatedly while a key is held.
// If commands stop for 350 ms, the robot stops.

unsigned long lastManualCommand = 0;
const unsigned long MANUAL_TIMEOUT = 350;


// =====================================================
// LINE MEMORY
// =====================================================

float lastError = 0;


// =====================================================
// MOTOR PWM
// =====================================================

void setMotorPWM(int leftSpeed, int rightSpeed) {

  leftSpeed  = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}


// =====================================================
// STOP
// =====================================================

void stopCar() {

  setMotorPWM(0, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// =====================================================
// FORWARD
// =====================================================

void driveForward(int leftSpeed, int rightSpeed) {

  // Your confirmed forward direction

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  setMotorPWM(leftSpeed, rightSpeed);
}


// =====================================================
// REVERSE
// =====================================================

void driveReverse(int leftSpeed, int rightSpeed) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  setMotorPWM(leftSpeed, rightSpeed);
}


// =====================================================
// MANUAL LEFT
// =====================================================

void manualLeft() {

  // left slower, right faster
  driveForward(45, manualSpeed);
}


// =====================================================
// MANUAL RIGHT
// =====================================================

void manualRight() {

  driveForward(manualSpeed, 45);
}


// =====================================================
// ULTRASONIC DISTANCE
// =====================================================

float getDistance() {

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 25000);

  if (duration == 0) {
    return 999;
  }

  return duration * 0.0343 / 2.0;
}


// =====================================================
// AUTO LINE FOLLOWING
// =====================================================

void followLine() {

  int s1 = digitalRead(IR1);
  int s2 = digitalRead(IR2);
  int s3 = digitalRead(IR3);
  int s4 = digitalRead(IR4);
  int s5 = digitalRead(IR5);


  // Sensor positions:
  //
  // IR1  IR2  IR3  IR4  IR5
  // -4   -2    0    +2   +4

  float weightedTotal = 0;
  int activeSensors = 0;


  if (s1 == BLACK) {
    weightedTotal += -4;
    activeSensors++;
  }

  if (s2 == BLACK) {
    weightedTotal += -2;
    activeSensors++;
  }

  if (s3 == BLACK) {
    weightedTotal += 0;
    activeSensors++;
  }

  if (s4 == BLACK) {
    weightedTotal += 2;
    activeSensors++;
  }

  if (s5 == BLACK) {
    weightedTotal += 4;
    activeSensors++;
  }


  // -------------------------------------------------
  // LINE LOST
  // -------------------------------------------------

  if (activeSensors == 0) {

    if (lastError < 0) {
      // search left
      driveForward(40, 105);
    }

    else if (lastError > 0) {
      // search right
      driveForward(105, 40);
    }

    else {
      stopCar();
    }

    Serial.println("AUTO | LINE LOST");

    return;
  }


  // -------------------------------------------------
  // CALCULATE ERROR
  // -------------------------------------------------

  float error = weightedTotal / activeSensors;

  lastError = error;


  int correction = (int)(Kp * error);


  int leftSpeed  = autoBaseSpeed + correction;
  int rightSpeed = autoBaseSpeed - correction;


  leftSpeed  = constrain(leftSpeed, 35, autoMaxSpeed);
  rightSpeed = constrain(rightSpeed, 35, autoMaxSpeed);


  driveForward(leftSpeed, rightSpeed);


  // -------------------------------------------------
  // DEBUG
  // -------------------------------------------------

  Serial.print("AUTO | IR: ");

  Serial.print(s1);
  Serial.print(" ");
  Serial.print(s2);
  Serial.print(" ");
  Serial.print(s3);
  Serial.print(" ");
  Serial.print(s4);
  Serial.print(" ");
  Serial.print(s5);

  Serial.print(" | ERR: ");
  Serial.print(error);

  Serial.print(" | L: ");
  Serial.print(leftSpeed);

  Serial.print(" R: ");
  Serial.println(rightSpeed);
}


// =====================================================
// SERIAL COMMANDS
// =====================================================
//
// Commands used by Python:
//
// 1 = AUTO MODE
// 2 = MANUAL MODE
//
// Manual:
// w = forward
// s = reverse
// a = left
// d = right
// x = stop
// + = faster
// - = slower
//
// =====================================================

void handleSerial() {

  while (Serial.available()) {

    char c = Serial.read();


    if (c == '\n' || c == '\r') {
      continue;
    }


    // ---------------- AUTO MODE ----------------

    if (c == '1') {

      autoMode = true;

      stopCar();

      Serial.println("MODE: AUTO");

      continue;
    }


    // ---------------- MANUAL MODE ----------------

    if (c == '2') {

      autoMode = false;

      stopCar();

      lastManualCommand = millis();

      Serial.println("MODE: MANUAL");

      continue;
    }


    // -------------------------------------------------
    // MANUAL COMMANDS ONLY
    // -------------------------------------------------

    if (!autoMode) {

      lastManualCommand = millis();


      if (c == 'w') {

        driveForward(manualSpeed, manualSpeed);
      }


      else if (c == 's') {

        driveReverse(manualSpeed, manualSpeed);
      }


      else if (c == 'a') {

        manualLeft();
      }


      else if (c == 'd') {

        manualRight();
      }


      else if (c == 'x') {

        stopCar();
      }


      else if (c == '+') {

        manualSpeed += 10;

        manualSpeed = constrain(manualSpeed, 70, 220);

        Serial.print("MANUAL SPEED: ");
        Serial.println(manualSpeed);
      }


      else if (c == '-') {

        manualSpeed -= 10;

        manualSpeed = constrain(manualSpeed, 70, 220);

        Serial.print("MANUAL SPEED: ");
        Serial.println(manualSpeed);
      }
    }
  }
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);


  // Motors

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);


  // IR sensors

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(IR5, INPUT);


  // Ultrasonic

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);


  stopCar();


  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32 ROBOT READY");
  Serial.println("==============================");
  Serial.println();
  Serial.println("1 = AUTO");
  Serial.println("2 = MANUAL");
  Serial.println();
  Serial.println("MANUAL:");
  Serial.println("W = Forward");
  Serial.println("S = Reverse");
  Serial.println("A = Left");
  Serial.println("D = Right");
  Serial.println("X = Stop");
  Serial.println("+ = Faster");
  Serial.println("- = Slower");
  Serial.println();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  handleSerial();


  // ===================================================
  // MANUAL MODE
  // ===================================================

  if (!autoMode) {

    if (millis() - lastManualCommand > MANUAL_TIMEOUT) {
      stopCar();
    }

    delay(5);

    return;
  }


  // ===================================================
  // AUTO MODE
  // ===================================================

  float distance = getDistance();


  // obstacle stop

  if (distance < STOP_DISTANCE) {

    stopCar();

    Serial.print("AUTO | OBSTACLE: ");
    Serial.print(distance);
    Serial.println(" cm");

    delay(40);

    return;
  }


  followLine();

  delay(5);
}