#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#ifndef WIFI_SSID
  #define WIFI_SSID "ALAP_NEV"
  #define WIFI_PASS "ALAP_JELSZO"
#endif
#ifndef SERVER_IP
  #define SERVER_IP "192.168.0.17" // Alapertelmezett IP cim a FastAPI szerverhez
#endif

const char* SERVER_URL = "http://" SERVER_IP ":8000/set-mode";

void setup() {
  // Soros kommunikáció indítása a PC felé (Logoláshoz)
  Serial.begin(115200);
  Serial.println("\n\n=== [AI Smart Desk Hub] ESP8266 Boot ===");
  
  // Wi-Fi csatlakozás
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("--> [Wi-Fi] Csatlakozás");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n--> [Wi-Fi] Sikeresen csatlakozva!");
  Serial.print("--> [ESP8266 IP Cím]: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Ha él a Wi-Fi kapcsolat, küldünk egy teszt módváltást a FastAPI szerverre
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("\n[HTTP] Kérés elküldése a backend felé: working mód...");
    
    // Kapcsolat felépítése
    http.begin(client, SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    
    // JSON payload küldése
    int httpResponseCode = http.POST("{\"mode\":\"working\"}");
    
    if (httpResponseCode > 0) {
      Serial.printf("--> [HTTP Sikeres] Válaszkód: %d\n", httpResponseCode);
      String response = http.getString();
      Serial.println("--> [Python Szerver Válasza]: " + response);
    } else {
      Serial.printf("--> [HTTP Hiba] A kérés sikertelen, hibakód: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    
    // Erőforrások felszabadítása
    http.end();
  } else {
    Serial.println("--> [Hiba] Megszakadt a Wi-Fi kapcsolat!");
  }

  // 10 másodperc szünet a következő automatikus tesztig
  delay(10000); 
}