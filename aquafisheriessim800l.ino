#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

Servo servo;

const int relay = 4;
const int servopin = 5;

// Define the SIM800L software serial connection
SoftwareSerial sim800Serial(7, 8); // RX, TX

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(relay, OUTPUT);
  servo.attach(servopin);
  servo.write(10);

  // Initialize the SIM800L module
  sim800Serial.begin(9600);

  // Connect to the cellular network (replace with your specific settings)
  sim800Serial.println("AT"); // Replace with your AT commands
  delay(1000);

  Serial.println("Waiting for network...");
  while (!isConnected()) {
    delay(1000);
  }
  Serial.println("Connected to the cellular network");
}

bool isConnected() {
  sim800Serial.println("AT+CREG?");
  delay(100);
  while (sim800Serial.available()) {
    if (sim800Serial.find("+CREG: 0,1")) {
      return true;
    }
  }
  return false;
}

void sendPostRequest(const char* apiEndpoint, const char* postData) {
  sim800Serial.print("AT+HTTPINIT\r\n");
  delay(1000);

  sim800Serial.print("AT+HTTPPARA=\"CID\",1\r\n");
  delay(1000);

  sim800Serial.print("AT+HTTPPARA=\"URL\",\"");
  sim800Serial.print(apiEndpoint);
  sim800Serial.print("\"\r\n");
  delay(1000);

  sim800Serial.print("AT+HTTPDATA=");
  sim800Serial.print(strlen(postData));
  sim800Serial.print(",5000\r\n");
  delay(1000);

  sim800Serial.print(postData);
  delay(1000);

  sim800Serial.print("AT+HTTPACTION=1\r\n");
  delay(1000);

  sim800Serial.print("AT+HTTPREAD\r\n");
  delay(1000);

  // Read the HTTP response
  while (sim800Serial.available()) {
    char c = sim800Serial.read();
    Serial.print(c);
  }

  // End the HTTP session
  sim800Serial.print("AT+HTTPTERM\r\n");
  delay(1000);
}

void loop() {
  float temperature = dht.readTemperature();

  if (temperature >= 29.50 && temperature <= 29.60) {
    digitalWrite(relay, HIGH);
    servo.write(10);
    Serial.println("Pump Activated");

    // Send a POST request to the API endpoint
    const char* apiEndpoint = "https://titan.africa/api/tmessage";
    const char* jsonMessage = "{\"message\":\"Pump Activated\"}";

    sendPostRequest(apiEndpoint, jsonMessage);
  }
  else if (temperature >= 36.20 && temperature <= 36.21) {
    digitalWrite(relay, LOW);
    servo.write(100);
    Serial.println("Food feeder Activated");

    // Send a POST request to the API endpoint
    const char* apiEndpoint = "https://titan.africa/api/tmessage";
    const char* jsonMessage = "{\"message\":\"Food feeder Activated\"}";

    sendPostRequest(apiEndpoint, jsonMessage);
  }
  else {
    Serial.println("Checking");
  }

  if (temperature <= 28.40) {
    digitalWrite(relay, LOW);
    servo.write(10);
    Serial.println("Normal operation");
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  delay(1000);
}
