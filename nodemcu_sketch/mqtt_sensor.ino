#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Wire.h"
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#define WLAN_SSID             "SSID"
#define WLAN_PASSWORD         "password"
#define MQTT_SERVER        "MQTT host"
#define MQTT_PORT        1883
#define MQTT_USERNAME    "MQTT username"
#define MQTT_PASSWORD    "MQTT password"

#define STATUS_LED            LED_BUILTIN // GPIO16
//#define STATUS_LED          2 // GPIO2 (for chinese NodeMCU)

/************ Global State (you don't need to change this!) ******************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

bool boot_up = true;

//Global sensor object
Adafruit_BME280 mySensor;

char MAC_char[17];

Adafruit_MQTT_Publish sensorTopic = Adafruit_MQTT_Publish(&mqtt, "sensor");

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  //Init I2C interface using GPIO pin 12 for SLA and 14 for SCL
  Wire.begin(12,14);
  
  Serial.begin(115200);
  delay(10);

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
    
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
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
 
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(STATUS_LED, HIGH);
    delay(250);
    digitalWrite(STATUS_LED, LOW);
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(STATUS_LED, HIGH);

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
    Serial.print("First boot, wait for sensor calibration");
    for (int i = 0; i < 30; i++) {
      Serial.print(".");
      digitalWrite(STATUS_LED, LOW);
      delay(1000UL);
      Serial.print(".");
      digitalWrite(STATUS_LED, HIGH);
      delay(1000UL);
    }
    Serial.println(".");
    Serial.println("Done!");
    boot_up = false;
  }

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  delay(250);
  digitalWrite(STATUS_LED, LOW);
  delay(250);
  digitalWrite(STATUS_LED, HIGH);

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

  char message_buff[160];
  root.printTo(message_buff, sizeof(message_buff));

  Serial.print("Sending data: ");
  root.printTo(Serial);
  Serial.println("");

  if (! sensorTopic.publish(message_buff)) {
    Serial.println("Failed");
  } else {
    Serial.println("OK!");
  }

  Serial.println("Disconnecting from MQTT.");
  mqtt.disconnect();

  delay(30UL * 60UL * 1000UL); //30 minutes
  //delay(15000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  Serial.print("mqtt.connected() = ");
  Serial.println(mqtt.connected());
  if (mqtt.connected()) {
    return;
  }
  Serial.println("Connecting to MQTT...");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds…");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
