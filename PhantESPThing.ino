#include <ESP8266WiFi.h>      //ESP8266 Core WiFi Library 

#include <DNSServer.h>        //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <Wire.h>             //I2C for TMP102 (and others)
#include <SparkFunTSL2561.h>  //Interactions and calculations with/for light sensor

// #define USE_LED // uncomment this to turn the LED on when we're awake (bad for saving power)

#include <Phant.h>
////////////////
// Phant Keys //
////////////////
const char PhantHost[] =  "data.sparkfun.com";
const char PublicKey[] =  "your public key here";
const char PrivateKey[] = "your private key here";

const int led = 5;

// Time between readings
#define SLEEPSECS 30

// Defines for lux sensor
SFE_TSL2561 light;
boolean gain = false;
unsigned int ms;
unsigned long integrationStart;

float getTemperature(){
  const int tmp102Address = 0x48;
  Wire.requestFrom(tmp102Address, 2); 

  byte MSB = Wire.read();
  byte LSB = Wire.read();

  //it's a 12bit int, using two's compliment for negative
  int16_t tempRegister = ((MSB << 8) | LSB) >> 4; 

  float celsius = tempRegister * 0.0625;
  return celsius;
}

void setup(void){
  Phant phant(PhantHost, PublicKey, PrivateKey);

  #ifdef USE_LED
    pinMode(led, OUTPUT);
    digitalWrite(led, 1);
  #endif

  // Onboard temp -- do this immediately to minimize self-heating from other chips
  Wire.begin();
  phant.add("temp", getTemperature());

  // Onboard lux setup
  light.begin(TSL2561_ADDR_0);
  light.setTiming(gain, 1, ms);
  light.setPowerUp();
  integrationStart = millis();

// Network stuff:
// Make up a name
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  String tmpName = "thing" + macID;
  char mdnsName[11];
  tmpName.toCharArray(mdnsName, 11);

//phant.add("batt", (float)analogRead(A0) * 5.7 / 1023.0); //470k into 100k

// Get on the network
  WiFiManager wifiManager;
  wifiManager.autoConnect(mdnsName);
  phant.add("name", mdnsName);

// Get data from lux (takes a while -- that's why we've waited until now)
  while (millis() - integrationStart < ms + 10) { // busy-wait until integration time has passed
    delay(10);
  }
  unsigned int data0, data1;
  if (light.getData(data0,data1))
  {
    // getData() returned true, communication was successful

    double lux;    // Resulting lux value
    boolean good;  // True if neither sensor is saturated
    
    // Perform lux calculation:

    good = light.getLux(gain,ms,data0,data1,lux);
    if (!good) {
      lux = 99999;
    }
    
    phant.add("lux", lux);
  }
  else
  {
    phant.add("lux", -1);
  }
  
  light.setPowerDown();

// Post data  
  WiFiClient client;
  if (client.connect(PhantHost, 80))
  {
    client.print(phant.post());
  }
  #ifdef USE_LED
    digitalWrite(led, 0);
  #endif
  ESP.deepSleep(SLEEPSECS * 1000000);
}

void loop(void){

}
