#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Wire.h"

//WiFi properties
const char* ssid = "SSID";
const char* password = "password";
const char* host = "192.168.1.1";
const int port = 3080;

const int MAX_ATTEMPTS = 5;
int attempts = 0;
bool boot_up = true;
 
int statusLed = 2; // GPIO16 (2 for chinese NodeMCU)

//Global sensor object
Adafruit_BME280 mySensor;

char MAC_char[17];

void setup() {
  //Init I2C interface using GPIO pin 12 for SLA and 14 for SCL
  Wire.begin(12,14);
  
  Serial.begin(115200);
  delay(10);

  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);
    
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  uint8_t MAC_array[6];
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array) - 1; ++i){
    sprintf(MAC_char,"%s%02x:",MAC_char,MAC_array[i]);
  }
  sprintf(MAC_char,"%s%02x",MAC_char,MAC_array[5]);
  Serial.print("WiFi MAC address: ");
  Serial.println(MAC_char);
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(statusLed, HIGH);
    delay(250);
    digitalWrite(statusLed, LOW);
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(statusLed, LOW);

  //Print signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
 
  // Print the IP address
  Serial.print("Assigned IP: ");
  Serial.println(WiFi.localIP());
  // connect to BME280 sensor
  if (!mySensor.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}
 
void loop() {
  if (boot_up == true) {
    // Wait for sensor calibration before sending first measurement
    Serial.println("First boot, wait for sensor calibration...");
    delay(60UL * 1000UL);
    boot_up = false;
  }

  Serial.println("Connecting to server...");

  WiFiClient client;
  if (!client.connect(host, port)) {
    attempts = attempts + 1;
    Serial.print("Error connecting to server ");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port);
    Serial.print(" (");
    Serial.print(attempts);
    Serial.println(")");
    if (attempts >= MAX_ATTEMPTS) {
      Serial.println("Reached MAX_ATTEMPTS while trying to connect to server, will try again in 30 minutes..");
      attempts = 0;
      //Reached MAX_ATTEMPTS, retry again after 30 minutes
      delay(30UL * 60UL * 1000UL); //30 minutes
    }
    return;
  }
  Serial.println("Connected to server ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  delay(250);
  digitalWrite(statusLed, HIGH);
  delay(250);
  digitalWrite(statusLed, LOW);

  Serial.println("Retrieving sensor data...");
  float tempC = mySensor.readTemperature();
  float pressure = mySensor.readPressure();
  float humidity = mySensor.readHumidity();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sensorIP"] = WiFi.localIP().toString();
  root["sensorMAC"] = MAC_char;
  root["tempC"] = String(tempC, 1);
  root["pressure"] = String(pressure, 2);
  root["humidity"] = String(humidity, 0);

  Serial.print("Sending data: ");
  root.printTo(Serial);
  Serial.println("");
  root.printTo(client);
  delay(100);
  client.flush();
  client.stop();
  Serial.println("Connection closed");
  Serial.println("");
  
  delay(250);
  digitalWrite(statusLed, HIGH);
  delay(250);
  digitalWrite(statusLed, LOW);

  delay(30UL * 60UL * 1000UL); //30 minutes
  //delay(15000);
}
