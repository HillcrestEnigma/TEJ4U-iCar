// impulse power, forward sustain, turn sustain
const int IMPULSE_POWER[2] = {90, 90}, SUSTAIN_POWER[2] = {40, 40}, TURN_POWER[2] = {60, 60};
// impulse delays
const int STRAIGHT_DELAY=200, TURN_DELAY=100;
// tilt sustain powers
const int RIGHT_SUSTAIN_POWER[2] = {60, 40}, LEFT_SUSTAIN_POWER[2] = {40, 60};
// states
const int WAIT=-1, FORWARD_STATE = 0, FORWARD_TILL_HIT = 6, TILT_RIGHT = 1, TILT_LEFT = 2, LEFT_CORNER = 3, RIGHT_CORNER = 4, TEST_OFF = 5;
// the buffer for when we're off before we start turning left
const int OFF_BUFFER = 1000;

// runs a motor
struct Motor {
  // control (power) pin, out1 point and out2 pin (for directions)
  int ctl, out1, out2;

  // sets up the relevant pins
  Motor(int ctl, int out1, int out2) {
    this->ctl = ctl;
    this->out1 = out1;
    this->out2 = out2;

    pinMode(ctl, OUTPUT);
    pinMode(out1, OUTPUT);
    pinMode(out2, OUTPUT);
  }

  // moves the motor forward with [power] power
  void forward(int power){
    // write power needed to control
    analogWrite(ctl, power);

    // set the direction
    digitalWrite(out1, HIGH);
    digitalWrite(out2, LOW);
  }

  // move the motor backwrad with [power] power
  void backward(int power){
    // write power needed to ctl pin
    analogWrite(ctl, power);

    // set the direction
    digitalWrite(out1, LOW);
    digitalWrite(out2, HIGH);    
  }

  // stops the motor
  void stop(){
    digitalWrite(ctl, LOW);
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);
  }
};

// A drivetrain configuring both motors
struct Drivetrain {
  // left and right motors
  Motor left, right;

  // class constructor
  Drivetrain(Motor& left, Motor& right): left(left), right(right) {}

  // stops the drivetrain
  void stop(){
    left.stop();
    right.stop();
  }

  /* moves the chassis forward
      mode -1: an initial impulse to get motors moving
      mode 0: normal forward fast
      mode 1: moves forward slowly
      mode 2: moves to the right a bit
      mode 3: moves to the left a bit
  */
  static const int IMPULSE = -1, FORWARD_FAST = 0, FORWARD = 1, LEFT = 2, RIGHT = 3;
  void forward(int mode){
    if (mode == IMPULSE) {
      left.forward(IMPULSE_POWER[0]);
      right.forward(IMPULSE_POWER[1]);
      delay(STRAIGHT_DELAY);
    } else if(mode == FORWARD_FAST){
      left.forward(IMPULSE_POWER[0]);
      right.forward(IMPULSE_POWER[1]);
    } else if(mode == FORWARD){
      // slow - adds voltage for momentum, then goes at a slow pace
      left.forward(SUSTAIN_POWER[0]);
      right.forward(SUSTAIN_POWER[1]);
    } else if(mode == RIGHT){
      left.forward(RIGHT_SUSTAIN_POWER[0]);
      right.forward(RIGHT_SUSTAIN_POWER[1]);
    } else if(mode == LEFT){
      left.forward(LEFT_SUSTAIN_POWER[0]);
      right.forward(LEFT_SUSTAIN_POWER[1]);
    } else {
      Serial.println("invalid forward mode: " + mode);
    } 
  }

  // moves the chassis backward, should be called once at the start of the movement
  void backward(int mode){
    if (mode == 0) {
      left.backward(IMPULSE_POWER[0]);
      right.backward(IMPULSE_POWER[1]);
    } else if (mode == -1) {
      // slow - adds voltage for momentum, then goes at a slow pace
      left.backward(IMPULSE_POWER[0]);
      right.backward(IMPULSE_POWER[1]);
      delay(STRAIGHT_DELAY);
      left.backward(SUSTAIN_POWER[0]);
      right.backward(SUSTAIN_POWER[1]);
    } else {
      Serial.println("invalid backward mode: " + mode);
    }
  }

  static const int LOW_TURN=4;
  /* rotates the chassis clockwise
   *  IMPULSE: a synchronous initial impulse to get motors up to speed
   *  LOW_TURN: a slow turn
   */
  void clockwise(int mode){
    if(mode == IMPULSE){
      left.forward(IMPULSE_POWER[0]);
      right.backward(IMPULSE_POWER[1]);
      delay(TURN_DELAY);
    } else if(mode == LOW_TURN){
      left.forward(TURN_POWER[0]);
      right.backward(TURN_POWER[1]);
    } else {
      Serial.println("invalid CW mode: " + mode);
    }
  }

  /*
   * Rotates the car CCW
   * IMPULSE: a synchronous initial impulse to get motors up to speed
   * LOW_TURN: a slow turn
   */
  void counterClockwise(int mode){
    if(mode == IMPULSE) {
      left.backward(IMPULSE_POWER[0]);
      right.forward(IMPULSE_POWER[1]);
      delay(TURN_DELAY);
    } else if(mode == LOW_TURN){
      left.backward(TURN_POWER[0]);
      right.forward(TURN_POWER[1]);
    } else {
      Serial.println("invalid CCW mode: " + mode);
    }
  }
};

