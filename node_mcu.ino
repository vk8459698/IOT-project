#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h> // For IFTTT

#define RELAYPIN D5       // Relay output
#define RX_PIN D6         // NodeMCU receives Arduino TX here
#define SERIAL_BAUD 9600  // Must match Arduino TX

const char* ssid = "Galaxy A03s b6b3";
const char* password = "gfut6454";
String apiKey = "D22T37BRMA3MFOTK";
const char* server = "api.thingspeak.com";

float temp, hum, dist;

// SoftwareSerial for Arduino communication
SoftwareSerial arduinoSerial(RX_PIN, D8); // RX=D6, TX unused

WiFiClient client;

// IFTTT Webhook settings
String iftttKey = "cdrShibkwHnsvPMoLaPWMh";        
String iftttEvent = "sensor_alert";        

void setup() {
  Serial.begin(115200);
  arduinoSerial.begin(SERIAL_BAUD);

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  // Read from Arduino
  while (arduinoSerial.available()) {
    String data = arduinoSerial.readStringUntil('\n');
    data.trim();
    if (data.length() > 0) {
      int idx1 = data.indexOf(',');
      int idx2 = data.lastIndexOf(',');

      if (idx1 > 0 && idx2 > idx1) {
        temp = data.substring(0, idx1).toFloat();
        hum  = data.substring(idx1 + 1, idx2).toFloat();
        dist = data.substring(idx2 + 1).toFloat();

        Serial.print("Temp: "); Serial.print(temp);
        Serial.print(" C, Hum: "); Serial.print(hum);
        Serial.print("%, Dist: "); Serial.println(dist);

        // Relay logic
        bool relayState = false;
        if (temp > 20.0) {
          digitalWrite(RELAYPIN, HIGH);
          relayState = true;
        } else {
          digitalWrite(RELAYPIN, LOW);
          relayState = false;
        }

        sendToThingSpeak(relayState);

        // IFTTT alert if thresholds exceeded
        if (temp > 35 || dist < 5) {
          sendIFTTTAlert(temp, dist);
        }
      }
    }
  }
}

// Send sensor data + relay state to ThingSpeak
void sendToThingSpeak(bool relayState) {
  if (client.connect(server, 80)) {
    String postStr = "api_key=" + apiKey;
    postStr += "&field1=" + String(temp);
    postStr += "&field2=" + String(hum);
    postStr += "&field3=" + String(dist);
    postStr += "&field4=" + String(relayState ? 1 : 0); // Relay state

    client.print("POST /update HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\r\n\r\n");
    client.print(postStr);

    Serial.println("Data + relay state sent to ThingSpeak!");
  }
  client.stop();
}

// Send alert via IFTTT webhook
void sendIFTTTAlert(float tempVal, float distVal) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://maker.ifttt.com/trigger/" + iftttEvent + "/with/key/" + iftttKey;
    url += "?value1=Temp:" + String(tempVal) + "&value2=Dist:" + String(distVal);

    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode == 200) Serial.println(" IFTTT alert sent!");
    else Serial.println(" IFTTT alert failed");
    http.end();
  }
}
