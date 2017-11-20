#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "SSD1306.h"

// pins
#define SDA 14
#define SCL 12
#define I2C 0x3C

// create display
SSD1306 display(I2C, SDA, SCL);

// wifi settings
const char* host     = "api.coindesk.com";
const char* ssid     = "<<WIFI_NETWORK>>";
const char* password = "<<WIFI_PASSWORD>>";

ESP8266WebServer server(80);

void handle_root() {
  server.send(200, "text/plain", "Hello from the esp8266");
  delay(100);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  

  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(8, 20, "Updating ...");
  display.display();

  // Connect to API
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/v1/bpi/currentprice.json";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);
  
  // Read all the lines of the reply from server and print them to Serial
  String answer;
  while(client.connected()){
    String line = client.readStringUntil('\r');
    answer += line;
  }

  client.stop();
  Serial.println();
  Serial.println("closing connection");

  // Process answer
  Serial.println();
  Serial.println("Answer: ");
  Serial.println(answer);

  // Convert to JSON
  String jsonAnswer;
  int jsonIndex;

  for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
      jsonIndex = i;
      break;
    }
  }

  // Get JSON data
  jsonAnswer = answer.substring(jsonIndex);
  Serial.println();
  Serial.println("JSON answer: ");
  Serial.println(jsonAnswer);
  jsonAnswer.trim();

  // Get rate as float
  int rateIndex = jsonAnswer.indexOf("rate_float");
  String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 18);
  priceString.trim();
  float price = priceString.toFloat();

  // Print price
  Serial.println();
  Serial.println("Bitcoin price: ");
  Serial.println(price);

  // Display on OLED
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(32, 20, priceString);
  display.display();

  // Wait 30 seconds
  delay(30000);
}

