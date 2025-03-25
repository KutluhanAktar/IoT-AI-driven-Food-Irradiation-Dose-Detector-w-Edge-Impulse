         /////////////////////////////////////////////  
        //   IoT AI-driven Food Irradiation Dose   // 
       //        Detector w/ Edge Impulse         //
      //           -----------------             //
     //            (Beetle ESP32-C3)            //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

// 
// Collate weight, color, and emitted ionizing radiation of foods to train a NN. Then, run it on Beetle C3 to detect food irradiation doses.
//
// For more information:
// https://www.theamplituhedron.com/projects/IoT_AI_driven_Food_Irradiation_Dose_Detector_w_Edge_Impulse
//
//
// Connections
// Beetle ESP32-C3 : 
//                                Gravity: Geiger Counter Module
// D5   --------------------------- D
// VCC  --------------------------- +
// GND  --------------------------- -
//                                Gravity: I2C 1Kg Weight Sensor Kit - HX711
// VCC  --------------------------- VCC
// GND  --------------------------- GND
// D9   --------------------------- SCL
// D8   --------------------------- SDA
//                                Fermion: 1.51” SSD1309 OLED Transparent Display
// D4   --------------------------- SCLK
// D6   --------------------------- MOSI
// D7   --------------------------- CS
// D2   --------------------------- RES
// D1   --------------------------- DC
//                                AS7341 11-Channel Spectral Color Sensor
// VCC  --------------------------- +
// GND  --------------------------- -
// D9   --------------------------- C
// D8   --------------------------- D
//                                Control Button (A)
// D0   --------------------------- +
//                                Control Button (B)
// D20  --------------------------- +
//                                Control Button (C)
// D21  --------------------------- +


// Include the required libraries:
#include <WiFi.h>
#include <DFRobot_Geiger.h>
#include <DFRobot_HX711_I2C.h>
#include <U8g2lib.h>
#include <SPI.h>
#include "DFRobot_AS7341.h"

char ssid[] = "<_SSID_>";        // your network SSID (name)
char pass[] = "<_PASSWORD_>";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

// Define the server (Raspberry Pi).
char server[] = "192.168.1.20";
// Define the web application path.
String application = "/food_irradiation_data_logger/get_data.php";

// Initialize the WiFi client library.
WiFiClient client; /* WiFiSSLClient client; */

// Define the Geiger counter module.
DFRobot_Geiger geiger(5);

// Define the HX711 weight sensor.
DFRobot_HX711_I2C MyScale;

// Define the AS7341 object.
DFRobot_AS7341 as7341;
// Define AS7341 data objects:
DFRobot_AS7341::sModeOneData_t data1;
DFRobot_AS7341::sModeTwoData_t data2;

// Define the 1.51” OLED transparent display (SSD1309).
#define OLED_DC  1
#define OLED_CS  7
#define OLED_RST 2

U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI u8g2(/* rotation=*/U8G2_R0, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC,/* reset=*/OLED_RST);

