/*
  UARTFUZZ

  Author: Alexis Vanden Eijnde
  Date: 21/03/2017

  Finds the correct pin transmitting data over serial.

  Switches through Software serial ports until data is found, then attempts to fuzz baud rate.
  Output gives the user which pin is TX, and what baud rate.

*/

#include <SoftwareSerial.h>
//(Rx,Tx)
SoftwareSerial pin1(9, 2);
SoftwareSerial pin2(10, 3);
SoftwareSerial pin3(11, 4);
SoftwareSerial pin4(12, 5);
SoftwareSerial pinArray[4] = {pin1, pin2, pin3, pin4};
long baudArray[13] = {300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 31250, 38400, 57600, 115200};
int pinArraySize = 4;
int baudArraySize = 13;
int baudBufferSize = 16; //How many chars to display per baud rate;

void setup() {
  // Our main communication channel back to user.
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // Start each software serial port
  for (int i = 0; i < pinArraySize; i++) {
    pinArray[i].begin(9600);
  }
  Serial.println("[+] Now fuzzing, keep reseting the device intermittently");
}

void loop() {
  int pin = queryPins();
  fuzzBaud(pin);
  //finished fuzzing; wait a little bit and start again.
  delay(5000);
}

// iterates through pins and checks for any data
// runs untill user decides to fuzz baud rate;
int queryPins() {
  while (1) {
    for (int i = 0; i < pinArraySize; i++) {
      pinArray[i].listen();
      delay(10); // breathe a little
      while (pinArray[i].available() > 0) {
        int tofuzz = reportAndAsk(i);
        if (tofuzz) return i;
        pinArray[i].flush(); // ¯\_(ツ)_/¯
        break;
      }
    }
  }
}

// Prints our pin found, and asks user fuzz baud query
// returns 1 if 'y' else 0
int reportAndAsk(int i) {
  Serial.print("[+] Data found on pin: ");
  Serial.println(i);
  Serial.println("[?] Would you like to fuzz baud rate? y/n");
  while (!Serial.available()) ;
  char fuzz = Serial.read();
  Serial.print("[+] ");
  Serial.println(fuzz);
  fuzz == 'y' ? 1 : 0;

}

//read bytes and present them to the user
void fuzzBaud(int pin) {
  Serial.println("[+] Fuzzing Baud rate");
  for (int i = 0; i < baudArraySize; i++) {
    int bufSize = baudBufferSize;
    Serial.print("\n---[");
    Serial.print(baudArray[i]);
    Serial.println("]---");
    pinArray[pin].end();
    pinArray[pin].begin(baudArray[i]);
    //fill buffer
    while (bufSize>0) {
      while (pinArray[pin].available() > 0) {
        Serial.write(pinArray[pin].read());
        bufSize-=1;
      }
    }
  }
}

