#include <ESP8266WiFi.h>
#include "DHT.h"

// Wifi - Details
const char* ssid = "Mysore Sandal";      // your Wi-Fi SSID
const char* password = "venky610";               // ⚠️ enter your Wi-Fi password here

// ---------- ThingSpeak details ----------
String apiKey = "D22T37BRMA3MFOTK";     // your ThingSpeak Write API key
const char* server = "api.thingspeak.com";

// DHT11 setup
#define DHTPIN 4        // D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// HC-SR04 setup
#define TRIGPIN 12      // D6
#define ECHOPIN 14      // D5

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(2000);
  dht.begin();
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // DHT11 Readings
  float tc = dht.readTemperature(false);  // Celsius
  float tf = dht.readTemperature(true);   // Fahrenheit
  float hu = dht.readHumidity();          // Humidity

  // Ultrasonic Readings
  long duration;
  float distance;

  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);

  duration = pulseIn(ECHOPIN, HIGH);
  distance = duration * 0.034 / 2; // Distance in cm

  // Print results
  Serial.println("----------------------------");
  if (isnan(tc) || isnan(tf) || isnan(hu)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temp: "); Serial.print(tc);
    Serial.print(" °C, "); Serial.print(tf);
    Serial.print(" °F, Hum: "); Serial.print(hu);
    Serial.println(" %");
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Send data to ThingSpeak
  if (client.connect(server, 80)) {
    String postStr = apiKey;
    postStr += "&field1=" + String(tc);
    postStr += "&field2=" + String(hu);
    postStr += "&field3=" + String(distance);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("Data sent to ThingSpeak!");
  }

  client.stop();
  delay(2000); // Wait 20 seconds before next update (min 15 sec allowed)
}