// Define monochrome graphics:
static const unsigned char error_bits[] U8X8_PROGMEM = {
   0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x0f, 0x00, 0x00, 0xfc,
   0xff, 0x3f, 0x00, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0x80, 0xff, 0xff, 0xff,
   0x01, 0xc0, 0xff, 0x81, 0xff, 0x03, 0xe0, 0xff, 0x00, 0xff, 0x07, 0xf0,
   0xff, 0x00, 0xff, 0x0f, 0xf0, 0x7f, 0x00, 0xfe, 0x0f, 0xf8, 0x7f, 0x00,
   0xfe, 0x1f, 0xfc, 0x7f, 0x00, 0xfe, 0x3f, 0xfc, 0xff, 0x00, 0xff, 0x3f,
   0xfe, 0xff, 0x00, 0xff, 0x7f, 0xfe, 0xff, 0x00, 0xff, 0x7f, 0xfe, 0xff,
   0x00, 0xff, 0x7f, 0xfe, 0xff, 0x00, 0xff, 0x7f, 0xff, 0xff, 0x00, 0xff,
   0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
   0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0x81,
   0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0xff,
   0xfe, 0xff, 0xc3, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0xff,
   0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xc3, 0xff,
   0x3f, 0xfc, 0xff, 0x81, 0xff, 0x3f, 0xf8, 0xff, 0x81, 0xff, 0x1f, 0xf0,
   0xff, 0x81, 0xff, 0x0f, 0xf0, 0xff, 0x81, 0xff, 0x0f, 0xe0, 0xff, 0xc3,
   0xff, 0x07, 0xc0, 0xff, 0xff, 0xff, 0x03, 0x80, 0xff, 0xff, 0xff, 0x01,
   0x00, 0xfe, 0xff, 0x7f, 0x00, 0x00, 0xfc, 0xff, 0x3f, 0x00, 0x00, 0xf0,
   0xff, 0x0f, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00
};
static const unsigned char data_colllect_bits[] U8X8_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0xf0, 0x3f, 0x00,
   0x1f, 0x00, 0x70, 0x1c, 0x30, 0x30, 0x80, 0x39, 0x00, 0x18, 0x30, 0x10,
   0x20, 0xc0, 0x20, 0x00, 0x8c, 0x63, 0xf0, 0x3f, 0x40, 0x60, 0x00, 0xc6,
   0xc6, 0x10, 0x20, 0x70, 0xf0, 0x00, 0x46, 0xcc, 0x10, 0x20, 0x38, 0x98,
   0x01, 0x42, 0x8c, 0xf0, 0x3f, 0x18, 0x08, 0x01, 0xc2, 0x86, 0xf0, 0x3f,
   0x18, 0x80, 0x01, 0xe2, 0x8f, 0x10, 0x20, 0xf0, 0xff, 0x01, 0x76, 0x9c,
   0xf0, 0x3f, 0xe0, 0xff, 0x00, 0x1e, 0xf0, 0xe0, 0x1f, 0xe0, 0x00, 0x00,
   0x1c, 0x60, 0x00, 0x03, 0x70, 0x00, 0x00, 0x18, 0x70, 0x00, 0x03, 0x38,
   0x00, 0x00, 0x70, 0xf8, 0x00, 0x03, 0x1c, 0x00, 0x00, 0xe0, 0xcf, 0xe1,
   0x1f, 0x0e, 0x00, 0x00, 0x00, 0x80, 0xfb, 0x7f, 0x07, 0x00, 0x00, 0x00,
   0x00, 0xdf, 0xec, 0x03, 0x00, 0x00, 0x00, 0x00, 0x66, 0x98, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x67, 0x98, 0x03, 0x00, 0x00, 0xfe, 0x07, 0xff, 0xff,
   0x83, 0xff, 0x01, 0x06, 0x8c, 0x21, 0x10, 0xc6, 0x80, 0x01, 0x06, 0x8c,
   0x31, 0x30, 0xc6, 0x80, 0x01, 0xfe, 0x8f, 0x31, 0x30, 0xc6, 0xff, 0x01,
   0x06, 0xfc, 0xff, 0xff, 0xff, 0x80, 0x01, 0x06, 0xfc, 0xff, 0xff, 0xff,
   0x80, 0x01, 0xfe, 0x8f, 0x31, 0x30, 0xc6, 0xff, 0x01, 0x06, 0x8c, 0x31,
   0x30, 0xc6, 0x80, 0x01, 0x06, 0x8c, 0x21, 0x10, 0xc6, 0x80, 0x01, 0xfe,
   0x07, 0xff, 0xff, 0x83, 0xff, 0x01, 0x00, 0x00, 0x67, 0x98, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x66, 0x98, 0x01, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xec,
   0x03, 0x00, 0x00, 0x00, 0x80, 0xfb, 0x7f, 0x07, 0x00, 0x00, 0x00, 0xc0,
   0xe1, 0x1f, 0x0e, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x03, 0x1c, 0x00, 0x00,
   0x00, 0x70, 0x00, 0x03, 0x38, 0x00, 0x00, 0x00, 0x20, 0x00, 0x03, 0xfc,
   0xff, 0x01, 0xe0, 0x01, 0xe0, 0x1f, 0x0c, 0x80, 0x01, 0xf8, 0x07, 0xf0,
   0x3f, 0x04, 0x00, 0x01, 0x0e, 0x1c, 0x10, 0x20, 0x24, 0x00, 0x01, 0xc4,
   0x00, 0xf0, 0x3f, 0x24, 0x08, 0x01, 0xf0, 0x03, 0xf0, 0x3f, 0x24, 0x18,
   0x01, 0x10, 0x02, 0x10, 0x20, 0x24, 0x19, 0x01, 0x00, 0x00, 0x10, 0x20,
   0xe4, 0x1b, 0x01, 0xe0, 0x01, 0xf0, 0x3f, 0xe4, 0x3f, 0x01, 0x20, 0x01,
   0x10, 0x20, 0x04, 0x00, 0x01, 0xe0, 0x01, 0x30, 0x30, 0x0c, 0x80, 0x01,
   0xe0, 0x01, 0xf0, 0x3f, 0xfc, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00
};

