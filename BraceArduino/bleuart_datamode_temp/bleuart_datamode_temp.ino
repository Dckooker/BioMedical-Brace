#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
//#include "Adafruit_BluefruitLE_UART.h"

//#include "BluefruitConfig.h"


/*
  analogRead(5);
  delay(10);
  nTemp = analogRead(5) * 5000L / 1024L  / 10;
 */

 
#define FACTORYRESET_ENABLE            1
#define MINIMUM_FIRMWARE_VERSION       "0.6.6"
#define MODE_LED_BEHAVIOUR             "MODE" //"BLEUART"
#define BUFSIZE                        128
#define VERBOSE_MODE                   true
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4

int tempPin = A1;
int tempReading;
int tempSet = 200;
int MOSFET1 = 10;
int MOSFET2 = 11;
int MOSFET3 = 12;
int MOSFET4 = 13;
int mode = 0;
float volt;
float tempC;
float tempF;
int index = 0;

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//error stop
void error(const __FlashStringHelper*err){
  Serial.println(err);
  while (1);
}

void setup(void){
  
  while (!Serial);
  delay(500);

  Serial.begin(115200);
  Serial.println(F("BLE UART TEST"));
  Serial.print(F("Initialising..."));

  if(!ble.begin(VERBOSE_MODE)){
    error(F("Feather not found!"));
  }
  Serial.println(F("Done!"));

  if(FACTORYRESET_ENABLE){
    Serial.println(F("Resetting..."));
    if(!ble.factoryReset()){
      error(F("Reset error!"));
    }
  }

  ble.echo(false);

  Serial.println("Info:");
  ble.info();

  Serial.println(F("Waiting for connection..."));

  //ble.verbose(false);
  while (!ble.isConnected()){
      delay(500);
  }

  if(ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION)){
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  ble.setMode(BLUEFRUIT_MODE_DATA);
}

void loop(void){

  tempReading = analogRead(tempPin);  
  Serial.print("Temp reading = ");
  Serial.print(tempReading);
 
  volt = tempReading * 3.3;
  volt /= 1024.0; 
  Serial.print(" - ");
  Serial.print(volt);
  Serial.println(" V");
 
  tempC = (volt - 0.5) * 100;
  Serial.print(tempC);
  Serial.println(" C");
 
  tempF = (tempC * 9.0 / 5.0) + 32.0;
  Serial.print(tempF);
  Serial.println(" F");

  /*
  char inputs[6];
  dtostrf(tempC, 3, 1, inputs);
  Serial.print("Sending: ");
  Serial.println(inputs);
  ble.print(inputs);
  */
  index = 0;
  char received[] = "00000";
  
  while (ble.available()){
    int c = ble.read();
    received[index] = (char)c;
    index++;
  }

  if(index == 0){
    return;
  }

  tempSet = atoi(received);

  if(tempSet > tempC){
    if(mode != 1){
      resetPeltier();
      digitalWrite(MOSFET1, HIGH);
      digitalWrite(MOSFET3, HIGH);
      mode = 1;
    }
  }
  else if(tempSet < tempC){
    if(mode != -1){
      resetPeltier();
      digitalWrite(MOSFET2, HIGH);
      digitalWrite(MOSFET4, HIGH);
      mode = -1;
    }
  }
  else{
    resetPeltier();
    mode = 0;
  }
  
  delay(100);
}

void resetPeltier(){
  digitalWrite(MOSFET1, LOW);
  digitalWrite(MOSFET2, LOW);
  digitalWrite(MOSFET3, LOW);
  digitalWrite(MOSFET4, LOW);
}
