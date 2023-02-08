// include the library code:
#include <LiquidCrystal.h>
//compass library
#include <Wire.h>
#define addr 0x60 //I2C Address

#define Motor_forward         1
#define Motor_return          0
#define Motor_L_dir_pin       7
#define Motor_R_dir_pin       8
#define Motor_L_pwm_pin       9
#define Motor_R_pwm_pin       10

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 37, en = 36, d4 = 35, d5 = 34, d6 = 33, d7 = 32;

//the pins for switching on and off via joystick button; pins for pwm counting
const int buttonPin = 19, ENCBleft = 3, ENCBright = 2;

//variables for pwm counting
volatile int iL = 0, iR = 0, frequencyHzL, frequencyHzR;

//joystick button (set to be off initially)
boolean joystickState = false;

//for compass
byte raw = 0;
byte fPos = 0;
byte iPos = 0;
String B;


LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // Initialize LCD set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  //Initialize compass
  Wire.begin();
  //Initialize serial
  Serial.begin(9600);

  //for switching on and off via joystick button
  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), printSerial, FALLING );

  //for counting pwm (therefore gaining distance)
  pinMode (2, INPUT);
  pinMode (3, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCBleft), countENCBL, FALLING );
  attachInterrupt(digitalPinToInterrupt(ENCBright), countENCBR, FALLING );

  //Initialize position
  carPos();
  iPos = raw;
  fPos = (byte(raw * 8 + 128)) / 8;

  //input via joystick
  pinMode (A7, INPUT);
  pinMode (A6, INPUT);
}

