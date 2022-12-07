// minimum unit needed on a pin to get the motor to run
int motorPower = 160;

// the maximum value of customizable power you can 
int motorMax = 255 - motorPower;

// clips the customizable motor power within a range, and clips it
void clipPower(int& power){
  if(power < 0 || power > motorMax){
    Serial.println("Invalid Power");
  }
  power = max(0, min(motorMax, power));
}

// runs a motor
struct Motor{
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
    // notify user if the power is too large and clip it
    clipPower(power);

    // write power needed to control
    analogWrite(ctl, 255);

    // set the direction
    digitalWrite(out1, HIGH);
    digitalWrite(out2, LOW);
  }

  // move the motor backwrad with [power] power
  void backward(int power){
    // notify user if power is out of range and clip it
    clipPower(power);

    // write power needed to ctl pin
    analogWrite(ctl, motorPower + power);

    // set the direction
    digitalWrite(out1, LOW);
    digitalWrite(out2, HIGH);    
  }

  // stops the motor
  void stop(){
    analogWrite(ctl, 0);
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

  // moves the chassis forward
  void forward(int power){
    clipPower(power);
    left.forward(power);
    right.forward(power);
  }

  // moves the chassis backward
  void backward(int power){
    clipPower(power);
    left.backward(power);
    right.backward(power);
  }

  // rotates the chassis clockwise
  void clockwise(int power){
    clipPower(power);
    left.forward(power);
    right.backward(power);
  }

  // rotates the chassis counterclockwise
  void counterClockwise(int power){
    clipPower(power);
    left.backward(power);
    right.forward(power);
  }
};

// the left and right wheel
Motor left{9, 7, 8};
Motor right{3, 4, 2};

// the chassis' drivetrain
Drivetrain drivetrain(left, right);

void setup(){
  left.setup();
  right.setup();
  Serial.begin(9600);
}

int power = motorMax;
void loop() {
  left.forward(power);
  right.forward(power);
  delay(1000);
  left.stop();
  right.stop();
  delay(1000);
}