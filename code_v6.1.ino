// ==========================================
//      MOTOR PINS
// ==========================================
const int ENA = 5;  const int IN1 = 24; const int IN2 = 25;
const int ENB = 2;  const int IN3 = 26; const int IN4 = 27;
const int ENC = 4;  const int IN5 = 30; const int IN6 = 31;
const int END = 3;  const int IN7 = 28; const int IN8 = 29;

// ==========================================
//      SENSOR PINS
// ==========================================
const int SENS_F_L = A0; const int SENS_F_C = A1; const int SENS_F_R = A2;
const int SENS_L_F = A6; const int SENS_L_C = A7; const int SENS_L_B = A8;
const int SENS_R_F = A3; const int SENS_R_C = A4; const int SENS_R_B = A5;

// ==========================================
//      PATH — EDIT THIS ONLY
// ==========================================
enum Move { FORWARD, BACKWARD, STRAFE_RIGHT, STRAFE_LEFT };

struct Step {
  Move direction;
  int junctions;
};

Step path[] = {
  { FORWARD,      3 },
  { STRAFE_RIGHT, 2 }   // stops completely after this
};

int totalSteps  = 2;
int currentStep = 0;

// ==========================================
//      SETTINGS
// ==========================================
int motorSpeed  = 180;
int strafeSpeed = 120;

// ==========================================
//      TRACKING
// ==========================================
int  junctionCount = 0;
bool inJunction    = false;

// ==========================================
//      SETUP
// ==========================================
void setup() {
  Serial.begin(9600);

  pinMode(ENA,OUTPUT); pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  pinMode(ENB,OUTPUT); pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
  pinMode(ENC,OUTPUT); pinMode(IN5,OUTPUT); pinMode(IN6,OUTPUT);
  pinMode(END,OUTPUT); pinMode(IN7,OUTPUT); pinMode(IN8,OUTPUT);

  pinMode(SENS_F_L,INPUT); pinMode(SENS_F_C,INPUT); pinMode(SENS_F_R,INPUT);
  pinMode(SENS_L_F,INPUT); pinMode(SENS_L_C,INPUT); pinMode(SENS_L_B,INPUT);
  pinMode(SENS_R_F,INPUT); pinMode(SENS_R_C,INPUT); pinMode(SENS_R_B,INPUT);

  Serial.println("Starting in 3 seconds...");
  delay(3000);
  Serial.println("GO!");
}

// ==========================================
//      MAIN LOOP
// ==========================================
void loop() {

  // read all sensors
  int fl = digitalRead(SENS_F_L);
  int fc = digitalRead(SENS_F_C);
  int fr = digitalRead(SENS_F_R);
  int lf = digitalRead(SENS_L_F);
  int lc = digitalRead(SENS_L_C);
  int lb = digitalRead(SENS_L_B);
  int rf = digitalRead(SENS_R_F);
  int rc = digitalRead(SENS_R_C);
  int rb = digitalRead(SENS_R_B);

  // live debug
  Serial.print("STEP:"); Serial.print(currentStep+1);
  Serial.print(" DIR:");
  if(path[currentStep].direction==FORWARD)      Serial.print("FWD  ");
  if(path[currentStep].direction==BACKWARD)     Serial.print("BWD  ");
  if(path[currentStep].direction==STRAFE_RIGHT) Serial.print("STR_R");
  if(path[currentStep].direction==STRAFE_LEFT)  Serial.print("STR_L");
  Serial.print(" JCT:"); Serial.print(junctionCount);
  Serial.print("/");     Serial.print(path[currentStep].junctions);
  Serial.print(" | F:["); Serial.print(fl); Serial.print(fc); Serial.print(fr); Serial.print("]");
  Serial.print(" L:["); Serial.print(lf); Serial.print(lc); Serial.print(lb); Serial.print("]");
  Serial.print(" R:["); Serial.print(rf); Serial.print(rc); Serial.print(rb); Serial.print("]");
  Serial.println();

  // --- ALL STEPS DONE ---
  if (currentStep >= totalSteps) {
    stopRobot();
    Serial.println("PATH COMPLETE. STOPPED.");
    while(true);
  }

  Step s = path[currentStep];

  // ==========================================
  //  JUNCTION DETECTION
  // ==========================================
  bool junctionHit = false;

  if (s.direction==FORWARD || s.direction==BACKWARD) {
    if (fl==HIGH && fc==HIGH && fr==HIGH) junctionHit = true;
  }
  else if (s.direction==STRAFE_RIGHT) {
    if (lf==HIGH && lc==HIGH && lb==HIGH) junctionHit = true;
  }
  else if (s.direction==STRAFE_LEFT) {
    if (rf==HIGH && rc==HIGH && rb==HIGH) junctionHit = true;
  }

  // ==========================================
  //  JUNCTION COUNTING & STEP SWITCHING
  // ==========================================
// ==========================================
  //  JUNCTION COUNTING & STEP SWITCHING
  // ==========================================
  if (junctionHit) {
    if (!inJunction) {
      inJunction = true;
      junctionCount++;
      Serial.print(">>> JUNCTION HIT #"); Serial.print(junctionCount);
      Serial.print(" / "); Serial.println(s.junctions);

      if (junctionCount >= s.junctions) {
        
        // --- THE FIX: CENTER ALIGNMENT ---
        // Continue moving for a short time to push the robot's center 
        // perfectly over the cross-line before stopping.
        executeMove(s.direction);
        delay(200); // <-- Adjust this value (e.g., 100 - 300) based on your robot's speed and sensor placement
        
        // always stop completely at end of every step
        stopRobot();
        Serial.println(">>> STEP DONE. STOPPED.");

        // last step — end completely
        if (currentStep == totalSteps-1) {
          Serial.println("PATH COMPLETE. STOPPED.");
          while(true);
        }

        // not last step — pause then switch
        delay(400);
        junctionCount = 0;
        inJunction    = false;
        currentStep++;
        Serial.print(">>> Switching to step ");
        Serial.println(currentStep+1);
        return;
      }
    }
    executeMove(s.direction);
  }
  else {
    inJunction = false;
    followLine(s.direction);
  }
}
// ==========================================
//  LINE FOLLOWING
// ==========================================
void followLine(Move dir) {

  int fl = digitalRead(SENS_F_L);
  int fc = digitalRead(SENS_F_C);
  int fr = digitalRead(SENS_F_R);
  int lf = digitalRead(SENS_L_F);
  int lc = digitalRead(SENS_L_C);
  int lb = digitalRead(SENS_L_B);
  int rf = digitalRead(SENS_R_F);
  int rc = digitalRead(SENS_R_C);
  int rb = digitalRead(SENS_R_B);

  if (dir==FORWARD || dir==BACKWARD) {
    if      (fc==HIGH && fl==LOW && fr==LOW) executeMove(dir);
    else if (fl==HIGH && fc==LOW && fr==LOW) { strafeLeft();  delay(30); executeMove(dir); }
    else if (fr==HIGH && fc==LOW && fl==LOW) { strafeRight(); delay(30); executeMove(dir); }
    else if (fl==HIGH && fc==HIGH)           { strafeLeft();  delay(20); executeMove(dir); }
    else if (fr==HIGH && fc==HIGH)           { strafeRight(); delay(20); executeMove(dir); }
    else                                       slowMove(dir);
  }

  else if (dir==STRAFE_RIGHT) {
    if      (lc==HIGH && lf==LOW && lb==LOW) strafeRight();
    else if (lf==HIGH && lb==LOW)            { moveForward();  delay(20); strafeRight(); }
    else if (lb==HIGH && lf==LOW)            { moveBackward(); delay(20); strafeRight(); }
    else                                       strafeRight();
  }

  else if (dir==STRAFE_LEFT) {
    if      (rc==HIGH && rf==LOW && rb==LOW) strafeLeft();
    else if (rf==HIGH && rb==LOW)            { moveForward();  delay(20); strafeLeft(); }
    else if (rb==HIGH && rf==LOW)            { moveBackward(); delay(20); strafeLeft(); }
    else                                       strafeLeft();
  }
}