// Define the control button pins:
#define button_A 0
#define button_B 20
#define button_C 21

// Define the data holders:
float weight;

void setup() {
  Serial.begin(9600);

  pinMode(button_A, INPUT_PULLUP);
  pinMode(button_B, INPUT_PULLUP);
  pinMode(button_C, INPUT_PULLUP);

  // Initialize the SSD1309 transparent display.
  u8g2.begin();
  u8g2.setFontPosTop();
  //u8g2.setDrawColor(0);
  
  // Check the connection status between the weight (HX711) sensor and the Beetle ESP32-C3.
  while (!MyScale.begin()) {
    Serial.println("HX711 initialization is failed!");
    err_msg();
    delay(1000);
  }
  Serial.println("HX711 initialization is successful!");
  
  // Set the calibration weight (g) to calibrate the weight sensor automatically.
  MyScale.setCalWeight(100);
  // Set the calibration threshold (g).
  MyScale.setThreshold(30);
  // Display the current calibration value. 
  Serial.print("\nCalibration Value: "); Serial.println(MyScale.getCalibration());
  MyScale.setCalibration(MyScale.getCalibration());
  delay(1000);

  // Check the connection status between the AS7341 visible light sensor and the Beetle ESP32-C3.
  while (as7341.begin() != 0) {
    Serial.println("AS7341 initialization is failed!");
    err_msg();
    delay(1000);
  }
  Serial.println("AS7341 initialization is successful!");

  // Enable the built-in LED on the AS7341 sensor.
  as7341.enableLed(true);

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, pass);
  // Attempt to connect to the WiFi network:
  while(WiFi.status() != WL_CONNECTED){
    // Wait for the connection:
    delay(500);
    Serial.print(".");
  }
  // If connected to the network successfully:
  Serial.println("Connected to the WiFi network successfully!");
  u8g2.firstPage();  
  do{
    u8g2.setFont(u8g2_font_open_iconic_all_8x_t);
    u8g2.drawGlyph(/* x=*/32, /* y=*/0, /* encoding=*/247);  
  }while(u8g2.nextPage());
  delay(2000);
}

void loop() {
  get_Weight();
  get_Visual_Light();
  activate_Geiger_counter();

  // Show the collected data on the screen.
  home_screen(8, 90, 20);

  // Transmit the collected data to the PHP web application with the selected irradiation dose class:
  if(!digitalRead(button_A)) make_a_get_request("0");
  if(!digitalRead(button_B)) make_a_get_request("1");
  if(!digitalRead(button_C)) make_a_get_request("2");
}

void make_a_get_request(String _class){
  // Connect to the web application named food_irradiation_data_logger. Change '80' with '443' if you are using SSL connection.
  if (client.connect(server, 80)){
    // If successful:
    Serial.println("\nConnected to the web application successfully!");
    // Create the query string:
    String query = application+"?weight="+String(weight)+"&F1="+data1.ADF1+"&F2="+data1.ADF2+"&F3="+data1.ADF3+"&F4="+data1.ADF4+"&F5="+data2.ADF5+"&F6="+data2.ADF6+"&F7="+data2.ADF7+"&F8="+data2.ADF8;
    query += "&CPM="+String(geiger.getCPM())+"&nSv="+String(geiger.getnSvh())+"&uSv="+String(geiger.getuSvh());
    query += "&class="+_class;
    // Make an HTTP Get request:
    client.println("GET " + query + " HTTP/1.1");
    client.println("Host: 192.168.1.20");
    client.println("Connection: close");
    client.println();
  }else{
    Serial.println("\nConnection failed to the web application!");
    err_msg();
  }
  delay(2000); // Wait 2 seconds after connecting...
  // If there are incoming bytes available, get the response from the web application.
  String response = "";
  while (client.available()) { char c = client.read(); response += c; }
  if(response != "" && response.indexOf("Data received and saved successfully!") > 0){
    Serial.println("Data registered successfully!");
    u8g2.firstPage();  
    do{
      //u8g2.setBitmapMode(true /* transparent*/);
      u8g2.drawXBMP( /* x=*/36 , /* y=*/0 , /* width=*/50 , /* height=*/50 , data_colllect_bits);
      u8g2.setFont(u8g2_font_4x6_tr);
      u8g2.drawStr(6, 55, "Data registered successfully!");
    }while(u8g2.nextPage());
  }
}