struct PhotoResistor{
  const static int NUM_RECORD = 3, RECORD_DELAY = 10;
  int pin;
  int threshold;

  int prev[NUM_RECORD];

  int lasRecord;
  PhotoResistor(int pin, int threshold){
    this->pin = pin;
    this->threshold = threshold;
    this->lasRecord = 0;

    pinMode(pin, INPUT);
  }

  int read(){
    if(millis() - lasRecord > RECORD_DELAY){
      for(int i = 0; i < NUM_RECORD - 1; ++i) prev[i] = prev[i + 1];
      prev[NUM_RECORD - 1] = analogRead(pin);
      lasRecord = millis();
    }
    int tot = 0;
    for(int i : prev) tot += i;
    return tot / NUM_RECORD;
  }
  bool triggered(){
    return read() >= threshold;
  }
};

struct SensorArray {
  PhotoResistor left, centre, right;

  SensorArray(PhotoResistor& left, PhotoResistor& centre, PhotoResistor& right): left(left), centre(centre), right(right) {}
  
  // int& read(){
  //   return {left.read(), centre.read(), right.read()};
  // }
};

struct Car {
  Drivetrain drivetrain;
  SensorArray sensors;

  Car(Drivetrain& drivetrain, SensorArray& sensors): drivetrain(drivetrain), sensors(sensors) {}
};

Motor left(3, 2, 4);
Motor right(9, 7, 8);
// the chassis' drivetrain
Drivetrain drivetrain(left, right);
PhotoResistor leftRes(A2, 155), centerRes(A0, 130), rightRes(A1, 120);
SensorArray sensors(leftRes, centerRes, rightRes);

void printLights(){
  Serial.print(leftRes.triggered());
  Serial.print(" ");
  Serial.print(centerRes.triggered());
  Serial.print(" ");
  Serial.print(rightRes.triggered());
}

String stateType(int state){
  if(state == WAIT) return "wait";
  if(state == FORWARD_STATE) return "forward";
  if(state == TILT_LEFT) return "tilt left";
  if(state == TILT_RIGHT) return "tilt right";
  if(state == RIGHT_CORNER) return "turn right";
  if(state == LEFT_CORNER) return "turn left";
  if(state == TEST_OFF) return "all white";
  if(state == FORWARD_TILL_HIT) return "forward till hit";
}

void setup(){
  Serial.begin(9600);
}

int state = -1;

// the time at which car went off line
int offTime = 0;
void loop() {
  printLights();
  Serial.print(" ");
  Serial.println(stateType(state));
  if (state == WAIT) {
    // waiting to be in a good starting position
    if (millis() > 200 && !leftRes.triggered() && rightRes.triggered() && !centerRes.triggered()) {
      drivetrain.forward(Drivetrain::IMPULSE);
      state = FORWARD_STATE;
    }
  } else if(state == FORWARD_STATE){
    // moving forwards
    drivetrain.forward(Drivetrain::FORWARD);

    // if the left side on line, tilt left
    if(centerRes.triggered()) state = TILT_LEFT;
    if(leftRes.triggered()) state = TILT_RIGHT;
    if(!rightRes.triggered()) {
      // if right side on line
      if(!centerRes.triggered()) {
        drivetrain.clockwise(Drivetrain::IMPULSE);
        state = RIGHT_CORNER;
      }
      else state = TILT_RIGHT;
    }

    // once we're off
    if(leftRes.triggered() && rightRes.triggered() && centerRes.triggered()){
      state = LEFT_CORNER;
      offTime = millis();
    }
  } else if(state == TILT_RIGHT){
    // tilt right
    drivetrain.forward(Drivetrain::RIGHT);

    // if center on line, move forward
    if(!centerRes.triggered()) state = FORWARD_STATE;
    // once we're off
    if(leftRes.triggered() && rightRes.triggered() && centerRes.triggered()){
      state = LEFT_CORNER;
      offTime = millis();
    }
  } else if(state == TILT_LEFT){
    // tilt left
    drivetrain.forward(Drivetrain::LEFT);
    // if center on line, move forward
    if(!centerRes.triggered()) state = FORWARD_STATE;

    // once we're off
    if(leftRes.triggered() && rightRes.triggered() && centerRes.triggered()){
      state = LEFT_CORNER;
      offTime = millis();
    }
  } else if(state == RIGHT_CORNER){
    // turn right
    drivetrain.clockwise(Drivetrain::LOW_TURN);

    // if center is off the line, then continue moving forward until we're back on track
    if(centerRes.triggered() && !leftRes.triggered()) {
      drivetrain.forward(Drivetrain::IMPULSE);
      state = FORWARD_STATE;
    }
  } else if(state == LEFT_CORNER){
    drivetrain.counterClockwise(Drivetrain::LOW_TURN);
    if(!centerRes.triggered()) {
      drivetrain.forward(Drivetrain::IMPULSE);
      state = FORWARD_STATE;
    }
  } else if(state == TEST_OFF){
    drivetrain.forward(Drivetrain::LEFT);
    if(millis() - offTime > OFF_BUFFER) {
      drivetrain.counterClockwise(Drivetrain::IMPULSE);
      state = LEFT_CORNER;
    }
    if(!centerRes.triggered()){
      state = FORWARD_STATE;
    }
  }
}
