// runs a motor
struct Motor {
  // control (power) pin, out1 point and out2 pin (for directions)
  int ctl, out1, out2;

  // sets up the relevant pins
  void setup(){
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
struct Drivetrain{
  // left and right motors
  Motor left, right;

  // class constructor
  Drivetrain(Motor left, Motor right){
    this->left = left;
    this->right = right;
  }

  void setup(){
    left.setup();
    right.setup();
  }

  // stops the drivetrain
  void stop(){
    left.stop();
    right.stop();
  }

  // moves the chassis forward, should be called once at the start of the movement
  void forward(int mode){
    if(mode == 0){
      left.forward(190);
      right.forward(145);
    } else if(mode == -1){
      // slow - adds voltage for momentum, then goes at a slow pace
      left.forward(160);
      right.forward(170);
      delay(200);
      left.forward(90);
      right.forward(95);
    } else {
      Serial.println("invalid forward mode: " + mode);
    } 
  }

  // moves the chassis backward, should be called once at the start of the movement
  void backward(int mode){
    if(mode == 0){
      left.backward(190);
      right.backward(145);
    } else if(mode == -1){
      // slow - adds voltage for momentum, then goes at a slow pace
      left.backward(160);
      right.backward(170);
      delay(300);
      left.backward(90);
      right.backward(95);
    }else {
      Serial.println("invalid backward mode: " + mode);
    }
  }

  // rotates the chassis clockwise
  void clockwise(int mode){
    if(mode == 0){
      left.forward(190);
      right.backward(145);
    } else {
      Serial.println("invalid CW mode: " + mode);
    }
  }

  // rotates the chassis counterclockwise
  void counterClockwise(int mode){
    if(mode == 0){
      left.backward(190);
      right.forward(145);
    }else {
      Serial.println("invalid CCW mode: " + mode);
    }
  }

  // turn [angle] degrees clockwise, make [angle] negative for CCW turns, then stop
  void turn(int angle){
    if(angle == 90){
      clockwise(0);
      delay(450);
      stop();
    }
  }
};
struct PhotoResistor{
  int pin;
  const int THRESHOLD = 45;
  PhotoResistor(int pin){
    this->pin = pin;
  }
  void setup(){
    pinMode(pin, INPUT);
  }
  int read(){
    return analogRead(pin);
  }
  bool triggered(){
    return read() >= THRESHOLD;
  }
};

Motor left = {3, 2, 4};
Motor right = {9, 7, 8};
// the chassis' drivetrain
Drivetrain drivetrain(left, right);
PhotoResistor center(A0);

void setup(){
  drivetrain.setup();
  center.setup();
  Serial.begin(9600);
  drivetrain.forward(-1);
}

bool stopped = false;
void loop() {
  Serial.println(center.read());
  if(!center.triggered() && !stopped){
    stopped = true;
    delay(10);
    drivetrain.backward(0);
    delay(10);
    drivetrain.stop();
  }
}