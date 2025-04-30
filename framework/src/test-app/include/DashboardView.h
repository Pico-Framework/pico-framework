#pragma once
#include "framework/HtmlTemplateView.h"

class DashboardView : public HtmlTemplateView {
public:
    DashboardView();

private:
    static constexpr const char* dashboard_html = R"!(
    <!DOCTYPE html>
    <html lang="en">
    
    <head>
        <meta charset="UTF-8">
        <title>Pico GPIO Dashboard</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            body {
                font-family: 'Raleway', sans-serif;
                background: #e0e0e0;
                margin: 0;
                padding: 2rem 1rem;
                display: flex;
                flex-direction: column;
                align-items: center;
            }
    
            h1 {
                color: #333;
                margin-bottom: 1.5rem;
            }
    
            .gpio-banks {
                display: flex;
                flex-wrap: wrap;
                justify-content: center;
                gap: 2rem;
                margin-bottom: 2.5rem;
                width: 100%;
                max-width: 900px;
            }
    
            .bank {
                display: grid;
                grid-template-columns: repeat(2, 1fr);
                gap: 1rem;
                background: #e0e0e0;
                padding: 1rem;
                border-radius: 20px;
                box-shadow: 9px 9px 16px #bebebe,
                    -9px -9px 16px #ffffff;
                width: 220px;
            }
    
            .gpio-card {
                background: #e0e0e0;
                border-radius: 16px;
                padding: 1rem;
                text-align: center;
                box-shadow: inset 2px 2px 6px #bebebe,
                    inset -2px -2px 6px #ffffff;
            }
    
            .gpio-card button {
                margin-top: 0.5rem;
                padding: 0.5rem 1rem;
                border: none;
                border-radius: 12px;
                font-weight: bold;
                background: #f0f0f0;
                box-shadow: 5px 5px 10px #bebebe,
                    -5px -5px 10px #ffffff;
                cursor: pointer;
                transition: background 0.2s;
            }
    
            .gpio-card button.active {
                background-color: #a5d6a7;
            }
    
            .gauge-section {
                display: flex;
                flex-wrap: wrap;
                justify-content: center;
                gap: 2rem;
                align-items: center;
            }
    
            .gauge-shell {
                width: 220px;
                height: 220px;
                border-radius: 50%;
                background: #e0e0e0;
                box-shadow: 9px 9px 16px #bebebe,
                    -9px -9px 16px #ffffff,
                    inset 5px 5px 15px #bebebe,
                    inset -5px -5px 15px #ffffff;
                display: flex;
                align-items: center;
                justify-content: center;
                position: relative;
            }
    
            svg.gauge {
                width: 160px;
                height: 160px;
                transform: rotate(-90deg);
            }
    
            .gauge-text {
                position: absolute;
                font-size: 1.6rem;
                font-weight: bold;
                color: #333;
            }
    
            .led-toggle {
                background: #e0e0e0;
                border-radius: 20px;
                box-shadow: 9px 9px 16px #bebebe,
                    -9px -9px 16px #ffffff;
                padding: 1rem 2rem;
                text-align: center;
                min-width: 160px;
            }
    
            .led-toggle button {
                padding: 0.5rem 1rem;
                border: none;
                border-radius: 12px;
                background: #f0f0f0;
                font-weight: bold;
                cursor: pointer;
                margin-top: 1.5rem;
                box-shadow: 5px 5px 10px #bebebe,
                    -5px -5px 10px #ffffff;
            }
    
            .led-toggle button.active {
                background-color: #ffca28;
            }
    
            @media (max-width: 768px) {
                .gpio-banks {
                    flex-direction: column;
                    align-items: center;
                }
    
                .gauge-section {
                    flex-direction: column;
                }
            }
        </style>
    </head>
    
    <body>
        <h1>Pico GPIO Dashboard</h1>
    
        <div class="gpio-banks" id="gpioBanks">
            <!-- JS adds GPIOs -->
        </div>
    
        <div class="gauge-section">
            <div class="gauge-shell">
                <svg class="gauge" viewBox="0 0 100 100">
                    <circle cx="50" cy="50" r="45" stroke="#ddd" stroke-width="10" fill="none" />
                    <circle id="gaugeArc" cx="50" cy="50" r="45" stroke="#81c784" stroke-width="10" fill="none"
                        stroke-linecap="round" stroke-dasharray="283" stroke-dashoffset="283" />
                </svg>
                <div class="gauge-text" id="tempValue">--°C</div>
            </div>
    
