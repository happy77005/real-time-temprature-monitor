#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>

const char *ssid = "ACTFIBERNET";
const char *password = "xxxxxxxx";


#define API_KEY " "
#define DATABASE_URL "  "

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


const long utcOffsetInSeconds = 5 * 3600 + 30 * 60;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
const long interval = 300000;  

void setup() {
  Serial.begin(115200);

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");


  timeClient.begin();
  dht.begin();
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Initialized");
}

void loop() {
  timeClient.update();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendDataToFirebase();
  }

  delay(1000);
}

void sendDataToFirebase() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int hour = timeClient.getHours();
  String period = (hour >= 12) ? "PM" : "AM";
  if (hour > 12) hour -= 12;
  if (hour == 0) hour = 12;

  String timestamp = String(hour) + ":" + (timeClient.getMinutes() < 10 ? "0" : "") + String(timeClient.getMinutes()) + " " + period;

  String path = "/weather_data/" + timestamp;

  if (Firebase.setFloat(fbdo, path + "/Temperature", temperature)) {
    Serial.println("Temperature sent to Firebase");
  } else {
    Serial.println("Failed to send temperature: " + fbdo.errorReason());
  }

  if (Firebase.setFloat(fbdo, path + "/Humidity", humidity)) {
    Serial.println("Humidity sent to Firebase");
  } else {
    Serial.println("Failed to send humidity: " + fbdo.errorReason());
  }
}