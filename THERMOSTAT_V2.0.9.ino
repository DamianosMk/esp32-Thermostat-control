#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <vector>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* adminUsername = "admin"; //username for login to the website
const char* adminPassword = "admin"; //password

WebServer server(80);

const int relayPin = 15; // Relay control pin
DHT dht(2, DHT11);

std::vector<String> loginLogs;

bool isAuthenticated = false;
float targetTemperature = 25.0;

void handleRoot() {
  if (!isAuthenticated) {
    server.sendHeader("Location", "/login", true);
    server.send(303);
    return;
  }
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  String webpage = "<!DOCTYPE html><html><head><title>Thermostat Control</title>";
  webpage += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  webpage += "<style>";
  webpage += "body { font-family: Arial, sans-serif; background-color: #f3f3f3; }";
  webpage += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background-color: #fff; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
  webpage += "h1 { color: #333; text-align: center; }";
  webpage += "form { text-align: center; }";
  webpage += "input[type='number'] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
  webpage += "input[type='submit'] { width: 100%; padding: 10px; margin-top: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }";
  webpage += "footer { text-align: center; margin-top: 20px; }";
  webpage += "</style></head><body>";
  webpage += "<div class='container'>";
  webpage += "<h1>Thermostat Control</h1>";
  webpage += "<p>Current Temperature: " + String(temperature) + "&deg;C</p>";
  webpage += "<p>Current Humidity: " + String(humidity) + "%</p>";
  webpage += "<p>Target Temperature: " + String(targetTemperature) + "&deg;C</p>";
  webpage += "<form action='/setpoint' method='GET'>";
  webpage += "<input type='number' name='targetTemp' step='0.5' value='" + String(targetTemperature) + "' placeholder='Enter Target Temperature'>";
  webpage += "<br><input type='submit' value='Set Temperature'>";
  webpage += "</form>";
  webpage += "<footer><form action='/logout' method='GET'><input type='submit' value='Logout'></form></footer>";
  webpage += "</div></body></html>";
  
  server.send(200, "text/html", webpage);
}

void handleLoginPage() {
  String errorMessage = server.arg("error") == "true" ? "<p style='color: red;'>Invalid username or password</p>" : "";
  String loginPage = "<!DOCTYPE html><html><head><title>Login</title>";
  loginPage += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  loginPage += "<style>";
  loginPage += "body { font-family: Arial, sans-serif; background-color: #f3f3f3; }";
  loginPage += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background-color: #fff; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
  loginPage += "h1 { color: #333; text-align: center; }";
  loginPage += "form { text-align: center; }";
  loginPage += "input[type='text'], input[type='password'] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; }";
  loginPage += "input[type='submit'] { width: 100%; padding: 10px; margin-top: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }";
  loginPage += "</style></head><body>";
  loginPage += "<div class='container'>";
  loginPage += "<h1>Login</h1>";
  loginPage += errorMessage;
  loginPage += "<form action='/login' method='POST'>";
  loginPage += "<input type='text' name='username' placeholder='Username'><br>";
  loginPage += "<input type='password' name='password' placeholder='Password'><br>";
  loginPage += "<input type='submit' value='Login'>";
  loginPage += "</form>";
  loginPage += "</div></body></html>";

  server.send(200, "text/html", loginPage);
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
  
  server.sendHeader("Location", "/login?error=true", true);
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
  }
  
  server.sendHeader("Location", "/", true);
  server.send(303);
}

void setup() {
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
  server.on("/login", HTTP_GET, handleLoginPage);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/setpoint", HTTP_GET, handleSetPoint);
  server.begin();
}

void loop() {
  server.handleClient();
  
  float temperature = dht.readTemperature();
  if (temperature >= targetTemperature + 0.5) {
    // Turn off the relay
    digitalWrite(relayPin, LOW);
  } else if (temperature <= targetTemperature - 0.5) {
    // Turn on the relay
    digitalWrite(relayPin, HIGH);
  }
}
