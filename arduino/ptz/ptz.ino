#include <MobaTools.h>
#include <arduino-timer.h>

const byte stepPin1 = D2;
const byte dirPin1 = D5;
const byte stepPin2 = D3;
const byte dirPin2 = D6;

const int stepsPerRev = 1600; // Steps per revolution, needs to take into account microstepping

auto yMovingUp = false;

MoToStepper stepper1( stepsPerRev, STEPDIR );
MoToStepper stepper2( stepsPerRev, STEPDIR );

auto timer = timer_create_default();

bool timer2s(void *) {
  // Serial.println("timer fired");
  return true;
}

void setup() {
  Serial.begin(19200);
  Serial.setTimeout(10);

  Serial.println("starting");
  timer.every(2000, timer2s);
  
  stepper1.attach(stepPin1, dirPin1);
  stepper1.setSpeed(0);

  stepper2.attach(stepPin2, dirPin2);
  stepper2.setSpeed(0);

  pinMode(D10, INPUT_PULLUP);
  pinMode(D13, OUTPUT);  
}

void loop() {

  timer.tick();

  int val = digitalRead(D10);   // read the input pin
  // Serial.println(val);
  digitalWrite(D13, val);  // sets the LED to the button's value

  if ((val == 1) && (!yMovingUp)) {
    // Serial.println("Hit limit switch, stopping downward movement.");
    stepper1.rotate(0);
    stepper1.setSpeed(0);
  }

  if (Serial.available() > 0) {
    String str = Serial.readString();
    str.trim();

    int x, y;
    splitStringToIntegers(str, x, y);

    // TODO: This can be cleaned up a lot, but it'll also need rewriting when I add limit switches
    if (x > 0) {
      stepper2.rotate(1);
      stepper2.setSpeed(x);
    } 
    else {
      stepper2.setSpeed(-x);
      stepper2.rotate(-1);
    }

    if (y > 0) {
      // Check for limit switch
      if (val == 1) {
        // Serial.println("Already hit limit switch, can't move further down.");
        stepper1.rotate(0);
        stepper1.setSpeed(0);
      }
      else {
        stepper1.rotate(1);
        stepper1.setSpeed(y);
        yMovingUp = false;
      }
    } 
    else {
      stepper1.setSpeed(-y);
      stepper1.rotate(-1);
      yMovingUp = true;
    }
  }
}

// Split a string like "-100,500" into two integers
void splitStringToIntegers(String input, int &firstInt, int &secondInt) {
  int commaIndex = input.indexOf(','); // Find the position of the comma
  
  // Extract substrings before and after the comma
  String firstPart = input.substring(0, commaIndex);
  String secondPart = input.substring(commaIndex + 1);
  
  // Convert the substrings to integers
  firstInt = firstPart.toInt();
  secondInt = secondPart.toInt();
}