// ==========================================
//  EXECUTE MOVE
// ==========================================
void executeMove(Move dir) {
  if      (dir==FORWARD)      moveForward();
  else if (dir==BACKWARD)     moveBackward();
  else if (dir==STRAFE_RIGHT) strafeRight();
  else if (dir==STRAFE_LEFT)  strafeLeft();
}

void slowMove(Move dir) {
  setSpeeds(120);
  if (dir==FORWARD) {
    setDir(IN1,IN2,HIGH,LOW); setDir(IN3,IN4,HIGH,LOW);
    setDir(IN5,IN6,HIGH,LOW); setDir(IN7,IN8,HIGH,LOW);
  } else {
    setDir(IN1,IN2,LOW,HIGH); setDir(IN3,IN4,LOW,HIGH);
    setDir(IN5,IN6,LOW,HIGH); setDir(IN7,IN8,LOW,HIGH);
  }
}

// ==========================================
//      MOVEMENT FUNCTIONS
// ==========================================
void moveForward() {
  setSpeeds(motorSpeed);
  setDir(IN1,IN2,HIGH,LOW); setDir(IN3,IN4,HIGH,LOW);
  setDir(IN5,IN6,HIGH,LOW); setDir(IN7,IN8,HIGH,LOW);
}

void moveBackward() {
  setSpeeds(motorSpeed);
  setDir(IN1,IN2,LOW,HIGH); setDir(IN3,IN4,LOW,HIGH);
  setDir(IN5,IN6,LOW,HIGH); setDir(IN7,IN8,LOW,HIGH);
}

void strafeRight() {
  setSpeeds(strafeSpeed);
  setDir(IN1,IN2,HIGH,LOW); setDir(IN3,IN4,LOW,HIGH);
  setDir(IN5,IN6,LOW,HIGH); setDir(IN7,IN8,HIGH,LOW);
}

void strafeLeft() {
  setSpeeds(strafeSpeed);
  setDir(IN1,IN2,LOW,HIGH); setDir(IN3,IN4,HIGH,LOW);
  setDir(IN5,IN6,HIGH,LOW); setDir(IN7,IN8,LOW,HIGH);
}

void stopRobot() {
  setSpeeds(0);
  setDir(IN1,IN2,LOW,LOW); setDir(IN3,IN4,LOW,LOW);
  setDir(IN5,IN6,LOW,LOW); setDir(IN7,IN8,LOW,LOW);
}

void setDir(int a, int b, int sa, int sb) {
  digitalWrite(a,sa); digitalWrite(b,sb);
}

void setSpeeds(int s) {
  analogWrite(ENA,s); analogWrite(ENB,s);
  analogWrite(ENC,s); analogWrite(END,s);
}