void loop() {
  //initializing variables
  int startMove = 255;
  int stopMove = 0;
  int turn = 150;
  int motorRight = 0, motorLeft = 0;
  int distantCount = 0;

  iL = 0;
  distantCount = 0;

  //driving the car through web page
  //ESP IP: 10.5.2.42
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    Serial.print("Instruction received, content: ");
    Serial.println(message);
    int pos_dire = message.indexOf("Turn");
    int pos_dist = message.indexOf("Move");
    int pos_danc = message.indexOf("Dance");
    int pos_s = message.indexOf(":");

    if (pos_dire > -1) {
      Serial.println("Command = dire ");
      if (pos_s > -1) {
        lcd.clear();
        //reading the number behind ":",the input from the ESP
        int stat = message.substring(pos_s + 1).toInt();
        //converting the stat number (should be cm) to pwn counts
        int targetPos = stat / 11.25;
        carPos();
        lcd.print("Current position: ");
        lcd.setCursor(0, 1);
        lcd.print(int(raw * 11.25));
        lcd.print(" degrees");

        if (stat > 0) {
          targetPos = raw + targetPos;
          targetPos = (targetPos > 31) ? targetPos - 32 : targetPos;
          while (raw != targetPos) {
            digitalWrite(Motor_R_dir_pin, Motor_return);
            digitalWrite(Motor_L_dir_pin, Motor_forward);
            analogWrite(Motor_L_pwm_pin, turn);
            analogWrite(Motor_R_pwm_pin, turn);
            carPos();
          }
        }
        else {
          stat = stat * -1;
          targetPos = raw + targetPos;
          targetPos = (targetPos < 0) ? targetPos + 32 : targetPos;
          while (raw != targetPos) {
            digitalWrite(Motor_R_dir_pin, Motor_forward);
            digitalWrite(Motor_L_dir_pin, Motor_return);
            analogWrite(Motor_L_pwm_pin, turn);
            analogWrite(Motor_R_pwm_pin, turn);
            carPos();
          }
        }
        analogWrite(Motor_L_pwm_pin, stopMove);
        analogWrite(Motor_R_pwm_pin, stopMove);
        lcd.clear();
        delay(1000);
        lcd.print("Current position: ");
        lcd.setCursor(0, 1);
        lcd.print(int(raw * 11.25));
        lcd.print(" degrees");

      }
    }

    else if (pos_dist > -1) {
      Serial.println("Command = dist ");
      if (pos_s > -1) {
        lcd.clear();
        //reading the number behind ":",the input from the ESP
        int stat = message.substring(pos_s + 1).toInt();
        //converting the stat number (should be cm) to pwn counts
        int disMove = stat * 13.8;

        if (disMove > 0) {
          while (distantCount < disMove) {
            digitalWrite(Motor_R_dir_pin, Motor_forward);
            digitalWrite(Motor_L_dir_pin, Motor_forward);
            analogWrite(Motor_L_pwm_pin, startMove);
            analogWrite(Motor_R_pwm_pin, startMove);
            distantCount = iL;
            //Serial.println(distantCount);
          }

        }
        else {
          disMove = disMove * -1;
          while (distantCount < disMove) {
            digitalWrite(Motor_R_dir_pin, Motor_return);
            digitalWrite(Motor_L_dir_pin, Motor_return);
            analogWrite(Motor_L_pwm_pin, startMove);
            analogWrite(Motor_R_pwm_pin, startMove);
            distantCount = iL;
            //Serial.println(distantCount);
          }
        }
        analogWrite(Motor_L_pwm_pin, stopMove);
        analogWrite(Motor_R_pwm_pin, stopMove);
        distantCount = 0;
        iL = 0;
        delay(1000);
        lcd.print("Moving distance: ");
        lcd.setCursor(0, 1);
        lcd.print(stat);
        lcd.print(" cm");
      }
    }
    //making it going forward 10cm turn 180 then go 10cm again
    else if (pos_danc > -1) {
      lcd.clear();
      Serial.println("Start Dancing");
      lcd.print("Let's Dance!");

      while (distantCount < 138) {
        digitalWrite(Motor_R_dir_pin, Motor_forward);
        digitalWrite(Motor_L_dir_pin, Motor_forward);
        analogWrite(Motor_L_pwm_pin, startMove);
        analogWrite(Motor_R_pwm_pin, startMove);
        distantCount = iL;
      }
      analogWrite(Motor_L_pwm_pin, stopMove);
      analogWrite(Motor_R_pwm_pin, stopMove);
      delay(1000);

      while (raw != fPos) {
        digitalWrite(Motor_R_dir_pin, Motor_return);
        digitalWrite(Motor_L_dir_pin, Motor_forward);
        analogWrite(Motor_L_pwm_pin, turn);
        analogWrite(Motor_R_pwm_pin, turn);
        carPos();
      }
      analogWrite(Motor_L_pwm_pin, stopMove);
      analogWrite(Motor_R_pwm_pin, stopMove);
      delay(1000);

      distantCount = 0;
      iL = 0;
      while (distantCount < 138) {
        digitalWrite(Motor_R_dir_pin, Motor_forward);
        digitalWrite(Motor_L_dir_pin, Motor_forward);
        analogWrite(Motor_L_pwm_pin, startMove);
        analogWrite(Motor_R_pwm_pin, startMove);
        distantCount = iL;
      }
      analogWrite(Motor_L_pwm_pin, stopMove);
      analogWrite(Motor_R_pwm_pin, stopMove);
      delay(1000);

      while (raw != iPos) {
        digitalWrite(Motor_R_dir_pin, Motor_forward);
        digitalWrite(Motor_L_dir_pin, Motor_return);
        analogWrite(Motor_L_pwm_pin, turn);
        analogWrite(Motor_R_pwm_pin, turn);
        carPos();
      }
      analogWrite(Motor_L_pwm_pin, stopMove);
      analogWrite(Motor_R_pwm_pin, stopMove);
      delay(1000);
    }
    else {
      Serial.println("No command found, try typing Print:Hi or Print:Hello\n");
    }
  }

  //adding the joystick control to the whole program
  if (joystickState) {
    int sensorValueX = analogRead(A7);
    int sensorValueY = analogRead(A6);
    int dirBwd = map(sensorValueY, 1023, 550, 255, 0);
    int dirFwd = map(sensorValueY, 0, 470, 255, 0);
    int dirRight = map(sensorValueX, 1023, 550, 255, 0);
    int dirLeft = map(sensorValueX, 0, 470, 255, 0);
    //Y-Axis value
    if (sensorValueY <= 470) {
      digitalWrite(Motor_R_dir_pin, Motor_forward);
      digitalWrite(Motor_L_dir_pin, Motor_forward);
      motorLeft = dirFwd;
      motorRight = dirFwd;
    }
    else if (sensorValueY >= 550) {
      digitalWrite(Motor_R_dir_pin, Motor_return);
      digitalWrite(Motor_L_dir_pin, Motor_return);
      motorLeft = dirBwd;
      motorRight = dirBwd;
    }
    else {
      motorLeft = 0;
      motorRight = 0;
    }

    //X-Axis value
    if (sensorValueX >= 550) {
      if (sensorValueY >= 550 || sensorValueY <= 470) {
        motorLeft = (motorLeft >= dirRight) ? motorLeft : dirRight;
        motorRight = motorLeft - dirRight / 2 - (motorLeft - motorRight) / 2;
      }
      else {
        digitalWrite(Motor_R_dir_pin, Motor_return);
        digitalWrite(Motor_L_dir_pin, Motor_forward);
        motorLeft = dirRight;
        motorRight = dirRight;
      }
    }
    else if (sensorValueX <= 470) {
      if (sensorValueY >= 550 || sensorValueY <= 470) {
        motorRight = (motorRight >= dirLeft) ? motorRight : dirLeft;
        motorLeft = motorRight - dirLeft / 2 - (motorRight - motorLeft) / 2;
      }
      else {
        digitalWrite(Motor_R_dir_pin, Motor_forward);
        digitalWrite(Motor_L_dir_pin, Motor_return);
        motorLeft = dirLeft;
        motorRight = dirLeft;
      }
    }

  }
  else {
    motorLeft = 0;
    motorRight = 0;

  }
  analogWrite(Motor_L_pwm_pin, motorLeft);
  analogWrite(Motor_R_pwm_pin, motorRight);


}


//car position from compass
void carPos() {
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x01);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 0x01, true);
  if (Wire.available() >= 0x01)
  {
    raw = byte (Wire.read() / 8);
  }
}

//joystick button on/off function
void printSerial() {
  joystickState = !joystickState;
  lcd.clear();
  Serial.println("Pressed");
  lcd.print("Pressed");
}

//PWM counting function, 13.8 pulses per cm
void countENCBL() {
  iL++;
}
void countENCBR() {
  iR++;
}
