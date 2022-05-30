#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPtimeESP.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <time.h>
#include "EEPROM.h"


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/************************* Oled spi *********************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int measurePin = A0;
int ledPower = 2;
 
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
 
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

/************************* WiFi Access Point *********************************/
String WLAN_SSID;                        //string variable to store ssid
String WLAN_PASS; 
unsigned long rst_millis;
/************************* cloudmqtt Setup *********************************/

#define serveruri "driver.cloudmqtt.com"
#define port       18643
#define username  "cqbfckol"
#define password  "mpSkyZ4D1N6f"

/*************************pin Setup *********************************/
const int DHTpin = 13; //đọc data từ chân  gpio4

#define LENGTH(x) (strlen(x) + 1)   // length of char string
#define EEPROM_SIZE 200
/*************************Do am dat Setup ******************************/
int value_soil, realvalue;
/*************************DHT Setup ************************************/
const int DHTtype = DHT11; //khai báo loại cảm biến
DHT dht(DHTpin, DHTtype);  //khởi tạo dht
float humidity;
float temperature;
unsigned long readTime;
unsigned long timeMqtt;
unsigned long feedBackTime;
unsigned long alarmTime;
unsigned long confirmTime;

/*************************Instance Setup ************************************/
//tạo 1 client
WiFiClient myClient;
//**************************** Server NTP ***********************************
NTPtime NTPch("vn.pool.ntp.org");

strDateTime dateTime;
byte nowHour = 0;     // Gio
byte nowMinute = 0;   // Phut
byte onHour = 0;     // Gio On
byte onMinute = 0;   // Phut On
byte offHour = 0;     // Gio Off
byte offMinute = 0;   // Phut Off

/*************************** Sketch Code ************************************/
void sensorRead();
void reconnect();
/*************************** Smartconfig ************************************/
Ticker ticker;

bool in_smartconfig = false;
void enter_smartconfig()
{
  if (in_smartconfig == false) {
    in_smartconfig = true;
//    ticker.attach(0.1, tick);
    WiFi.beginSmartConfig();
  }
}

void exit_smart()
{
  in_smartconfig = false;
}
/******************************************************************/
//*****************khởi tạo pubsubclient***************************
PubSubClient mqtt(serveruri, port, callback, myClient);

void setup()
{
  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Do An Quan trac!");
  display.display(); 
  WiFi.disconnect();
  EEPROM.begin(EEPROM_SIZE);
  WLAN_SSID = readStringFromFlash(0); // Read SSID stored at address 0
  Serial.print("SSID = ");
  Serial.println(WLAN_SSID);
  WLAN_PASS = readStringFromFlash(40); // Read Password stored at address 40
  Serial.print("psss = ");
  Serial.println(WLAN_PASS);
  WiFi.begin(WLAN_SSID.c_str(), WLAN_PASS.c_str());
  
  dht.begin(); //khởi động cảm biến
  pinMode(5, INPUT_PULLUP); //doc cam bien do am dat
  //  Connect to WiFi access point.
  Serial.println();
  Serial.println();
  display.setCursor(0, 10);
  // Display static text
  display.clearDisplay();
  display.setCursor(0, 10);
  // Display static text
  display.println("Dang ket noi wifi");
  display.display();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(5000);
  while (WiFi.status() != WL_CONNECTED)
  {
    display.clearDisplay();
   display.setCursor(0, 10);
  // Display static text
   display.println("Che do smart config");
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
     
    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
      display.setCursor(0, 20);
  // Display static text
  display.println("Cho ket noi");
  display.display();
    }

    Serial.println("");
    Serial.println("SmartConfig received.");

    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi Connected.");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // read the connected WiFi SSID and password
    WLAN_SSID = WiFi.SSID();
    WLAN_PASS = WiFi.psk();
    Serial.print("SSID:");
    Serial.println(WLAN_SSID);
    Serial.print("PSS:");
    Serial.println(WLAN_PASS);
    Serial.println("Store SSID & PSS in Flash");
    writeStringToFlash(WLAN_SSID.c_str(), 0); // storing ssid at address 0
    writeStringToFlash(WLAN_PASS.c_str(), 40); // storing pss at address 40
    WiFi.begin(WLAN_SSID.c_str(), WLAN_PASS.c_str());
    delay(5000);
  }
     display.clearDisplay();
     display.setCursor(0, 10);
  // Display static text
    display.println("Ket noi Wifi thanh cong");
     display.display();
     delay(1000);
  // Get MacAddress and remove ":"
  String MacAddress = WiFi.macAddress();
  MacAddress.remove(2,1);
  MacAddress.remove(4,1);
  MacAddress.remove(6,1);
  MacAddress.remove(8,1);
  MacAddress.remove(10,1);
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(MacAddress);

  // kết nối với mqtt server
  while (1)
  {
    delay(500);
    if (mqtt.connect("ESP8266", username, password))
      break;
  }
  Serial.println("connected to MQTT server.....");

  //set mode
  pinMode(DHTpin, INPUT);
  pinMode(ledPower,OUTPUT);
  
}

