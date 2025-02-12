#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Servo.h>

// Sensor pins configuration
#define DHTPIN D5          // DHT11 data pin
#define DHTTYPE DHT11      // DHT sensor type
#define SOIL_MOISTURE_PIN D6  // Digital soil moisture sensor
#define LIGHT_SENSOR_PIN A0   // Photoresistor analog pin
#define SERVO_PIN D2       // Servo motor control pin

// Network credentials (replace with your own)
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor
Servo myservo;              // Create servo object

WiFiServer server(80);      // Web server on port 80

void setup() {
  Serial.begin(115200);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  dht.begin();               // Start DHT sensor
  myservo.attach(SERVO_PIN); // Attach servo to pin

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
  server.begin();            // Start web server
}

void loop() {
  // Read all sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soilMoisture = digitalRead(SOIL_MOISTURE_PIN);
  int lightValue = analogRead(LIGHT_SENSOR_PIN);
  int servoAngle = map(lightValue, 0, 1023, 0, 180);
  
  // Update servo position
  myservo.write(servoAngle);
  delay(15);  // Short delay for servo movement

  // Handle web client
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    unsigned long currentTime = millis();
    unsigned long previousTime = currentTime;
    
    while (client.connected() && currentTime - previousTime <= 2000) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Send HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Generate HTML page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Aqua Floralis</title>");
            client.println("<style>body{font-family: Arial; margin: 20px; background: #f0f0f0;}");
            client.println(".container{background: white; padding: 20px; border-radius: 10px;}");
            client.println(".sensor{margin: 15px 0; padding: 10px; border-left: 4px solid;}");
            client.println(".alert{color: #ff4444; font-weight: bold;}</style></head>");
            client.println("<body><div class='container'>");
            client.println("<h1>🌱 Aqua Floralis</h1>");
            
            // Environmental data
            client.println("<div class='sensor' style='border-color: #2196F3;'>");
            client.println("<h3>🌡️ Environment</h3>");
            client.printf("<p>Temperature: %.1f°C</p>", temperature);
            client.printf("<p>Humidity: %.1f%%</p></div>", humidity);
            
            // Soil moisture
            client.println("<div class='sensor' style='border-color: " + String(soilMoisture ? "#4CAF50" : "#ff4444") + ";'>");
            client.println("<h3>💧 Soil</h3>");
            client.printf("<p>Status: %s</p>", soilMoisture ? "<span style='color:#4CAF50;'>Moist</span>" : "<span class='alert'>DRY - WATER NEEDED!</span>");
            client.printf("<p>Digital reading: %d</p></div>", soilMoisture);
            
            // Light and servo
            client.println("<div class='sensor' style='border-color: #FFC107;'>");
            client.println("<h3>☀️ Light</h3>");
            client.printf("<p>Intensity: %d</p>", lightValue);
            client.printf("<p>Servo angle: %d°</p></div>", servoAngle);
            
            client.println("</div></body></html>");
            client.println();
            break;
          }
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}