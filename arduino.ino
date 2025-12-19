#include <DHT.h>

// DHT11 setup
#define DHTPIN 2        // D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// HC-SR04 setup
#define TRIGPIN 3       // D3
#define ECHOPIN 4       // D4

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
}

void loop() {
  // DHT11 readings
  float tempC = dht.readTemperature();
  float hum = dht.readHumidity();

  // HC-SR04 readings
  long duration;
  float distance;
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = duration * 0.034 / 2; // cm

  // Print as CSV: Temp, Humidity, Distance
  Serial.print(tempC); Serial.print(",");
  Serial.print(hum); Serial.print(",");
  Serial.println(distance);

  delay(5000); // 2 sec delay
}
