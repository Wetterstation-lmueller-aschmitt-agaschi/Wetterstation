# Projekt Wetterstation
Verfasser: **Luis Müller, Alexander Schmitt, Amir Gashi**  
Datum: **5.6.2025** 

## 1.  Einführung

Es sollte eine Wetterstation erstellt werden, die mithilfe eines DHT11 Sensors Temperatur und Luftfeuchtigkeit misst. Außerdem sollten die gemessenen Daten auf einer Website angezeigt werden. Die
Hier wird in einem Absatz eine kurze (zwei bis drei Sätze) Einführung zum Thema geschrieben. Zum Beispiel kann hier stehen, warum dieses Thema gewählt wurde, warum es wichtig ist. Grundsätzlich ist eine Dokumentation im passiv zu verfassen. Es ist nicht wichtig wer etwas getan hat, sondern was getan wurde.

Z.B.:

Es gibt unterschiedliche Methoden um Daten zu speichern, zum Beispiel in Dateien auf einer Festplatte oder in einem Cloudstorage. Für Webanwendungen oder maschinelle Weiterverarbeitung eignet sich dies eher schlecht, dafür werden zu meist Datenbanken verwendet. In Informationssysteme der 3. Klasse wird sich vor allem mit den relationalen Datenbanken beschäftigt.

## 2.  Projektbeschreibung

Es wurde eine kleine Wetterstation mit hilfe eine ESP32c3, einem DHT11 und Vibrationssensor realisiert. Die gemessenen Daten werden über eine vom Microcontroller gehostete Website dargestellt. Außerdem wurde eine RGB-Status-LED mit drei verschiedenen Zusänden realisiert, wobei man über die Website die einzelnen Farben der Zustände einstellen kann. 

## 3.  Theorie
Bei diesem Projekt wurde viel Theorie aus dem letzten Jahr Syt-Unterricht in der Praxis angewant. 


### Code

Sehr wichtig ist es den verwendeten Code zu dokumentieren.

```c++
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
    Serial.println("ESP32 Web Server: New request received:");  // for debugging
    Serial.println("GET /");                                    // for debugging

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
```
Anfangs haben wir sehr viele Objekte und variablen definiert bzw deklariert, die wir im Laufe des Codes benötigen, wie z.B. für den DHT Sensor, AsyncWebServer, oder eine Variable zum speichern des aktuellen Status der Status-LED. Auch wurde eine Klasse für Messung erstellt. Dabei hat es jeweils ein Attribut für die Zeit, Temperatur, Vibration und Luftfeuchtigkeit. Danach wurde ein Messungs-Objekt-Array erstellt, welches drei Messungen speichert. Dadurch ermitteln wir dann den Durchschnitt der Messungen und geben diese dann über den WebServer an Clients weiter. 


### Setup
Im Setup initialisieren wir erstmal pixels für die RGB-LED, Serial auf einer BAUD von 9600 und setzten unseren PIN zum lesen des Vibrationssensors auf INPUT_PULLUP.
Dann initialisieren wir nvs zum auslesen der Lokal auf dem Flash gespeicherten Daten der Status-LED und laden danach die gespeicherten Farben. Anschließend setzten wir den Status auf 1 bzw. auf offline.

Danach verbindet sich der ESP mit dem WLAN und setzt bei Erfolg den Status der LED auf online. Anschließend wird der Webserver aufgesetzt. Dabei haben wir ein eigenes File geschrieben und eingebunden, in welchem einerseits ein 2D-Array zum speichern der Farben und andererseits ein const char* mit der Website erstellt wird. Dieses File haben wir dann oben im Code mithilfe von ``` #include index.h``` eingebunden. Wenn nun eine Request gesendet wird, wird dieser char zurückgeschickt.
Danach sorgen wir noch dafür, dass mit mit den Spezifikationen /time, /temperature, etc... jeweils die Temperatur, Zeit, Luftfeuchtigkeit, etc... zurückbekommt. Auch haben wir einen Befehl /setLED zum setzen der Farbe der StatusLED erstellt. Dabei benutzen wir die Argumente in der URL um die RGB-Werte zu bekommen und die Id, sprich welcher Status verändert werden soll.

Danach wird der Server mit server.begin(); gestartet.

### Loop
Im loop() benutzen wir millis() und eine Variable um alle 5 Sekunden eine neue Messung zu erstellen. Vor der Messung wird der Status auf 2 bzw. auf messen gestellt. Dann wird durch eine Schleife die älteste Messung im Array gesucht und durch die neue Ersetzt. Nach der Messung wird der Status der LED wieder auf online gesetzt.

### setStatus
Bei SetStatus setzten wir die Farbe der LED mithilfe des color-arrays auf den angegebenen Status.