            <div class="led-toggle">
                <div>Wi-Fi LED</div>
                <button id="ledBtn" onclick="toggleLED()">OFF</button>
            </div>
        </div>
    
        <script>
            const gpioBanks = [
              [2, 3, 4, 5],
              [6, 7, 8, 9],
              [16, 17, 18, 19]
            ];
          
            const gpioButtons = {}; // pin -> button
          
            function createGpioCards() {
              const container = document.getElementById("gpioBanks");
          
              gpioBanks.forEach(bank => {
                const bankDiv = document.createElement("div");
                bankDiv.className = "bank";
          
                bank.forEach(pin => {
                  const card = document.createElement("div");
                  card.className = "gpio-card";
          
                  const label = document.createElement("div");
                  label.textContent = `GPIO ${pin}`;
          
                  const btn = document.createElement("button");
                  btn.textContent = "OFF";
                  btn.onclick = () => toggleGPIO(pin, btn);
          
                  card.appendChild(label);
                  card.appendChild(btn);
                  bankDiv.appendChild(card);
          
                  gpioButtons[pin] = btn;
                });
          
                container.appendChild(bankDiv);
              });
            }
          
            function toggleGPIO(pin, btn) {
              const isActive = btn.classList.toggle("active");
              const state = isActive ? 1 : 0;
              btn.textContent = state ? "ON" : "OFF";
              fetch(`/api/v1/gpio/${pin}/${state}`, { method: "POST" })
                .catch(err => console.warn(`GPIO ${pin} toggle failed`, err));
            }
          
            function syncAllGpios() {
            const pins = Object.keys(gpioButtons);
    
            if (pins.length === 0) {
                console.warn("No GPIO pins to sync");
                return;
            }
    
            const params = new URLSearchParams();
            pins.forEach(pin => params.append("pin", pin));
    
            fetch(`/api/v1/gpios?${params.toString()}`)
                .then(res => res.json())
                .then(data => {
                data.forEach(pinData => {
                    const pin = pinData.pin;
                    const isOn = pinData.state === 1;
                    const btn = gpioButtons[pin];
                    if (btn) {
                    btn.classList.toggle("active", isOn);
                    btn.textContent = isOn ? "ON" : "OFF";
                    }
                });
                })
                .catch(err => console.warn("Failed to sync GPIOs", err));
            }
         
            function setupLedButton(ledBtn) {
              function applyLedState(isOn) {
                ledBtn.classList.toggle("active", isOn);
                ledBtn.textContent = isOn ? "ON" : "OFF";
              }
          
              ledBtn.onclick = () => {
                const isOn = !ledBtn.classList.contains("active");
                applyLedState(isOn);
                fetch(`/api/v1/led/${isOn ? 1 : 0}`, { method: "POST" })
                  .catch(err => console.warn("LED toggle failed", err));
              };
          
              fetch('/api/v1/led')
                .then(res => res.json())
                .then(data => {
                  const isOn = data.state === 1 || data.state === "on";
                  applyLedState(isOn);
                })
                .catch(err => console.warn("LED sync failed", err));
            }
          
            function setupTemperatureGauge(arc, tempValue) {
              function update(temp) {
                tempValue.textContent = `${temp.toFixed(1)}°C`;
                const percent = Math.min(Math.max(temp / 60, 0), 1);
                arc.style.strokeDashoffset = 283 * (1 - percent);
                arc.style.stroke =
                  temp < 50 ? "#81c784" :
                  temp < 65 ? "#ffb74d" :
                              "#e57373";
              }
          
              function refresh() {
                fetch('/api/v1/temperature')
                  .then(res => res.json())
                  .then(data => update(data.temperature))
                  .catch(err => {
                    console.warn("Temp fetch failed", err);
                    tempValue.textContent = "--°C";
                    arc.style.strokeDashoffset = 283;
                    arc.style.stroke = "#ccc";
                  });
              }
          
              refresh(); // initial
              setInterval(refresh, 10000); // every 10s
            }
          
            // ----- Full load: all layout and rendering done -----
            window.addEventListener("load", () => {
              createGpioCards();
              syncAllGpios();
          
              const ledBtn = document.getElementById("ledBtn");
              setupLedButton(ledBtn);
          
              const arc = document.getElementById("gaugeArc");
              const tempValue = document.getElementById("tempValue");
              setupTemperatureGauge(arc, tempValue);
            });
          </script>
    </body>
    </html>
    )!";
};
