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
Den Code findet man anbei im script Folder unter script.ino bzw. index.h für die Website.

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

### Bilder und Schaltungen
<image src="Bilder/schaltbild.jpg">
Wir haben leider keinen Vibrationssensor gefunden, weswegen wir im Schaltbild einen DS18B20 stattdessen eingebaut haben, da er die gleichen Pins wie unser Vibrationssensor besitzt. 
