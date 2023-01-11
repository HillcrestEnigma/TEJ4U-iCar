// impulse power, forward sustain, turn sustain
const int FAST_POWER[2] = {200, 200}, SLOW_POWER[2] = {140, 140}, TURN_POWER[2] = {200, 200};
// tilt sustain powers
const int RIGHT_SUSTAIN_POWER[2] = {110, 70}, LEFT_SUSTAIN_POWER[2] = {130, 170};

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
  static const int FORWARD_FAST = 0, FORWARD = 1, LEFT = 2, RIGHT = 3;
  void forward(int mode){
    if(mode == FORWARD_FAST){
      left.forward(FAST_POWER[0]);
      right.forward(FAST_POWER[1]);
    } else if(mode == FORWARD){
      // slow - adds voltage for momentum, then goes at a slow pace
      left.forward(SLOW_POWER[0]);
      right.forward(SLOW_POWER[1]);
    } else if(mode == RIGHT){
      left.forward(RIGHT_SUSTAIN_POWER[0]);
      right.backward(RIGHT_SUSTAIN_POWER[1]);
    } else if(mode == LEFT){
      left.backward(LEFT_SUSTAIN_POWER[0]);
      right.forward(LEFT_SUSTAIN_POWER[1]);
    } else {
      Serial.println("invalid forward mode: " + mode);
    } 
  }

  // moves the chassis backward, should be called once at the start of the movement
  void backward(int mode){
    if (mode == 0) {
      left.backward(FAST_POWER[0]);
      right.backward(FAST_POWER[1]);
    } else {
      Serial.println("invalid backward mode: " + mode);
    }
  }

  static const int LOW_TURN=4, HIGH_TURN=5;
  /* rotates the chassis clockwise
   *  IMPULSE: a synchronous initial impulse to get motors up to speed
   *  LOW_TURN: a slow turn
   */
  void clockwise(int mode){
    if(mode == LOW_TURN){
      left.forward(TURN_POWER[0]);
      right.backward(TURN_POWER[1]);
    } else if(mode == HIGH_TURN){
      left.forward(SLOW_POWER[0]);
      right.backward(SLOW_POWER[1]);
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
    if(mode == LOW_TURN){
      left.backward(TURN_POWER[0]);
      right.forward(TURN_POWER[1]);
    } else if(mode == HIGH_TURN){
      left.backward(SLOW_POWER[0]);
      right.forward(SLOW_POWER[1]);
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

  long threshSum, readings;
  PhotoResistor(int pin, int threshold){
    this->pin = pin;
    this->threshold = threshold;
    this->lasRecord = 0;
    this->threshSum = 0;
    this->readings = 0;

    pinMode(pin, INPUT);
  }

  void threshRead(){
    threshSum += analogRead(pin);
    readings += 1;
    threshold = threshSum / readings - 30;
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

Motor left(3, 2, 4);
Motor right(9, 7, 8);
// the chassis' drivetrain
Drivetrain drivetrain(left, right);
PhotoResistor frontRes(A2, 180), centerRes(A0, 70), rightRes(A1, 120);

void printLights(){
  Serial.print(frontRes.read());
  Serial.print(" ");
  Serial.print(centerRes.read());
  Serial.print(" ");
  Serial.print(rightRes.read());
}

void setup(){
  Serial.begin(9600);

  for(int i = 0; i < 100; i++){
    frontRes.threshRead();
    centerRes.threshRead();
    rightRes.threshRead();
    delay(10);
  }

  Serial.print(frontRes.threshold);
  Serial.print(" ");
  Serial.print(centerRes.threshold);
  Serial.print(" ");
  Serial.println(rightRes.threshold);
}

void loop() {
  printLights();
  Serial.println();
  
  if(!frontRes.triggered()){
    drivetrain.clockwise(Drivetrain::LOW_TURN);
  } else {
    // if the center sensor is not triggered, the car is off the line
    drivetrain.forward(Drivetrain::LEFT);
  }

  if(!frontRes.triggered() && !frontRes.triggered()){
    delay(50);
  }
}
