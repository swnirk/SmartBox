/*******************************************************************
    A telegram bot that sends you a message when ESP
    starts up

    Parts:
    D1 Mini ESP8266 * - http://s.click.aliexpress.com/e/uzFUnIe
    (or any ESP8266 board)

      = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Wifi network station credentials
#define WIFI_SSID "MTLINK"
#define WIFI_PASSWORD "LeninIsAlive1917"


#define PIN_TRIG D6
#define PIN_ECHO D5
// Telegram BOT Token (Get from Botfather)
// ------- Telegram config --------
#define BOT_TOKEN "6250246633:AAGHeih1vRyHBZ0k6kH6TOCaGdgh-D_v2yE"  // your Bot Token (Get from Botfather)
#define CHAT_ID "263170564" // Chat ID of where you want the message to go (You can use MyIdBot to get the chat ID)

int teaLowLevelFlag1 = false;
int teaLowLevelFlag2 = false;
int teaLowLevelFlag3 = false;
long duration;
long cm = 10;
int batteryLowLevelFlag = false;
int batteryInPin  = A0;    // Analog input pin
int sensorValue;           // Analog Output of Sensor
float calibration = 0.31;  // Check Battery voltage using multimeter & add/subtract the value
int bat_percentage;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);
  Serial.println();

  // Wait for serial to initialize.
  while(!Serial) { }

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

 // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  bot.sendMessage(CHAT_ID, "Bot started up", "");
}
void sendNotification(){
  String message = "";
  if(teaLowLevelFlag1 || teaLowLevelFlag2 || teaLowLevelFlag3){
    message = "Please buy more tea!";
  }
  else if(batteryLowLevelFlag){
    message = "Please charge the battery!";
  }
  else{
    message = "Error Flag!";
  }
  message.concat("\n");
  if(bot.sendMessage(CHAT_ID, message, "")){
    Serial.println("TELEGRAM Successfully sent");
  }
  
  if(teaLowLevelFlag1 || teaLowLevelFlag2 || teaLowLevelFlag3){
    teaLowLevelFlag1 = false;
    teaLowLevelFlag2 = false;
    teaLowLevelFlag3 = false;
  }
  else if(batteryLowLevelFlag){
    batteryLowLevelFlag = false;
  }
}

void echoLocator(int* flag){
  Serial.println("EchoLocator is working!");
  
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  duration = pulseIn(PIN_ECHO, HIGH);

  cm = (duration / 2) / 29.1;

  Serial.print("Расстояние до объекта: ");
  Serial.print(cm);
  Serial.println(" см.");

  if(cm >= 15){
    Serial.println("Distance exceeds 15 cm!");
    *flag = true;
  }
}

void batteryCheck(){
  sensorValue = analogRead(batteryInPin);
  float voltage = (((sensorValue * 3.3) / 1024) * 2 - calibration); //multiply by two as voltage divider network is 100K & 100K Resistor
  Serial.println("voltage: ");
  Serial.println(voltage);

  bat_percentage = mapfloat(voltage, 2.8, 4.2, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage

  Serial.println("percentage: ");
  Serial.println(bat_percentage);
  Serial.println("%");

  if(bat_percentage <= 10){
    batteryLowLevelFlag = true;
  }
}

void loop() {

  echoLocator(&teaLowLevelFlag1);
  delay(100);
  echoLocator(&teaLowLevelFlag2);
  if(teaLowLevelFlag1 && teaLowLevelFlag2){
    sendNotification();  
  }
  else if(teaLowLevelFlag1 || teaLowLevelFlag2){
    delay(100);
    echoLocator(&teaLowLevelFlag3);
    if(teaLowLevelFlag3){
      sendNotification();
    }
  }
  
  teaLowLevelFlag1 = false;
  teaLowLevelFlag2 = false;
  teaLowLevelFlag3 = false;

  batteryCheck();
  if(batteryLowLevelFlag){
    sendNotification();
  }
  //ESP.deepSleep(1e6);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
