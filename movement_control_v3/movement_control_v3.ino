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
    if(power < 0){
      backward(-power);
      return;
    }
    // write power needed to control
    analogWrite(ctl, power);

    // set the direction
    digitalWrite(out1, HIGH);
    digitalWrite(out2, LOW);
  }

  // move the motor backwrad with [power] power
  void backward(int power){
    if(power < 0){
      forward(-power);
      return;
    }
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

  void drive(int lef, int rig){
    left.forward(lef);
    right.forward(rig);
  }
};

struct PhotoResistor{
  const static int NUM_RECORD = 2, RECORD_DELAY = 5;
  int pin;
  int threshold;
  int prev[NUM_RECORD];
  int lasRecord;

  long threshSum, readings;
  PhotoResistor(int pin, int threshold){
    this->pin = pin;
    this->lasRecord = 0;
    this->threshSum = 0;
    this->readings = 0;
    this->threshold = threshold;

    pinMode(pin, INPUT);
  }

  void threshRead(){
    threshSum += analogRead(pin);
    readings += 1;
    threshold = threshSum / readings - 40;
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

Motor left(3, 4, 2);
Motor right(9, 7, 8);
// the chassis' drivetrain
Drivetrain drivetrain(left, right);
PhotoResistor leftRes(A1, 55), centerRes(A2, 55), rightRes(A0, 55);

void printLights(){
  Serial.print(leftRes.triggered());
  Serial.print(" ");
  Serial.print(centerRes.triggered());
  Serial.print(" ");
  Serial.println(rightRes.triggered());
}

void setup(){
  Serial.begin(9600);

  for(int i = 0; i < 100; i++){
    leftRes.threshRead();
    centerRes.threshRead();
    rightRes.threshRead();
    delay(10);
  }
}

int TURN = 90, FORWARD_FAST = 80, FORWARD_SLOW = 40;
void loop() {
  printLights();
  if(centerRes.triggered() && leftRes.triggered() && rightRes.triggered()){
    drivetrain.drive(-TURN, TURN);
    Serial.println("left");
  } else if(!rightRes.triggered()){
    drivetrain.drive(TURN, -TURN);
    Serial.println("right");
  } else if(leftRes.triggered()){
    drivetrain.drive(FORWARD_FAST, FORWARD_SLOW);
    Serial.println("tilt right");
  } else if(centerRes.triggered()){
    drivetrain.drive(FORWARD_SLOW, FORWARD_FAST);
    Serial.println("tilt left");
  } else {
    drivetrain.drive(FORWARD_FAST, FORWARD_FAST);
    Serial.println("forward");
  }
}
