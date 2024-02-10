#include <WiFi.h>
#include <HTTPClient.h>

WiFiServer server(80);
const char *ssid = "ADT";
const char *password = "adt@12345";
const char *server_address = "192.168.1.2";
const int serverPort = 5000;
const int analogPin = 34;
const int lamp1Pin = 2;
const int lamp2Pin = 4;
const int lamp3Pin = 5;
const int lamp4Pin = 18;
const int lamp5Pin = 19;

unsigned long previousMillis = 0;
const long interval = 100; // Adjust the interval based on your needs

void setup() {
  Serial.begin(115200);
  pinMode(lamp1Pin, OUTPUT);
  pinMode(lamp2Pin, OUTPUT);
  pinMode(lamp3Pin, OUTPUT);
  pinMode(lamp4Pin, OUTPUT);
  pinMode(lamp5Pin, OUTPUT);

  // Connect to WiFi
  connectToWiFi();

  // Start server
  server.begin();
}

void loop() {
  handleWiFiClientRequests();
  handlePinData();
  // handleEmergencyLamps();
}

void blinkLamp(int lampPin, int blinkDelay) {
  digitalWrite(lampPin, HIGH);
  delay(blinkDelay / 2); // On for half of the blink delay
  digitalWrite(lampPin, LOW);
  delay(blinkDelay / 2); // Off for the other half of the blink delay
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void handleWiFiClientRequests() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New WiFi client");

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print("Received from client: ");
        Serial.println(line);
      }
    }

    client.stop();
    Serial.println("WiFi client disconnected");
  }
}

void handlePinData() {
  int sensorValue = analogRead(analogPin);
  Serial.println("Sensor Value: " + String(sensorValue));

  // Send the sensor value to the HTTP server
  sendSensorValueToHTTPServer(sensorValue);

  // Send the sensor value over WiFi
  sendSensorValueOverWiFi(sensorValue);

 // Fetch emergency level over WiFi
  int emergencyLevel = fetchEmergencyLevelOverWiFi();
  Serial.println("Received Emergency Level: " + String(emergencyLevel));
  delay(1000);

 // Handle emergency lamps based on the received level
  handleEmergencyLamps(emergencyLevel);

}

void sendSensorValueToHTTPServer(int sensorValue) {
  HTTPClient http;
  String url = "/update";
  String postData = "value=" + String(sensorValue);

  http.begin(server_address, serverPort, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);

  if (httpCode == 200) {
    Serial.println("HTTP POST request successful");
    Serial.println(http.getString());
  } else {
    Serial.println("HTTP POST request failed");
    Serial.println("HTTP Code: " + String(httpCode));
  }

  
  http.end();
  delay(1000);
}

void sendSensorValueOverWiFi(int value) {
  WiFiClient client;
  String url = "/update?value=" + String(value);

  if (client.connect(server_address, serverPort)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server_address + "\r\n" +
                 "Connection: close\r\n\r\n");

    delay(1000);

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }

    client.stop();
    Serial.println("Server disconnected");
  } else {
    Serial.println("Connection to server failed");
  }
}


int fetchEmergencyLevelOverWiFi() {
  WiFiClient client;
  String url = "/emergency";

  if (client.connect(server_address, serverPort)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server_address + "\r\n" +
                 "Connection: close\r\n\r\n");

    delay(1000);

    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.print("read this: ");
      Serial.println(line);
      if (line.startsWith("{\"level\":")) {
        int emergencyLevel = line.substring(9).toInt(); // Adjust the substring index
        Serial.println("we are turning emergency level");
        Serial.write(emergencyLevel);
        return emergencyLevel;
      }
    }

    client.stop();
    Serial.println("Server disconnected");
  } else {
    Serial.println("Connection to server failed");
  }

  return 0; // Return an appropriate default value if the fetch fails
}

void handleEmergencyLamps(int emergencyLevel) {
  unsigned long currentMillis = millis();

  switch (emergencyLevel) {
    case 1:
      // Blink lamp1 continuously
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        blinkLamp(lamp1Pin, 750); // Blink every 750 milliseconds
      }
      break;
    case 2:
      // Blink lamp2 continuously with faster speed
      if (currentMillis - previousMillis >= interval / 2) {
        previousMillis = currentMillis;
        blinkLamp(lamp2Pin, 600); // Blink every 600 milliseconds
      }
      break;
    case 3:
      // Blink lamp3 continuously with even faster speed
      if (currentMillis - previousMillis >= interval / 4) {
        previousMillis = currentMillis;
        blinkLamp(lamp3Pin, 450); // Blink every 450 milliseconds
      }
      break;
    case 4:
      // Blink lamp4 continuously with even faster speed
      if (currentMillis - previousMillis >= interval / 8) {
        previousMillis = currentMillis;
        blinkLamp(lamp4Pin, 300); // Blink every 300 milliseconds
      }
      break;
    case 5:
      // Blink lamp5 continuously with even faster speed
      if (currentMillis - previousMillis >= interval / 16) {
        previousMillis = currentMillis;
        blinkLamp(lamp5Pin, 150); // Blink every 150 milliseconds
      }
      break;
    default:
      // Turn off all lamps if emergency level is not recognized
      digitalWrite(lamp1Pin, LOW);
      digitalWrite(lamp2Pin, LOW);
      digitalWrite(lamp3Pin, LOW);
      digitalWrite(lamp4Pin, LOW);
      digitalWrite(lamp5Pin, LOW);
      break;
  }
}