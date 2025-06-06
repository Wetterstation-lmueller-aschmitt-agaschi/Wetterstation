#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "index.h"  

#include "nvs.h"

#include "DHT.h"
#include "DHT_U.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include <Adafruit_NeoPixel.h>


#define PIN         8
#define NUMPIXELS   1

#include <time.h>

// Zeitzone (z. B. Österreich: GMT+1 → 3600 Sekunden, im Sommer GMT+2 → 7200 Sekunden)
const long gmtOffset_sec = 3600;        // Standardzeit
const int daylightOffset_sec = 3600;    // Sommerzeit


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "IOT"; // CHANGE IT
const char* password = "20tgmiot18"; // CHANGE IT

AsyncWebServer server(80);

DHT dht(2, DHT11);
int inputPin = 0;

int status = 0;
bool changeStatus = true;
bool ledON = true;

class Messung{
  public: 
    int time;
    float temp;
    float humidity;
    bool vibration;
    Messung(){
      //setStatus(2);
      time = millis()/1000;
      temp = dht.readTemperature();
      humidity = dht.readHumidity();
      vibration = false;
      //setStatus(0);
    }
};

Messung m[3] = {{}, {}, {}};
int await;

void setup() {

  pixels.begin();
  Serial.begin(9600);

  pinMode(inputPin, INPUT_PULLUP);
 

  //Status LED initialisieren und Farben aus speicher holen
  init_nvs(); 
  loadColors();
  setStatus(1);
  
  //Server initialisieren
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  setStatus(0);

  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:"); 
    Serial.println("GET /");                                   

    request->send(200, "text/html", webpage);
  });

  Serial.println(dht.readTemperature());

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  Serial.println("Warte auf Zeitdaten...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      Serial.println("Fehler beim Abrufen der Zeit");
      delay(1000);
    }

    Serial.println("Zeit erfolgreich abgerufen!");


  server.on("/time", HTTP_GET, [](AsyncWebServerRequest* request) {
      struct tm timeinfo;

      if (!getLocalTime(&timeinfo)) {
          request->send(500, "text/plain", "Zeit konnte nicht geladen werden");
          return;
      }

      char timeString[64];
      strftime(timeString, sizeof(timeString), "Datum: %d.%m.%Y  Zeit: %H:%M:%S", &timeinfo);

      request->send(200, "text/plain", timeString);
  });
  

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    float temperature = getTemperature();
    String temperatureStr = String(temperature, 2);
    request->send(200, "text/plain", temperatureStr);
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    float humidity = getHumidity();
    String humidityStr = String(humidity, 2);
    request->send(200, "text/plain", humidityStr);
  });

  server.on("/vibration", HTTP_GET, [](AsyncWebServerRequest* request) {
    setStatus(2);
    bool v = digitalRead(inputPin);
    String vStr = String(v);
    request->send(200, "text/plain", vStr);
    setStatus(0);
  });

  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest* request) {
    float time = millis()/1000;
    String uptime = String(time, 2);
    request->send(200, "text/plain", uptime);
  });

    server.on("/ledoff", HTTP_GET, [](AsyncWebServerRequest* request) {
      if(ledON){
        ledON = false;
        setStatus(0);
      } 
      else {
        ledON = true;
        //digitalWrite(LED_BUILTIN, HIGH);
        setStatus(0);
      }
      Serial.println(ledON);
      request->send(200, "text/plain", "LED changed");
  });

  server.on("/setLED", HTTP_GET, [](AsyncWebServerRequest *request){
  if (request->hasParam("id")) {
    String sid = request->getParam("id")->value();
    String r = request->getParam("r")->value();
    int id = sid.toInt();
    String g = request->getParam("g")->value();
    String b = request->getParam("b")->value();
    
    
    color[id][0] = r.toInt();
    color[id][1] = g.toInt();
    color[id][2] = b.toInt();
    saveColors();
    setStatus(0);
    request->send(200, "text/plain", "Received: ");
  }
  });
  // Start the server
  server.begin();
}

void loop() {
  
  if(millis() > await){
    setStatus(2);
    int id = 0;
    for(int i = 0; i < 3; i++){
      if(m[i].time < m[id].time) id = i;
    }
    m[id].time = millis();
    m[id].temp = dht.readTemperature();
    m[id].humidity = dht.readHumidity();
    m[id].vibration = digitalRead(inputPin);
    await = millis()+5000;
    setStatus(0);
  }

}

void setStatus(int id){
  if(ledON == false) {
    Serial.println("!!!");
    pixels.setPixelColor(0, pixels.Color(color[3][0], color[3][1], color[3][2]));
    pixels.show();
    return;
  }
  pixels.setPixelColor(0, pixels.Color(color[id][0], color[id][1], color[id][2])); 
  pixels.show();
}

float getTemperature(){
  float temp = 0;
  int l = 0;
  for(int i = 0; i < 3; i++){
    if(m[i].temp < 50 && m[i].temp > 0){
      temp += m[i].temp;
      l++;
    }
  }
  temp /= l;
  return temp;
}

float getHumidity(){
  float h = 0;
  int l = 0;
  for(int i = 0; i < 3; i++){
    if(m[i].humidity < 100 && m[i].humidity > 0){
      h += m[i].humidity;
      l++;
    }
  }
  h /= l;
  return h;
}

void saveColors(){
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < 3; j++){
      save_value((String(i) + "_" + String(j)).c_str(), color[i][j]);
    }
  }
}

void loadColors(){
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < 3; j++){
      color[i][j] = load_value((String(i) + "_" + String(j)).c_str());
    }
  }
}

void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void save_value(const char* key, int32_t value) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_i32(my_handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    //printf("Save Wert = %d\n", value);
}


int32_t load_value(const char* key) {
    nvs_handle_t my_handle;
    int32_t value = 0;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &my_handle));
    esp_err_t err = nvs_get_i32(my_handle, key, &value);
    switch (err) {
        case ESP_OK:
            printf("Wert = %d\n", value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("Schlüssel nicht gefunden\n");
            break;
        default:
            printf("Fehler: %s\n", esp_err_to_name(err));
    }
    nvs_close(my_handle);
    return value;
}
