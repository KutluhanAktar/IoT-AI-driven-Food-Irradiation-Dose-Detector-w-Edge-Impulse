         /////////////////////////////////////////////  
        //   BLE Mobile Star Wars Remote Lamp      //
       //        w/ Weather & Gas Station         //
      //             ---------------             //
     //          (Arduino Nano 33 BLE)          //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// I built this luminous lighting system and developed an Android app to control its various features and display real-time weather & gas data.
//
// For more information:
// https://www.theamplituhedron.com/projects/BLE_Mobile_Star_Wars_Remote_Lamp_w_Weather_Gas_Station/
//
//
// Connections
// Arduino Nano 33 BLE :  
//                                Waveshare 2.9'' E-Paper Module
// 3.3V -------------------------- VCC
// GND --------------------------- GND
// D11 --------------------------- DIN
// D13 --------------------------- CLK
// D10 --------------------------- CS
// D9  --------------------------- DC
// D8  --------------------------- RST
// D7  --------------------------- BUSY
//                                DHT22 Temperature and Humidity Sensor
// D2  --------------------------- DATA
// 3.3V -------------------------- VCC
// GND --------------------------- GND
//                                MQ-4 Air Quality Sensor
// A0  --------------------------- S
//                                MQ-7 Air Quality Sensor
// A1  --------------------------- S
//                                2-Way Relay
// A2  --------------------------- IN_1
// A3  --------------------------- IN_2
// 5V  --------------------------- 5V
// GND --------------------------- GND
//                                RGB LEB (RAGB)
// A4  --------------------------- R
// A5  --------------------------- G
// A6  --------------------------- B
//                                Keyes Fan Motor - L9110 (Left)
// GND --------------------------- GND
// 5V  --------------------------- VCC
// D3  --------------------------- INA
// D4  --------------------------- INB
//                                Keyes Fan Motor - L9110 (Right)
// GND --------------------------- GND
// 5V  --------------------------- VCC
// D5  --------------------------- INA
// D6  --------------------------- INB
//                                Buzzer
// D12 --------------------------- +


// Include the required libraries.
#include <ArduinoBLE.h>
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include "DHT.h"

// Create the BLE service:
BLEService BLE_remote_lamp("19B10000-E8F2-537E-4F6C-D104768A1214");

