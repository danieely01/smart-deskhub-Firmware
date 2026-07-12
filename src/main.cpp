#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "secrets.h"


const char* SERVER_URL = "http://" SERVER_IP ":8000/set-mode";

// == GOMBOK INICIALIZALASA
#define BTN_WORK D6
#define BTN_STUDY D7
#define BTN_GAME D3


// === OLED KIJELZŐ BEÁLLÍTÁSOK ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //A kijelzo peldanyositasa objektumkent
                                                                  // -1 azert kell, mert nincs fizikai reset pin az alkatreszen
// === DHT HŐMÉRSÉKLETSZENZOR===
#define DHTPIN D5 //Definialjuk, hogy melyik pinre van kotve a szenzor
#define DHTTYPE DHT22 //Definialjuk a szenzor tipusat (DHT11, DHT22, AM2302) stbb
DHT dht(DHTPIN, DHTTYPE); //A szenzor peldanyositasa objektumkent

//idozites a szenzor olvasasok kozott, nem delay!!! hanem millis, mert a delay blokkolja a tovabbi kodot, es nem tudunk kozben mas dolgokat csinalni
unsigned long lastMillis = 0;
const long interval = 5000; // 5 mp

float temperature = 0.0;
float humidity = 0.0;
String currentMode = "READY"; // a currentmode valtozoban lesz majd kesobb az aktualis mod, ami alapjan led szint valtoztatunk, egyenlore ez meg csak pelda

//Mod valtas
void setBackendMode(String mode) {
  WiFiClient client;
  HTTPClient http;

  String url = "https://" + String(SERVER_IP) + ":8000/set-mode";

  Serial.print("HTTP Post kérés küldése a backendnek: ");
  Serial.println(mode);

  if(http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    
    //Json csomag osszeallitasa
    String payload = "{\"mode\": \"" + mode + "\"}";
    int httpCode = http.POST(payload);

    if(httpCode > 0 ) {
      Serial.println("HTTP POST sikeres, kód: " + String(httpCode));
      currentMode = mode; //Ha sikeres a POST, akkor frissitjuk a currentMode valtozot
    } else {
      Serial.println("HTTP POST hiba: " + String(httpCode));
    }
    http.end();


  }
  else {
    Serial.println("HTTP POST hiba: Nem sikerült csatlakozni a backendhez.");
  }
}



// Kijelző frissítő függvény
void updateDisplay() {
  display.clearDisplay(); //Toroljuk a kijelzot, hogy ne legyenek maradvanyok a korabbi kiiratasokbol
  display.setTextColor(SSD1306_WHITE); 
  
  // A mod kiirasa, fent
  display.setTextSize(1);
  display.setCursor(0, 0); // A kurzor poziciojanak beallitasa a kiirashoz, vagyis bal felso sarok
  display.print("MOD: ");
  display.println(currentMode);
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE); // egy <hr> vonal a mod es a tobbi adat kozott
  
  // Homerseklet kiirasa
  display.setTextSize(2);
  display.setCursor(0, 14);
  if (isnan(temperature)) { //Ha a homerseklet merheto, vagyis a tipust jol adtuk meg, akkor kiirjuk
    display.print("ERR");
  } else {
    display.print(temperature, 1); // 1 tizedesjegy pontossaggal irjuk ki a homersekletet
    display.print("C");
  }
  
  //Paratartalom
  display.setCursor(76, 14);
  if (isnan(humidity)) {
    display.print("ERR");
  } else {
    display.print(humidity, 0);
    display.print("%");
  }
  
  display.display(); // A kijelzo frissitese, hogy a fenti kiiratasok megjelenjenek!!
}
void setup() { //A program indulasakor egyszer fut csak le


  Serial.begin(115200); // Soros port inditasa, hogy a debug uzeneteket ki tudjuk irni a soros monitorra
  Serial.println("\n\n=== [AI Smart Desk Hub] 1. Fázis: Szenzorok & OLED ==="); //Hogy tudjuk hogy elindult a soros monitorozas

  //Gombok bemenetkent valo beallitasa, hogy tudjuk olvasni a gombnyomasokat
  pinMode(BTN_WORK, INPUT_PULLUP);
  pinMode(BTN_STUDY, INPUT_PULLUP);
  pinMode(BTN_GAME, INPUT_PULLUP);




  // DHT szenzor inditasa
  dht.begin();

  //Oled kijelzo inditasa
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //Ha a kijelzo nem talalhato, akkor irunk egy hibauzenetet a soros monitorra
    Serial.println("--> [Hiba] Nem találom az OLED kijelzőt! Ellenőrizd az SCL/SDA bekötést!");
  } else {
    Serial.println("--> [OLED] Kijelző sikeresen elindítva.");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 25);
    display.println("Booting Desk Hub...");
    display.display();
  }

  //Wifi csatlakozas
  WiFi.mode(WIFI_STA); 
  
  // ---> EZT A SORT ADD HOZZÁ, HOGY LÁSSUK MIT KERES: <---
  Serial.printf("--> [Debug] Próbálkozás ezzel az SSID-vel: '%s'\n", WIFI_SSID); //debug 
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("--> [Wi-Fi] Csatlakozás");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n--> [Wi-Fi] Sikeres csatlakozas!");
  Serial.print("--> [ESP8266 IP]: ");
  Serial.println(WiFi.localIP()); 

  updateDisplay(); //Kijelzo frissites meghivasa, hogy a bootolasi uzenet eltunjon, es a mod/homerseklet/paratartalom adatok jelenjenek meg
}

void loop() {
  unsigned long currentMillis = millis(); //A millis() fuggveny visszaadja, hogy az ESP8266 mennyi ideje fut, ezert hasznaljuk a szenzor olvasasok kozotti idoziteshez
  
  if (currentMillis - lastMillis >= interval) { //Ha eltelt az interval ideje, akkor olvassuk le a szenzor adatokat
    lastMillis = currentMillis;
    
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    
    if (isnan(newTemp) || isnan(newHum)) {
      Serial.println("--> [Hiba] A DHT szenzor olvasása sikertelen! (Vagy rossz típus van megadva)");
    } else {
      temperature = newTemp;
      humidity = newHum;
      Serial.print("--> [Szenzor] Hőmérséklet: ");
      Serial.print(temperature);
      Serial.print(" °C | Páratartalom: ");
      Serial.print(humidity);
      Serial.println(" %");
    }
    
  }

    //Gombnyomas ellenorzes, es backendhez kuldes, ha valamelyik gombot megnyomtak
    if(digitalRead(BTN_WORK) == LOW) {
        setBackendMode("working"); //Backend fuggveny meghivasa a "working" mod beallitasaert
        updateDisplay(); //Kijelzo frissitese, hogy az uj mod megjelenjen
        delay(500); //Debounce, hogy ne legyen tobb POST kuldes egy gombnyomasra
    }

    if(digitalRead(BTN_STUDY) == LOW) {
      setBackendMode("studying");
      updateDisplay();
      delay(500);
    }

    if(digitalRead(BTN_GAME) == LOW) {
      setBackendMode("relaxing");
      updateDisplay();
      delay(500);
    }


    // Kijelző frissítése az új adatokkal
    updateDisplay();
}