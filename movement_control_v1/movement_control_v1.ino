const int IMPULSE[2] = {80, 80}, SUSTAIN[2] = {40, 40}, STRAIGHT_DELAY=300;
const int RIGHT_SUSTAIN[2] = {50, 40}, LEFT_SUSTAIN[2] = {40, 50};
const int DELAY_90 = 500;
const int WAIT=-1, FORWARD_STATE = 0, TILT_RIGHT = 1, TILT_LEFT = 2, TURN_LEFT = 3, TURN_RIGHT = 4;

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
  }
};

// A drivetrain configuring both motors
struct Drivetrain {
  // left and right motors
  Motor left, right;

  // class constructor
  Drivetrain(Motor left, Motor right){
    this->left = left;
    this->right = right;

    left.setup();
    right.setup();
  }

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
  static const int FORWARD = 1, LEFT = 3, RIGHT = 2;
  void forward(int mode){
    if(mode == -1){
      left.forward(IMPULSE[0]);
      right.forward(IMPULSE[1]);
      delay(STRAIGHT_DELAY);
    } else if(mode == 0){
      left.forward(IMPULSE[0]);
      right.forward(IMPULSE[1]);
    } else if(mode == FORWARD){
      // slow - adds voltage for momentum, then goes at a slow pace
      left.forward(SUSTAIN[0]);
      right.forward(SUSTAIN[1]);
    } else if(mode == RIGHT){
      left.forward(RIGHT_SUSTAIN[0]);
      right.forward(RIGHT_SUSTAIN[1]);
    } else if(mode == LEFT){
      left.forward(LEFT_SUSTAIN[0]);
      right.forward(LEFT_SUSTAIN[1]);
    } else {
      Serial.println("invalid forward mode: " + mode);
    } 
  }

  // moves the chassis backward, should be called once at the start of the movement
  void backward(int mode){
    if(mode == 0){
      left.backward(IMPULSE[0]);
      right.backward(IMPULSE[1]);
    } else if(mode == -1){
      // slow - adds voltage for momentum, then goes at a slow pace
      left.backward(IMPULSE[0]);
      right.backward(IMPULSE[1]);
      delay(STRAIGHT_DELAY);
      left.backward(SUSTAIN[0]);
      right.backward(SUSTAIN[1]);
    }else {
      Serial.println("invalid backward mode: " + mode);
    }
  }

  // rotates the chassis clockwise
  void clockwise(int mode){
    if(mode == 0){
      left.forward(IMPULSE[0]);
      right.backward(IMPULSE[1]);
    } else {
      Serial.println("invalid CW mode: " + mode);
    }
  }

  // rotates the chassis counterclockwise
  void counterClockwise(int mode){
    if(mode == 0){
      left.backward(IMPULSE[0]);
      right.forward(IMPULSE[1]);
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
    lasRecord = 0;
  }
  void setup(){
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

  SensorArray(PhotoResistor left, PhotoResistor centre, PhotoResistor right){
    this->left = left;
    this->centre = centre;
    this->right = right;
  }
}

struct Car {
};

Motor left = {3, 2, 4};
Motor right = {9, 7, 8};
// the chassis' drivetrain
Drivetrain drivetrain(left, right);
PhotoResistor leftRes(A2, 200), centerRes(A0, 60), rightRes(A1, 60);

void printLights(){
  Serial.println("LEFT CENTER RIGHT");
  Serial.print(leftRes.triggered());
  Serial.print(" ");
  Serial.print(centerRes.triggered());
  Serial.print(" ");
  Serial.println(rightRes.triggered());
}

void setup(){
  drivetrain.setup();
  leftRes.setup();
  rightRes.setup();
  Serial.begin(9600);
}

int state = -1;
const int WAIT=-1, FORWARD_STATE = 0, TILT_RIGHT = 1, TILT_LEFT = 2, TURN_LEFT = 3, TURN_RIGHT = 4;
void loop() {
  printLights();
  if (state == WAIT) {
    if (millis() > 200 && leftRes.triggered() && rightRes.triggered() && !centerRes.triggered()) {
      drivetrain.forward(-1);
      state = FORWARD_STATE;
    }
  } else if(state == FORWARD_STATE){
    drivetrain.forward(Drivetrain::FORWARD);
    if(!leftRes.triggered()) state = TILT_LEFT;
    if(!rightRes.triggered()) state = TILT_RIGHT;
  } else if(state == TILT_RIGHT){
    drivetrain.forward(Drivetrain::RIGHT);
    if(!centerRes.triggered()) state = FORWARD_STATE;
  } else if(state == TILT_LEFT){
    drivetrain.forward(Drivetrain::LEFT);
    if(!centerRes.triggered()) state = FORWARD_STATE;
  }
}