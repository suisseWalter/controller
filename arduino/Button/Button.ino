


#include <Wire.h>
//#include <LiquidCrystal_I2C.h>

uint8_t buttonPins[] = {2, 3, 4, 5, 6, 7};
uint8_t analogpin[] = {A0,A1,A2};
uint32_t buttonState = 0;
uint32_t newstate=0;
const int lowercut=0,highercut=1023;
uint32_t time = 0, k = 0, time2=0,time3=0;
bool resend = false, resend2 = false;
//LiquidCrystal_I2C lcd(0x27, 16, 4);
char readdata[30] = {""};
char readdata0[30] = {""};
int analogState[] = {0,0,0};
int newanalogState[] = {0,0,0};
struct Encoder {
  uint8_t first;//first pin number
  uint8_t second;//second pin number
  bool state;//current state. SET TO 0 AT THE BEGINNING
};
struct potiswitch {
  uint8_t pin;//pin the switch is connected to.
  uint8_t numpos;//number of positions the switch has.
  uint8_t pos; // current position of the switch. SET TO 0 AT THE BEGINNING
};
struct toggle {
  uint8_t pin;//pin on the active side of the switch;
  bool state;//current position of the toggle-switch. SET TO 0 AT THE BEGINNING
};
toggle toggleswitch[] = {0};// {{9, 0}};
potiswitch testswitch[] ={0};// {{A1, 4, 0}};

Encoder encod[] ={0};// {{7, 8, 0}}; //{{7,8,0,{9,10,0}};


unsigned long timer1[sizeof(testswitch) / 3] = {0};
//void lcdinit() {
//  lcd.init();
//  delay(100);
//  lcd.backlight();
//  lcd.setCursor(0, 0);
//  lcd.print("flightcontrolertest");
//  //lcd.write(100);
//}
//int lcdcontrol() {
//  char rc;
//  while (Serial.available() > 0) {
//    rc = Serial.read();
//    readdata[k++] = rc;
//    //Serial.print(readdata);
//
//  }
//  if (readdata[k - 2] == '>' || readdata[k - 1] == '>') {
//    k = 0;
//    lcd.setCursor(0, 1);
//    //need to remove the control char
//    lcd.print(readdata);
//    //Serial.println("yeah");
//    //Serial.println(readdata);
//    for (int i = 0; i < 30; i++)readdata[i] = "";
//    //  readdata="";
//  }
//
//}
void setup() {
  Serial.begin(115200);
  Serial.println("success");
  //lcdinit();
  pinMode(12, OUTPUT);

  for (int i = 0; i < sizeof( buttonPins); i++) {
    pinMode(buttonPins[i], INPUT);
  }

  for (int i = 0; i < sizeof( analogpin); i++) {
    pinMode(analogpin[i], INPUT);
  }
  for (int i = 0; i < sizeof(encod) / 3; i++) {
    pinMode(encod[i].first, INPUT);
    pinMode(encod[i].second, INPUT);
  }
  for (int i = 0; i < sizeof(testswitch) / 3; i++) {
    pinMode(testswitch[i].pin, INPUT);
  }
  for (int i = 0; i < sizeof(toggleswitch) / 2; i++) {
    pinMode(toggleswitch[i].pin, INPUT);
  }
 delay(1000);
}

void loop() {
  //lcdcontrol();
  //Serial.print("1");
  uint32_t currentbit = 1;
  //uint32_t newstate = 0;
  uint8_t currentSread = 0;
  bool currentread = false;
  bool currentread2 = false;

  newstate=0;
  for (int i = 0; i < sizeof(buttonPins); i++) {
    currentread = digitalRead(buttonPins[i]);
    
    newstate += currentbit * currentread;
    currentbit = currentbit << 1;
    //Serial.println(currentread);

  }
  //Serial.print("2");
  for (int i = 0; i < sizeof(encod) / 3; i++) {
    currentread = digitalRead(encod[i].first);
    currentread2 = digitalRead(encod[i].second);
    if (currentread == 0 && encod[i].state == 1) {
      newstate += (2 - currentread2) * currentbit;
      encod[i].state = 0;
      digitalWrite(12, HIGH);
    }
    else if (currentread == 1 && encod[i].state == 0) {
      encod[i].state = 1;
      digitalWrite(12, LOW);
    }
    currentbit = currentbit << 2;

  }
  //Serial.print("3");
  for (int i = 0; i < sizeof(testswitch) / 3; i++) {
    if (millis() < timer1[i] + 20) {
      currentbit = currentbit << 2;
      continue;

    }
    currentSread = map(analogRead(testswitch[i].pin), 0, 1050, 0, testswitch[i].numpos);
    //currentSread=analogRead(testswitch[i].pin)/(1023/testswitch[i].numpos);
    if (currentSread > testswitch[i].pos) {
      timer1[i] = millis();
      testswitch[i].pos += 1;
      newstate += currentbit;
      resend = true;
    }
    else if (currentSread < testswitch[i].pos) {
      timer1[i] = millis();
      newstate += currentbit * 2;

      testswitch[i].pos -= 1;
      resend = true;
    }
    currentbit = currentbit << 2;
    //Serial.println(map(analogRead(testswitch[i].pin),0,1023,0,testswitch[i].numpos));
  }
  //Serial.print("4");
  for (int i = 0; i < sizeof(toggleswitch) / 2; i++) {
    currentread = digitalRead(toggleswitch[i].pin);
    if (toggleswitch[i].state != currentread) {
      newstate += currentbit * (1 + currentread);

      toggleswitch[i].state = currentread;
      resend = true;
    }
    currentbit = currentbit << 2;
  }
//Serial.print("1");
  if (buttonState != newstate &&(time3+1<millis())) {
   // while (time2 + 10 > millis())delay(1);
    Serial.print("-B<");
    Serial.print(newstate);
    Serial.println('>');
    //Serial.print(buttonState);
    buttonState = newstate;
    time3 = millis();

  }
  //Serial.print("6");
  for (int i = 0; i < sizeof(analogpin); i++) {
    int temp= (analogRead(analogpin[i])+ newanalogState[i])/2;
    
    int temp2= temp*(temp>lowercut&&temp<highercut)+highercut*(temp>=highercut)+lowercut*(temp<=lowercut);
    newanalogState[i] =map(temp2,lowercut,highercut,0,1024);
    //Serial.print ( newanalogState[i]);
    //delayMicroseconds(100);
    //newanalogState[i]+=analogRead(analogpin[i]);
    //newanalogState[i]/=2;
  }
  //Serial.println(newanalogState[0]);

  for (int i = 0; i < sizeof(analogpin); i++) {
    if ((time3+2<millis())&&((!compare(newanalogState[i], analogState[i], 3))||((newanalogState[i]==0||newanalogState[i]==1024)&&newanalogState[i]!=analogState[i]))) {
      Serial.print("-");
      Serial.print(i + 1);
      Serial.print("<");
      Serial.print(1024-newanalogState[i]);
      Serial.println('>');
      analogState[i] = newanalogState[i];
      time3 = millis();
    }
  }
  //delay(100);

}


bool compare(int x, int y, int acc) {
  return ((x + acc >= y) && (x - acc <= y));
}