void loop()
{
  if (WiFi.status() == WL_DISCONNECTED )
  {
    Serial.println("Restarting the ESP");
    ESP.restart(); 
  }

  //làm mqtt luôn sống
  mqtt.loop();

  //phản hồi trạng thái relay lên server
  if (mqtt.connected())
  {
    //check if 5 seconds has elapsed since the last time we read the sensors.
    if (millis() > readTime + 2000)
    {
      sensorRead();
    }
  }

}

//************************* hàm đọc giá trị sensor *****************
void sensorRead()
{
  readTime = millis();
  humidity = dht.readHumidity();       //đọc nhiệt độ
  temperature = dht.readTemperature(); // đọc độ ẩm
  digitalWrite(ledPower,LOW); // Bật IR LED
  delayMicroseconds(samplingTime);  //Delay 0.28ms
  voMeasured = analogRead(A0);
  delayMicroseconds(deltaTime); //Delay 0.04ms
  digitalWrite(ledPower,HIGH); // Tắt LED
  delayMicroseconds(sleepTime); //Delay 9.68ms
  calcVoltage = voMeasured*(5.0/1024);
  dustDensity = (0.17*calcVoltage-0.1)*100;
  if ( dustDensity < 0)
  {
    dustDensity = 0.00;
  }
    display.clearDisplay();
  Serial.print("Dust: ");
  Serial.println(dustDensity);
   display.setCursor(0, 10);
  // Display static text
  display.print("Nhiet do(C):");
  display.println(temperature);
   display.setCursor(0, 20);
  // Display static text
  display.print("Do am(%):");
  display.println(humidity);
   display.setCursor(0, 30);
  // Display static text
  display.print("Bui(mg/m3):");
  display.println(dustDensity);
  display.display();
  if (millis()>timeMqtt+60000){
    timeMqtt= millis();
  // Get MacAddress and remove ":"
  String MacAddress = WiFi.macAddress();
  MacAddress.remove(2,1);
  MacAddress.remove(4,1);
  MacAddress.remove(6,1);
  MacAddress.remove(8,1);
  MacAddress.remove(10,1);
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(MacAddress);

  // init mqttChannel
  String mqttMainChannel = "ESPs/enviroment/";
  String mqttChannel =MacAddress;

  // convert data to JSON
  StaticJsonDocument<200> doc;
  JsonObject content  = doc.to<JsonObject>();
  doc["temperature"] = temperature;
  doc["humidity"]   = humidity;
  doc["dust"]   = dustDensity;
 
  char content_string[256];
  serializeJson(content, content_string);

  // push data to mqtt
  mqtt.publish(mqttChannel.c_str(), content_string);
  }

}

//********************* hàm trả dữ liệu về *******************************
void callback(char *tp, byte *message, unsigned int length)
{
  
}

//*********************** hàm reconnect **********************
void reconnect()
{
  // lặp đến khi kết nối lại
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Attempting connection...");
    WiFi.reconnect();
    mqtt.connect("ESP8266", username, password);
    delay(500);
    // chờ để kết nối lại
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("reconnected");
      return;
    }
    else
    {
      Serial.print("failed to connect WiFi!!");
      Serial.println(" try again in 5 seconds...");
      // chờ 5s
      delay(5000);
    }
  }

  while (!mqtt.connected())
  {
    Serial.println("Attempting connection...");
    mqtt.connect("ESP8266", username, password);
    delay(500);
    // chờ để kết nối lại
    if (mqtt.connected())
    {
      Serial.println("reconnected");
        String MacAddress = WiFi.macAddress();
        MacAddress.remove(2,1);
        MacAddress.remove(4,1);
        MacAddress.remove(6,1);
        MacAddress.remove(8,1);
        MacAddress.remove(10,1);
        Serial.print("ESP8266 Board MAC Address:  ");
        Serial.println(MacAddress);
      return;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // chờ 5s
      delay(5000);
    }
  }
}



void writeStringToFlash(const char* toStore, int startAddr) {
  int i = 0;
  for (; i < LENGTH(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}


String readStringFromFlash(int startAddr) {
  char in[128]; // char array of size 128 for reading the stored data 
  int i = 0;
  for (; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}