void home_screen(int y, int x, int s){
  u8g2.firstPage();  
  do{
    u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
    u8g2.drawGlyph(/* x=*/0, /* y=*/y-3, /* encoding=*/142);
    u8g2.drawGlyph(/* x=*/0, /* y=*/y+s-3, /* encoding=*/259);
    u8g2.drawGlyph(/* x=*/0, /* y=*/y+(2*s)-3, /* encoding=*/280);
    u8g2.setFont(u8g2_font_freedoomr10_mu);
    u8g2.drawStr(25, y, "WEIGHT:"); drawNumber(x, y, weight);
    u8g2.drawStr(25, y+s, "F1:"); drawNumber(x, y+s, data1.ADF1);
    u8g2.drawStr(25, y+(2*s), "CPM:"); drawNumber(x, y+(2*s), geiger.getCPM());
  }while(u8g2.nextPage());
}

void activate_Geiger_counter(){
  // Initialize the Geiger counter module and enable the external interrupt.
  geiger.start();
  delay(3000);
  // If necessary, pause the count and turn off the external interrupt trigger.
  geiger.pause();
  
  // Evaluate the current CPM (Counts per Minute) by dropping the edge pulse within 3 seconds: the error is ±3CPM.
  Serial.print("\nCPM: "); Serial.println(geiger.getCPM());
  // Get the current nSv/h (nanoSieverts per hour).
  Serial.print("nSv/h: "); Serial.println(geiger.getnSvh());
  // Get the current μSv/h (microSieverts per hour).
  Serial.print("μSv/h: "); Serial.println(geiger.getuSvh());
}

void get_Weight(){
  weight = MyScale.readWeight();
  if(weight < 0.5) weight = 0;
  Serial.print("\nWeight: "); Serial.print(weight); Serial.println(" g");
  delay(1000);
}

void get_Visual_Light(){
  // Start spectrum measurement:
  // Channel mapping mode: 1.eF1F4ClearNIR
  as7341.startMeasure(as7341.eF1F4ClearNIR);
  // Read the value of sensor data channel 0~5, under eF1F4ClearNIR
  data1 = as7341.readSpectralDataOne();
  // Channel mapping mode: 2.eF5F8ClearNIR
  as7341.startMeasure(as7341.eF5F8ClearNIR);
  // Read the value of sensor data channel 0~5, under eF5F8ClearNIR
  data2 = as7341.readSpectralDataTwo();
  // Print data:
  Serial.print("\nF1(405-425nm): "); Serial.println(data1.ADF1);
  Serial.print("F2(435-455nm): "); Serial.println(data1.ADF2);
  Serial.print("F3(470-490nm): "); Serial.println(data1.ADF3);
  Serial.print("F4(505-525nm): "); Serial.println(data1.ADF4);
  Serial.print("F5(545-565nm): "); Serial.println(data2.ADF5);
  Serial.print("F6(580-600nm): "); Serial.println(data2.ADF6);
  Serial.print("F7(620-640nm): "); Serial.println(data2.ADF7);
  Serial.print("F8(670-690nm): "); Serial.println(data2.ADF8);
  // CLEAR and NIR:
  Serial.print("Clear_1: "); Serial.println(data1.ADCLEAR);
  Serial.print("NIR_1: "); Serial.println(data1.ADNIR);
  Serial.print("Clear_2: "); Serial.println(data2.ADCLEAR);
  Serial.print("NIR_2: "); Serial.println(data2.ADNIR);
  delay(1000);
}

void err_msg(){
  // Show the error message on the SSD1309 transparent display.
  u8g2.firstPage();  
  do{
    //u8g2.setBitmapMode(true /* transparent*/);
    u8g2.drawXBMP( /* x=*/44 , /* y=*/0 , /* width=*/40 , /* height=*/40 , error_bits);
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 47, "Check the serial monitor to see");
    u8g2.drawStr(40, 55, "the error!");
  }while(u8g2.nextPage());
}

void drawNumber(int x, int y, int __){
    char buf[7];
    u8g2.drawStr(x, y, itoa(__, buf, 10));
}
