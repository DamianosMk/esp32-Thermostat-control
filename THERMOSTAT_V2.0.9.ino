#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <vector>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* adminUsername = "admin"; //username for login to the website
const char* adminPassword = "admin"; //password

WebServer server(80);

const int relayPin = 4; // Relay control pin
DHT dht(2, DHT11);

std::vector<String> loginLogs;

bool isAuthenticated = false;
float targetTemperature = 25.0;

// Timer for real-time updates
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 5000; // Update interval in milliseconds (5 seconds)

void handleRoot() {
  if (!isAuthenticated) {
    server.sendHeader("Location", "/login", true);
    server.send(303);
    return;
  }
  
  // Check if it's time to update temperature and humidity readings
  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();
    dht.readTemperature(); // Read temperature to trigger sensor update
  }
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Error handling for sensor readings
  if (isnan(temperature) || isnan(humidity)) {
    temperature = 0;
    humidity = 0;
  }
  
  String tempTitle = "Thermostat Control - " + String(temperature, 2) + "&deg;C"; // Display temperature with 2 decimal places
  
  String webpage = "<!DOCTYPE html><html><head><title>" + tempTitle + "</title>";
  webpage += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  webpage += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css'>";
  webpage += "<style>";
  webpage += "body { font-family: Arial, sans-serif; background-color: #f3f3f3; margin: 0; padding: 0; }";
  webpage += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background-color: #fff; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
  webpage += "h1 { color: #333; text-align: center; }";
  webpage += "form { text-align: center; }";
  webpage += "input[type='number'] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
  webpage += "input[type='submit'] { width: 100%; padding: 10px; margin-top: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }";
  webpage += ".version { text-align: center; margin-top: 20px; }";
  webpage += ".tips { margin-top: 20px; padding: 10px; background-color: #f9f9f9; border-radius: 5px; }"; // Style for energy efficiency tips section
  webpage += "</style>";
  webpage += "</head><body>";
  webpage += "<div class='container'>";
  webpage += "<h1><i class='fas fa-thermometer-half'></i> Thermostat Control</h1>";
  webpage += "<p>Current Temperature: " + String(temperature, 2) + "&deg;C</p>"; // Display temperature with 2 decimal places
  webpage += "<p>Current Humidity: " + String(humidity) + "%</p>";
  webpage += "<p>Target Temperature: " + String(targetTemperature) + "&deg;C</p>"; // Display target temperature with degree symbol
  webpage += "<form action='/setpoint' method='GET'>";
  webpage += "<input type='number' name='targetTemp' step='0.5' value='" + String(targetTemperature) + "' placeholder='Enter Target Temperature'>";
  webpage += "<br><input type='submit' value='Set Temperature'>";
  webpage += "</form>";
  webpage += "<div class='tips'>";
  webpage += "<h2>Energy Efficiency Tips</h2>";
  webpage += "<ul>";
  webpage += "<li>Keep doors and windows closed to prevent heat loss.</li>";
  webpage += "<li>Use curtains to block out drafts and retain heat.</li>";
  webpage += "<li>Lower your thermostat by 1&deg;C to save energy.</li>";
  webpage += "</ul>";
  webpage += "</div>";
  webpage += "<form action='/logout' method='GET'><input type='submit' value='Logout'></form>"; // Logout button
  webpage += "<p class='version'>Version 2.0.9</p>"; // Display version
  webpage += "</div>";
  webpage += "</body></html>";
  
  server.send(200, "text/html", webpage);
}

void handleLogin() {
  if (server.hasArg("username") && server.hasArg("password")) {
    String username = server.arg("username");
    String password = server.arg("password");
    
    if (username.equals(adminUsername) && password.equals(adminPassword)) {
      isAuthenticated = true;
      server.sendHeader("Location", "/", true);
      server.send(303);
      return;
    }
  }
  
  String loginPage = "<!DOCTYPE html><html><head><title>Login</title>";
  loginPage += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  loginPage += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css'>";
  loginPage += "<style>";
  loginPage += "body { font-family: Arial, sans-serif; background-color: #f3f3f3; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }";
  loginPage += ".container { max-width: 300px; padding: 20px; background-color: #fff; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
  loginPage += "h1 { color: #333; text-align: center; }";
  loginPage += "form { text-align: center; }";
  loginPage += "input[type='text'], input[type='password'] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
  loginPage += "input[type='submit'] { width: 100%; padding: 10px; margin-top: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }";
  loginPage += "</style></head><body>";
  loginPage += "<div class='container'>";
  loginPage += "<h1><i class='fas fa-lock'></i> Login</h1>";
  loginPage += "<p style='color: red;'>Wrong Password or Username</p>";
  loginPage += "<form action='/login' method='GET'>";
  loginPage += "<input type='text' name='username' placeholder='Username'><br>";
  loginPage += "<input type='password' name='password' placeholder='Password'><br>";
  loginPage += "<input type='submit' value='Login'>";
  loginPage += "</form>";
  loginPage += "</div>";
  loginPage += "</body></html>";
  
  server.send(200, "text/html", loginPage);
}

void handleLogout() {
  isAuthenticated = false;
  server.sendHeader("Location", "/login", true);
  server.send(303);
}

void handleSetPoint() {
  if (!isAuthenticated) {
    server.sendHeader("Location", "/login", true);
    server.send(303);
    return;
  }
  
  if (server.hasArg("targetTemp")) {
    targetTemperature = server.arg("targetTemp").toFloat();
    // Validate temperature range
    if (targetTemperature < 0 || targetTemperature > 100) {
      targetTemperature = 25.0; // Reset to default if out of range
    }
  }
  
  server.sendHeader("Location", "/", true);
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure relay is initially off
  
  dht.begin();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_GET, handleLogin);
  server.on("/logout", HTTP_GET, handleLogout);
  server.on("/setpoint", HTTP_GET, handleSetPoint);
  server.begin();
}

void loop() {
  server.handleClient();
}
