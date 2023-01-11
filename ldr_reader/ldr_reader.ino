const int IMPULSE_POWER[2] = {80, 80}, SUSTAIN_POWER[2] = {40, 40}, TURN_POWER[2] = {60, 60}, STRAIGHT_DELAY=300;
const int RIGHT_SUSTAIN_POWER[2] = {50, 40}, LEFT_SUSTAIN_POWER[2] = {40, 50};
const int DELAY_90 = 500;
const int WAIT=-1, FORWARD_STATE = 0, TILT_RIGHT = 1, TILT_LEFT = 2, LEFT_CORNER = 3, RIGHT_CORNER = 4, TEST_OFF = 5;

const int OFF_BUFFER = 100;

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

  // rotates the chassis clockwise
  void clockwise(int mode){
    if(mode == 0){
      left.forward(TURN_POWER[0]);
      right.backward(TURN_POWER[1]);
    } else {
      Serial.println("invalid CW mode: " + mode);
    }
  }

  // rotates the chassis counterclockwise
  void counterClockwise(int mode){
    if(mode == 0){
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
PhotoResistor leftRes(A2, 180), centerRes(A0, 120), rightRes(A1, 120);
SensorArray sensors(leftRes, centerRes, rightRes);

void printLights(){
  Serial.println("LEFT CENTER RIGHT");
  Serial.print(leftRes.read());
  Serial.print(" ");
  Serial.print(centerRes.read());
  Serial.print(" ");
  Serial.println(rightRes.read());
}

void setup(){
  Serial.begin(9600);
}

int state = -1;

// the time at which car went off line
int offTime = 0;
void loop() {
  printLights();
}
