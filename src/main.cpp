#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "SSID";
const char* password = "PSK";
const char* api_key = "API";
const char* Gemini_Max_Tokens = "1000"; //more the tokens long the answer gets
String res = "";

const char* persona_message = "You are Archimedes Q. Beak, a wise and kind owl who loves helping children understand the world. "
                              "You always explain things clearly, using simple words, fun examples, and an encouraging tone. "
                              "You never talk down to kids. You're cheerful, patient, and love sharing cool facts!"
                              "Keep answers to around 100 words unless the question requires a long explanation, this will help in their understanding";
                  
const char* base_url = "https://generativelanguage.googleapis.com/v1/models/gemini-2.0-flash-001:generateContent?key=";

void askGemini(String user_input) {
  HTTPClient https;
  WiFiClientSecure client;
  client.setInsecure();  // NOTE: For development only

  if (https.begin(client, String(base_url) + api_key)) {
    https.addHeader("Content-Type", "application/json");

    String payload = String("{\"contents\": [") +
      "{\"role\":\"user\",\"parts\":[{\"text\":\"" + String(persona_message) + "\"}]}," +
      "{\"role\":\"user\",\"parts\":[{\"text\":\"" + user_input + "\"}]}]," +
      "\"generationConfig\": {\"maxOutputTokens\": " + (String)Gemini_Max_Tokens + "}}";

    int httpCode = https.POST(payload);
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String response = https.getString();

      DynamicJsonDocument doc(2048);
      deserializeJson(doc, response);
      String Answer = doc["candidates"][0]["content"]["parts"][0]["text"];

      Answer.trim();
      Serial.println("\n" + Answer);
    } else {
      Serial.printf("[HTTPS] POST failed: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Connection failed\n");
  }
}

void setup() {
  Serial.begin(9600); //serial startup
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  while (!Serial);
  // wait for WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to "); 
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address: "); //esp's ip address
  Serial.println(WiFi.localIP());
  askGemini("Please introduce yourself to the children. Be friendly and fun!");
}

void loop() {
  Serial.println("\nAsk your Question:");
  while (!Serial.available());
  
  String input = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\r' && c != '\n') input += c;
    delay(1);
  }

  if (input.length() > 0) {
    askGemini(input);
  }

  res = "";
}