// Create data characteristics and allow the remote device (central) to write, read, and notify:
BLEFloatCharacteristic temperatureCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEFloatCharacteristic mq4Characteristic("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEFloatCharacteristic mq7Characteristic("19B10004-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEByteCharacteristic lamp_1_Characteristic("19B10005-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic lamp_2_Characteristic("19B10006-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic fan_L_Characteristic("19B10007-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic fan_R_Characteristic("19B10008-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic RGB_Characteristic("19B10009-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// Define the DHT22 temperature and humidity sensor settings and the DHT object.
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Select the required display class for the Waveshare 2.9" e-Paper module.
#include <GxGDEM029T94/GxGDEM029T94.h> // 2.9" b/w

// Define the required pin settings for the Waveshare 2.9" e-Paper module.
GxIO_Class io(SPI, /*CS=*/ 10, /*DC=*/ 9, /*RST=*/ 8); 
GxEPD_Class display(io, /*RST=*/ 8, /*BUSY=*/ 7);

// Include BMP (monochrome) images as C arrays.
#include "falcon.c"
#include "bluetooth.c"
#include "sun.c"
#include "humidity.c"
#include "gas.c"
#include "error.c"

// Define fonts.
#include <Fonts/FreeSans9pt7b.h>

// Define the gas sensor pins.
#define mq_4 A0
#define mq_7 A1

// Define the L9110 fan motor control pins.
#define L_INA 3
#define L_INB 4
#define R_INA 5
#define R_INB 6

// Define the 2-way relay pins.
#define IN1 A2
#define IN2 A3 

// Define the RGB LED pins:
#define redPin   A4
#define greenPin A5
#define bluePin  A6

#define buzzer 12

// Define the data holders:
float humidity, temperature, hic;
int mq_4_val, mq_7_val, threshold = 750;
long timer;
volatile boolean _connected = false;

void setup(){
  Serial.begin(115200);

  pinMode(buzzer, OUTPUT);
  pinMode(L_INA, OUTPUT);
  pinMode(L_INB, OUTPUT);
  pinMode(R_INA, OUTPUT);
  pinMode(R_INB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(L_INA, LOW);
  digitalWrite(L_INB, LOW);
  digitalWrite(R_INA, LOW);
  digitalWrite(R_INB, LOW);
  // RGB:
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  adjustColor(255,255,255);
  
  // Initialize the 2.9" e-Paper module and print errors, if any.
  display.init(115200);
  display.setRotation(1);

  // Initialize the DHT22 sensor.
  dht.begin();

  // Check the BLE initialization status:
  while(!BLE.begin()){
    Serial.println("BLE initialization is failed!");
    err_msg();
  }
  Serial.println("\nBLE initialization is successful!\n");
  // Print this peripheral device's address information:
  Serial.print("MAC Address: "); Serial.println(BLE.address());
  Serial.print("Service UUID Address: "); Serial.println(BLE_remote_lamp.uuid()); Serial.println();

  // Set the local name this peripheral advertises: 
  BLE.setLocalName("BLE Remote Lamp");
  // Set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(BLE_remote_lamp);

  // Add the given data characteristics to the service:
  BLE_remote_lamp.addCharacteristic(temperatureCharacteristic);
  BLE_remote_lamp.addCharacteristic(humidityCharacteristic);
  BLE_remote_lamp.addCharacteristic(mq4Characteristic);
  BLE_remote_lamp.addCharacteristic(mq7Characteristic);
  BLE_remote_lamp.addCharacteristic(lamp_1_Characteristic);
  BLE_remote_lamp.addCharacteristic(lamp_2_Characteristic);
  BLE_remote_lamp.addCharacteristic(fan_L_Characteristic);
  BLE_remote_lamp.addCharacteristic(fan_R_Characteristic);
  BLE_remote_lamp.addCharacteristic(RGB_Characteristic);

  // Add the service to the device:
  BLE.addService(BLE_remote_lamp);

  // Assign event handlers for connected and disconnected devices to/from this peripheral:
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // Assign event handlers for the data characteristics modified (written) by the central device (via the Android application).
  // Then, obtain the transferred (written) commands from the Android application over BLE.
  lamp_1_Characteristic.setEventHandler(BLEWritten, get_commands);
  lamp_2_Characteristic.setEventHandler(BLEWritten, get_commands); 
  fan_L_Characteristic.setEventHandler(BLEWritten, get_commands);
  fan_R_Characteristic.setEventHandler(BLEWritten, get_commands);
  RGB_Characteristic.setEventHandler(BLEWritten, get_commands);
  
  // Start advertising:
  BLE.advertise();
  Serial.println(("Bluetooth device active, waiting for connections..."));

  delay(5000);
  timer = millis();
}

void loop(){
  // Every minute, advertise (transmit) the collected weather and gas data to the Android application over BLE.
  if(millis() - timer > 60*1000){
    collect_weather_data();
    // Show the collected data on the Waveshare 2.9" e-Paper module.
    home_screen(1);
    // Update characteristics:
    update_characteristics();
    // Update the timer:
    timer = millis();
  }
  
  // Poll for BLE events:
  BLE.poll();
}

void update_characteristics(){
  // Update all weather and gas data characteristics (floats):
  temperatureCharacteristic.writeValue(temperature);
  humidityCharacteristic.writeValue(humidity);
  mq4Characteristic.writeValue(float(mq_4_val));
  mq7Characteristic.writeValue(float(mq_7_val));
  Serial.println("\n\nBLE: Weather and Gas Data Characteristics Updated Successfully!\n");
}

void get_commands(BLEDevice central, BLECharacteristic characteristic){
  // Get the recently transferred commands over BLE.
  if(characteristic.uuid() == lamp_1_Characteristic.uuid()){
    Serial.print("\nLamp (1) => "); Serial.println(lamp_1_Characteristic.value());
    switch(lamp_1_Characteristic.value()){
      case 0: digitalWrite(IN1, HIGH);  break;
      case 1: digitalWrite(IN1, LOW);   break;
    }
  }
  if(characteristic.uuid() == lamp_2_Characteristic.uuid()){
    Serial.print("Lamp (2) => "); Serial.println(lamp_2_Characteristic.value());
    switch(lamp_2_Characteristic.value()){
      case 0: digitalWrite(IN2, HIGH);  break;
      case 1: digitalWrite(IN2, LOW);   break;
    }
  }
  if(characteristic.uuid() == fan_L_Characteristic.uuid()){
    Serial.print("Fan (L) => "); Serial.println(fan_L_Characteristic.value());
    switch(fan_L_Characteristic.value()){
      case 0: digitalWrite(L_INA, LOW); digitalWrite(L_INB, LOW);  break;
      case 1: digitalWrite(L_INA, HIGH); digitalWrite(L_INB, LOW); break;
      case 2: digitalWrite(L_INA, LOW); digitalWrite(L_INB, HIGH); break;
    }
  }
  if(characteristic.uuid() == fan_R_Characteristic.uuid()){
    Serial.print("Fan (R) => "); Serial.println(fan_R_Characteristic.value());
    switch(fan_R_Characteristic.value()){
      case 0: digitalWrite(R_INA, LOW); digitalWrite(R_INB, LOW);  break;
      case 1: digitalWrite(R_INA, HIGH); digitalWrite(R_INB, LOW); break;
      case 2: digitalWrite(R_INA, LOW); digitalWrite(R_INB, HIGH); break;
    }
  }
  if(characteristic.uuid() == RGB_Characteristic.uuid()){
    Serial.print("RGB => "); Serial.println(RGB_Characteristic.value());
    switch(RGB_Characteristic.value()){
      case 0: adjustColor(0,0,0);       break;
      case 1: adjustColor(255,0,0);     break;
      case 2: adjustColor(0,255,0);     break;
      case 3: adjustColor(0,0,255);     break;
      case 4: adjustColor(255,0,255);   break;
      case 5: adjustColor(255,255,0);   break;
      case 6: adjustColor(0,255,255);   break;
      case 7: adjustColor(255,255,255); break;
    }
  }
}

void collect_weather_data(){
  delay(2000);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature(); // Celsius
  // Compute the heat index in Celsius (isFahreheit = false).
  hic = dht.computeHeatIndex(temperature, humidity, false);

  // Get measurements generated by the gas sensors.
  mq_4_val = (analogRead(mq_4) * 3.3) / 5;
  mq_7_val = (analogRead(mq_7) * 3.3) / 5;

  // Notify the user if the evaluated gas measurements exceed the given threshold.
  while(mq_4_val > threshold || mq_7_val > threshold){
    digitalWrite(buzzer, HIGH); delay(10000); digitalWrite(buzzer, LOW); err_msg();
    mq_4_val = (analogRead(mq_4) * 3.3) / 5;
    mq_7_val = (analogRead(mq_7) * 3.3) / 5;
  }
  
  Serial.print(F("\nHumidity: ")); Serial.print(humidity); Serial.println("%");
  Serial.print(F("Temperature: ")); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Heat Index: "); Serial.print(hic); Serial.println(" °C");
  Serial.print("MQ-4 (Methane): "); Serial.println(mq_4_val);
  Serial.print("MQ-7 (Carbon Monoxide): "); Serial.println(mq_7_val);
  Serial.println("\n");
}

void home_screen(int _clear){
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  // Images:
  int l_offset = 30;
  int x_offset = 3;
  int e_offset = 50;
  int y_offset = ((display.height()/2)-40)/2;
  display.drawExampleBitmap(gImage_falcon, (display.width()/2)+25, 0, 128, 128, GxEPD_BLACK);
  display.drawExampleBitmap(gImage_sun, x_offset-1, y_offset, 40, 39, GxEPD_BLACK);
  display.drawExampleBitmap(gImage_gas, (display.width()/2)+l_offset-x_offset-24-e_offset, y_offset, 24, 40, GxEPD_BLACK);
  display.drawExampleBitmap(gImage_humidity, x_offset, (display.height()/2)+y_offset, 40, 39, GxEPD_BLACK);
  if(_connected){ display.drawExampleBitmap(gImage_bluetooth, (display.width()/2)+l_offset-x_offset-e_offset+10, (display.height()/2)+y_offset, 29, 40, GxEPD_BLACK); }
  else{ display.drawExampleBitmap(gImage_not_connected, (display.width()/2)+l_offset-x_offset-e_offset, (display.height()/2)+y_offset, 39, 40, GxEPD_BLACK); }
  // Frames (Borders):
  display.drawRect(0, 0, display.width(), display.height(), GxEPD_BLACK);
  display.drawRect((display.width()/2)+l_offset, 0, (display.width()/2)-l_offset, display.height(), GxEPD_BLACK);
  display.drawRect(0, 0, (display.width()/2)+l_offset, display.height()/2, GxEPD_BLACK);
  display.drawRect(0, display.height()/2, (display.width()/2)+l_offset, display.height()/2, GxEPD_BLACK);
  // Measurements:
  display.setFont(&FreeSans9pt7b);
  display.setCursor(48, y_offset+25);
  display.print(temperature);
  display.setCursor(48, (display.height()/2)+y_offset+25);
  display.print(humidity);
  display.setCursor((display.width()/2)+l_offset-45, y_offset+15);
  display.print(mq_4_val);
  display.setCursor((display.width()/2)+l_offset-45, y_offset+35);
  display.print(mq_7_val);
  display.update();
  delay(10000);
  // If activated, clear the e-paper module.
  if(_clear){
    display.fillScreen(GxEPD_WHITE);
    display.update();
    delay(10000);
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
  // Central connected event handler:
  Serial.print("\nConnected event, central: ");
  Serial.println(central.address());
  _connected = true;
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // Central disconnected event handler:
  Serial.print("\nDisconnected event, central: ");
  Serial.println(central.address());
  _connected = false;
}

void err_msg(){
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.drawExampleBitmap(gImage_error, (display.width()-128)/2, 0, 128, 128, GxEPD_BLACK);
  display.update();
  adjustColor(255,0,0);
  delay(10000);
}

void adjustColor(int r, int g, int b){
  analogWrite(redPin, (255-r));
  analogWrite(greenPin, (255-g));
  analogWrite(bluePin, (255-b));
}