### getTemperature und getHumidity
Bei getTemperature nehmen wir alle Sinnvollen Messwerte aus dem Array (zwischen 0 und 50°) und dividieren dann durch die Anzahl der Sinnvollen Messwerte um den Durchschnitt zu erhalten. Bei getHumidity wird das gleiche nur mit der Luftfeuchtigkeit gemacht.


### loadColors
Bei loadColors laden wir die Farben aus dem Flash speicher mithilfe der load-value funktion. Wir haben dabei 9 key-value Paare, die mithilfe einer For-schleife ausgelesen werden.

### saveColors
Bei saveColors werden die aktuelle color-array in 9 keys gespeichert.

### save_value und load_value
Diese beiden Methoden speichern oder laden einen Datensatz mithilfe eines keys aus der Flash-Speicher des ESPs.

```c++
#ifndef WEBPAGE_H
#define WEBPAGE_H

/* 
 * [0] -> online
 * [1] -> no WLAN/Offline
 * [2] -> taking measurements
 */
short color[4][3] = {{0, 150, 0}, {150, 0, 0}, {150, 100, 0}, {0, 0, 0}}; 

const char* webpage = R"=====(
<!DOCTYPE html>
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <title>ESP32 Wetterstation</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.6/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    <style>
        :root {
            --primary-color: #2c3e50;
            --secondary-color: #34495e;
            --accent-1: #3498db;
            --accent-2: #2ecc71;
            --accent-3: #e74c3c;
            --accent-4: #f1c40f;
            --text-light: #ecf0f1;
        }

        body {
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            min-height: 100vh;
        }

        .sensor-card {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            transition: all 0.3s ease;
        }

        .sensor-card:hover {
            transform: translateY(-5px) scale(1.02);
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.2);
        }

        .value-badge {
            font-size: 1.8rem;
            padding: 0.8rem 1.2rem;
            border-radius: 12px;
            background: rgba(255, 255, 255, 0.15);
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
            transition: all 0.3s ease;
        }

        .control-panel {
            background: rgba(255, 255, 255, 0.05);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        .led-card {
            background: rgba(0, 0, 0, 0.2);
            border-radius: 15px;
            transition: all 0.3s ease;
            padding: 1.5rem;
        }

        .led-card:hover {
            transform: translateY(-3px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }

        .custom-btn {
            background: linear-gradient(135deg, var(--accent-1), var(--accent-2));
            border: none;
            border-radius: 10px;
            padding: 12px 25px;
            transition: all 0.3s ease;
        }

        .custom-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }

        .header-title {
            color: var(--text-light);
            font-weight: bold;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.2);
        }

        .color-picker {
            width: 100% !important;
            height: 60px !important;
            padding: 5px;
            border-radius: 10px;
            border: 2px solid rgba(255, 255, 255, 0.1);
        }

        .custom-btn-muted {
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.15);
            border-radius: 10px;
            padding: 10px 20px;
            font-size: 1rem;
            color: var(--text-light);
            transition: all 0.3s ease;
        }

        .custom-btn-muted:hover {
            background: rgba(255, 255, 255, 0.15);
            transform: translateY(-1px);
            box-shadow: 0 3px 8px rgba(0, 0, 0, 0.1);
        }

    </style>
</head>
<body>

<div class="container py-5">
    <!-- Header -->
    <div class="text-center mb-5">
        <h1 class="display-3 header-title mb-3">🌡️ ESP32 Wetterstation</h1>
        <p class="text-light opacity-75">Letzte Aktualisierung: <span id="time1" class="fw-bold">Loading...</span></p>
    </div>

    <!-- Sensor Dashboard -->
    <div class="row g-4 mb-5">
        <!-- Temperatur -->
        <div class="col-md-6 col-lg-3">
            <div class="sensor-card h-100 p-3">
                <div class="text-center">
                    <i class="fas fa-temperature-high fa-2x mb-3" style="color: #e74c3c;"></i>
                    <h5 class="text-light mb-4">Temperatur</h5>
                    <div class="value-badge">
                        <span id="temperature" class="text-light">Loading...</span> °C
                    </div>
                </div>
            </div>
        </div>

        <!-- Luftfeuchtigkeit -->
        <div class="col-md-6 col-lg-3">
            <div class="sensor-card h-100 p-3">
                <div class="text-center">
                    <i class="fas fa-tint fa-2x mb-3" style="color: #3498db;"></i>
                    <h5 class="text-light mb-4">Luftfeuchtigkeit</h5>
                    <div class="value-badge">
                        <span id="humidity" class="text-light">Loading...</span>%
                    </div>
                </div>
            </div>
        </div>

        <!-- Laufzeit -->
        <div class="col-md-6 col-lg-3">
            <div class="sensor-card h-100 p-3">
                <div class="text-center">
                    <i class="fas fa-clock fa-2x mb-3" style="color: #2ecc71;"></i>
                    <h5 class="text-light mb-4">Laufzeit</h5>
                    <div class="value-badge">
                        <span id="uptime" class="text-light">Loading...</span> Sekunden
                    </div>
                </div>
            </div>
        </div>

        <!-- Vibrationen -->
        <div class="col-md-6 col-lg-3">
            <div class="sensor-card h-100 p-3">
                <div class="text-center">
                    <i class="fas fa-wave-square fa-2x mb-3" style="color: #f1c40f;"></i>
                    <h5 class="text-light mb-4">Vibrationen</h5>
                    <div class="value-badge">
                        <span id="vibration" class="text-light">Loading...</span>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <!-- LED Control Panel -->
    <div class="control-panel p-4 mt-4">
        <h2 class="text-light mb-4"><i class="fas fa-lightbulb"></i> LED-Steuerung</h2>
        <div class="row g-4">
            <div class="col-md-4">
                <div class="led-card">
                    <h5 class="text-light mb-3">Status ON</h5>
                    <input type="color" class="color-picker" id="on" value="#ff0000">
                </div>
            </div>
            <div class="col-md-4">
                <div class="led-card">
                    <h5 class="text-light mb-3">Kein WLAN</h5>
                    <input type="color" class="color-picker" id="off" value="#ff0000">
                </div>
            </div>
            <div class="col-md-4">
                <div class="led-card">
                    <h5 class="text-light mb-3">Messung aktiv</h5>
                    <input type="color" class="color-picker" id="messen" value="#ff0000">
                </div>
            </div>
            <div class="col-12 text-end mt-4">
                <button type="button" class="custom-btn-muted text-light" onclick="submitClicked()">
                    <i class="fas fa-save me-2"></i>Einstellungen anwenden
                </button>
            </div>
            
        
        
            <div class="col-12 text-end mt-4">
                <button type="button" class="custom-btn-muted text-light" onclick="ledOff()" id="offLED">
                    LED ausschalten
                </button>
            </div>
        
        

        </div>
    </div>
</div>

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.6/dist/js/bootstrap.bundle.min.js"></script>
<script>
    const on = document.getElementById("on");
    const off = document.getElementById("off");
    const messen = document.getElementById("messen");

    function fetchTemperature() {
        fetch("/temperature")
            .then(response => response.text())
            .then(data => {
                if (!isNaN(data)) {
                    document.getElementById("temperature").textContent = data;
                }
            });
    }

    function fetchUptime() {
        fetch("/uptime")
            .then(response => response.text())
            .then(data => {
                if (!isNaN(data)) {
                    document.getElementById("uptime").textContent = data;
                }
            });
    }

    function fetchHumidity() {
        fetch("/humidity")
            .then(response => response.text())
            .then(data => {
                if (!isNaN(data)) {
                    document.getElementById("humidity").textContent = data;
                }
            });
    }

    function fetchVibration() {
        fetch("/vibration")
            .then(response => response.text())
            .then(data => {
                if (!isNaN(data)) {
                    document.getElementById("vibration").textContent = Boolean(parseInt(data));
                }
            });
    }

    function fetchTime() {
        fetch("/time")
            .then(response => {
                if (!response.ok) throw new Error("404 Not Found");
                return response.text();
            })
            .then(data => {
                document.getElementById("time1").textContent = data;
            })
            .catch(() => {
                document.getElementById("time1").textContent = "404 Not Found";
            });
    }


    function submitClicked() {
        sendColor(on.value, 0);
        sendColor(off.value, 1);
        sendColor(messen.value, 2);
    }

    function ledOff() {
        fetch('/ledoff');
        const s = document.getElementById("offLED");
        if(s.innerHTML == "LED: AN"){
          s.innerHTML = "LED: AUS";
        }else{
          s.innerHTML = "LED: AN";
        }
    }

    function sendColor(hex, id) {
        const r = parseInt(hex.slice(1, 3), 16);
        const g = parseInt(hex.slice(3, 5), 16);
        const b = parseInt(hex.slice(5, 7), 16);
        let url = `/setLED?id=${id}&r=${r}&b=${b}&g=${g}`;
        fetch(url);
    }

    // Initial fetch and update every 4 seconds
    fetchTemperature();
    fetchUptime();
    fetchHumidity();
    fetchVibration();
    fetchTime();

    setInterval(fetchTemperature, 4000);
    setInterval(fetchUptime, 4000);
    setInterval(fetchHumidity, 4000);
    setInterval(fetchVibration, 4000);
    setInterval(fetchTime, 4000);
</script>

</body>
</html>
)=====";

#endif
```


### Bilder und Schaltungen
<image src="Bilder/schaltbild.jpg">
Wir haben leider keinen Vibrationssensor gefunden, weswegen wir im Schaltbild einen DS18B20 stattdessen eingebaut haben, da er die gleichen Pins wie unser Vibrationssensor besitzt. 