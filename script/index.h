

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
