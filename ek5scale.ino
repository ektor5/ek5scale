#include <Streaming.h>
#include "HX711.h"

#define cout Serial
#define endl '\n'
const int BP1 = A0, BP2 = A1, BP3 = A2, BP4 = A3; //backplanes
int S1 = A4, S2 = 2, S3 = 3, S4 = 4, S5 = 5, S6 = 6, S7 = 7, S8 = 8, S9 = 9; //segment outputs.  S1 is always blanked, S2&3 are driven
int LOADCELL_DOUT_PIN = 11, LOADCELL_SCK_PIN = 12;

int weight = 0;
int oldst;

HX711 scale;

void setup()
{
  Serial.begin(19200);
  cout << "Hello from the clock LCD test mule \n";
  pinMode(S1, OUTPUT); pinMode(S2, OUTPUT); pinMode(S3, OUTPUT); //segments are live
  pinMode(S4, OUTPUT); pinMode(S5, OUTPUT);
  pinMode(S6, OUTPUT); pinMode(S7, OUTPUT);
  pinMode(S8, OUTPUT); pinMode(S9, OUTPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  oldst = (millis() / 100);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
  // by the SCALE parameter (not set yet)

  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  // by the SCALE parameter set with set_scale

}
void loop() {
  if (millis() / 100 - oldst > 5) {
    weight = scale.get_units(1);
    oldst = (millis() / 100);
    cout << weight << endl;

  }
  //cout << weight << endl;
  //int displaydigit = (millis() / 1000) % 9999; //test pattern cycles from 0 to 10
  drivelcd(weight);
}


void drivelcd(int digit_) { //drive the LCD to display a digit
  //each of the LCD backplanes is cycled low then high for 4ms while the others are held at a middle-voltage
  //the segments are lit by setting them opposite to the active backplane
  //each bit in the digit segment table below defines the state of a segment for a particular backplane for that digit
  //the segment 2 bits are first then the segment 3 bits
  //e.g. for a 0(the 1st element) segment 2 is on for backplanes 2 and 4 (0101) and segment 3 is on for all of them(1111)
  const unsigned char dst[] = {
    0b10101111,
    0b00001100,
    0b11001011,
    0b01001111,
    0b01101100,
    0b01100111,
    0b11100111,
    0b00001101,
    0b11101111,
    0b01101111,
    0b00000000
    //0b11101101
  }; //digits 0-NULL
  static int stateprev = 8; //tells us when we change states
  bool minus;
  int digit;
  if ( digit_ > 0 ) {
    digit = digit_;
    minus = 0;
  } else {
    digit = -digit_;
    minus = 1;
  }
  int statenow = (millis() / 4) & 0x07; //cycles from 0-7 changing every 4 ms
  int digit4 = digit % 10;
  int digit3 = digit > 10 ?  (digit / 10) % 10   : 10;
  int digit2 = digit > 100 ? (digit / 100) % 10  : 10;
  int digit1 = digit > 1000 ? (digit / 1000) % 10 : 10;

  if (statenow != stateprev) { //if the state has changed
    //cout <<  " " << digit4 << " "<< digit3<< " " << digit2 << " " << digit1 << endl;

    //reset backplane pins to input and activate the pull-up resistors
    pinMode(BP1, INPUT); pinMode(BP2, INPUT);  pinMode(BP3, INPUT); pinMode(BP4, INPUT);
    digitalWrite(BP1, HIGH); digitalWrite(BP2, HIGH); digitalWrite(BP3, HIGH); digitalWrite(BP4, HIGH);
    switch (statenow) {
      case 0:
        pinMode(BP1, OUTPUT); digitalWrite(BP1, LOW); //activate backplane 1 for 1st half of its cycle
        digitalWrite(S1, LOW); //ensure segment 1 stays blank
        digitalWrite(S2, (dst[digit1] & 0x80) >> 7); //activate segment 2 if needed
        digitalWrite(S3, (dst[digit1] & 0x08) >> 3); //activate segment 3 if needed
        digitalWrite(S4, (dst[digit2] & 0x80) >> 7); //activate segment 2 if needed
        digitalWrite(S5, (dst[digit2] & 0x08) >> 3); //activate segment 3 if needed
        digitalWrite(S6, (dst[digit3] & 0x80) >> 7); //activate segment 2 if needed
        digitalWrite(S7, (dst[digit3] & 0x08) >> 3); //activate segment 3 if needed
        digitalWrite(S8, (dst[digit4] & 0x80) >> 7); //activate segment 2 if needed
        digitalWrite(S9, (dst[digit4] & 0x08) >> 3); //activate segment 3 if needed
        break;
      case 1:
        pinMode(BP1, OUTPUT); digitalWrite(BP1, HIGH); //set BP1 2nd half of its cycle
        digitalWrite(S1, HIGH); //ensure segment 1 stays blank
        digitalWrite(S2, !digitalRead(S2)); //toggle segment 2
        digitalWrite(S3, !digitalRead(S3)); //toggle segment 3
        digitalWrite(S4, !digitalRead(S4)); //toggle segment 2
        digitalWrite(S5, !digitalRead(S5)); //toggle segment 3
        digitalWrite(S6, !digitalRead(S6)); //toggle segment 2
        digitalWrite(S7, !digitalRead(S7)); //toggle segment 3
        digitalWrite(S8, !digitalRead(S8)); //toggle segment 2
        digitalWrite(S9, !digitalRead(S9)); //toggle segment 3
        break;
      case 2:
        pinMode(BP2, OUTPUT); digitalWrite(BP2, LOW); //set BP2 1st half of its cycle
        digitalWrite(S1, LOW); //ensure segment 1 stays blank
        digitalWrite(S2, (dst[digit1] & 0x40) >> 6); //activate segment 2 if needed
        digitalWrite(S3, (dst[digit1] & 0x04) >> 2); //activate segment 3 if needed
        digitalWrite(S4, (dst[digit2] & 0x40) >> 6); //activate segment 2 if needed
        digitalWrite(S5, (dst[digit2] & 0x04) >> 2); //activate segment 3 if needed
        digitalWrite(S6, (dst[digit3] & 0x40) >> 6); //activate segment 2 if needed
        digitalWrite(S7, (dst[digit3] & 0x04) >> 2); //activate segment 3 if needed
        digitalWrite(S8, (dst[digit4] & 0x40) >> 6); //activate segment 2 if needed
        digitalWrite(S9, (dst[digit4] & 0x04) >> 2); //activate segment 3 if needed
        break;
      case 3:
        pinMode(BP2, OUTPUT); digitalWrite(BP2, HIGH); //set BP2 2nd half of its cycle
        digitalWrite(S1, HIGH); //ensure segment 1 stays blank
        digitalWrite(S2, !digitalRead(S2)); //toggle segment 2
        digitalWrite(S3, !digitalRead(S3)); //toggle segment 3
        digitalWrite(S4, !digitalRead(S4)); //toggle segment 2
        digitalWrite(S5, !digitalRead(S5)); //toggle segment 3
        digitalWrite(S6, !digitalRead(S6)); //toggle segment 2
        digitalWrite(S7, !digitalRead(S7)); //toggle segment 3
        digitalWrite(S8, !digitalRead(S8)); //toggle segment 2
        digitalWrite(S9, !digitalRead(S9)); //toggle segment 3
        break;
      case 4:
        pinMode(BP3, OUTPUT); digitalWrite(BP3, LOW); //set BP3 1st half of its cycle
        digitalWrite(S1, LOW); //ensure segment 1 stays blank
        digitalWrite(S2, (dst[digit1] & 0x20) >> 5); //activate segment 2 if needed
        digitalWrite(S3, (dst[digit1] & 0x02) >> 1); //activate segment 3 if needed
        digitalWrite(S4, (dst[digit2] & 0x20) >> 5); //activate segment 2 if needed
        digitalWrite(S5, (dst[digit2] & 0x02) >> 1); //activate segment 3 if needed
        digitalWrite(S6, (dst[digit3] & 0x20) >> 5); //activate segment 2 if needed
        digitalWrite(S7, (dst[digit3] & 0x02) >> 1); //activate segment 3 if needed
        digitalWrite(S8, (dst[digit4] & 0x20) >> 5); //activate segment 2 if needed
        digitalWrite(S9, (dst[digit4] & 0x02) >> 1); //activate segment 3 if needed
        break;
      case 5:
        pinMode(BP3, OUTPUT); digitalWrite(BP3, HIGH); //set BP3 2nd half of its cycle
        digitalWrite(S1, HIGH); //ensure segment 1 stays blank
        digitalWrite(S2, !digitalRead(S2)); //toggle segment 2
        digitalWrite(S3, !digitalRead(S3)); //toggle segment 3
        digitalWrite(S4, !digitalRead(S4)); //toggle segment 2
        digitalWrite(S5, !digitalRead(S5)); //toggle segment 3
        digitalWrite(S6, !digitalRead(S6)); //toggle segment 2
        digitalWrite(S7, !digitalRead(S7)); //toggle segment 3
        digitalWrite(S8, !digitalRead(S8)); //toggle segment 2
        digitalWrite(S9, !digitalRead(S9)); //toggle segment 3
        break;
      case 6:
        pinMode(BP4, OUTPUT); digitalWrite(BP4, LOW); //set BP4 1st half of its cycle
        digitalWrite(S1, LOW); //ensure segment 1 stays blank
        digitalWrite(S2, minus); //activate segment 2 if needed
        digitalWrite(S3, (dst[digit1] & 0x01) >> 0); //activate segment 3 if needed
        digitalWrite(S4, (dst[digit2] & 0x10) >> 4); //activate segment 2 if needed
        digitalWrite(S5, (dst[digit2] & 0x01) >> 0); //activate segment 3 if needed
        digitalWrite(S6, (dst[digit3] & 0x10) >> 4); //activate segment 2 if needed
        digitalWrite(S7, (dst[digit3] & 0x01) >> 0); //activate segment 3 if needed
        digitalWrite(S8, (dst[digit4] & 0x10) >> 4); //activate segment 2 if needed
        digitalWrite(S9, (dst[digit4] & 0x01) >> 0); //activate segment 3 if needed
        break;
      case 7:
        pinMode(BP4, OUTPUT); digitalWrite(BP4, HIGH); //set BP4 2nd half of its cycle
        digitalWrite(S1, HIGH); //ensure segment 1 stays blank
        digitalWrite(S2, !digitalRead(S2)); //toggle segment 2
        digitalWrite(S3, !digitalRead(S3)); //toggle segment 3
        digitalWrite(S4, !digitalRead(S4)); //toggle segment 2
        digitalWrite(S5, !digitalRead(S5)); //toggle segment 3
        digitalWrite(S6, !digitalRead(S6)); //toggle segment 2
        digitalWrite(S7, !digitalRead(S7)); //toggle segment 3
        digitalWrite(S8, !digitalRead(S8)); //toggle segment 2
        digitalWrite(S9, !digitalRead(S9)); //toggle segment 3

    }
    stateprev = statenow;
  }
}
