#include <pico/cyw43_arch.h>
#include <hardware/adc.h>

#include "network/Network.h"
#include "PicoModel.h"
#include "utility/WithFlag.h"

// Define your model properties and methods here
// For example, you might have properties for temperature, LED state, etc.

// Add methods to manipulate these properties

PicoModel::PicoModel()
    : FrameworkModel("pico_model.json") // Initialize with a storage manager and path
{
}

void PicoModel::onStart()
{
    // Initialize GPIO for LED
    restoreState(); // Restore the state of the model
}

float PicoModel::getTemperature()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    float voltage = adc_read() * (3.3f / (1 << 12));
    return 27.0f - (voltage - 0.706f) / 0.001721f; // Convert voltage to temperature
}
// LED state management

void PicoModel::setLedState(bool state)
{
    printf("[PicoModel] Setting Wi-Fi LED state to %s\n", state ? "ON" : "OFF");
    cyw43_arch_gpio_put(0, state ? 1 : 0); // Set GPIO 0 for LED state
    printf("[PicoModel] Wi-Fi LED state set to %s\n", state ? "ON" : "OFF");
    setValue("led", getLedState());
    printf("[PicoModel] LED state saved to model\n");
    if (!suppressSave)
    {
        printf("[PicoModel] Saving state...\n");
        saveState(); // Only save if not restoring
        printf("[PicoModel] State saved\n");
    }
    printf("[PicoModel] end of LED state set to %s\n", state ? "ON" : "OFF");
}
bool PicoModel::getLedState()
{
    if (cyw43_is_initialized){
        printf("[PicoModel] Getting Wi-Fi LED state\n");
        return cyw43_arch_gpio_get(0); // Get GPIO 0 state for LED
    }
    else{
        printf("[PicoModel] Wi-Fi LED not initialized\n");
        return false; // Return false if Wi-Fi is not initialized
    }
}

bool PicoModel::getGpioState(int pin)
{
    bool state = gpio_get(pin);
    return state; // Return the state of the specified GPIO pin
}

void PicoModel::setGpioState(int pin, bool state)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, state);
    if (!suppressSave)
    {
        saveState(); // Only save if not restoring
    }
}

void PicoModel::saveState()
{
    // Update the model with the new state
    // Save the LED state
    setValue("led", getLedState());
    // Save the state of all active GPIO pins
    json gpios;
    for (int pin : activePins)
    {
        gpios[std::to_string(pin)] = getGpioState(pin);
    }
    setValue("gpio_states", gpios);
    printf("[PicoModel] Saving state to storage...\n");
    save();
}

void PicoModel::onNetworkReady()
{
    // This method is called when the network is ready
    // Initialize the LED state based on the stored value
    printf("[PicoModel] Network is ready, initializing LED state...\n");
    bool ledState = getValue("led", false);
    printf("[PicoModel] Restoring LED state: %s\n", ledState ? "ON" : "OFF");
    WithFlag _(suppressSave); // Suppress saving during initialization
    printf("[PicoModel] Setting LED state: %s\n", ledState ? "ON" : "OFF");
    setLedState(ledState);
    printf("[PicoModel] LED state set to %s\n", ledState ? "ON" : "OFF");
    printf("[PicoModel] Network ready, LED state initialized to %s\n", ledState ? "ON" : "OFF");
}

void PicoModel::restoreState()
{
    if (!load())
        return;
    printf("[PicoModel] Restoring state from storage...\n");
    // Restore LED state
    suppressSave = true; // Suppress saving during restore
    if (Network::isConnected())
    {
        bool ledState = getValue("led", false);
        setLedState(ledState);
    }
    else
    {
        printf("[PicoModel] Network not connected, deferring LED initialization\n");
    }

    // Restore GPIO pin states
    auto gpioStates = getValue<json>("gpio_states", {});
    for (auto &[pinStr, stateJson] : gpioStates.items())
    {
        int pin = std::stoi(pinStr);
        bool state = stateJson.get<bool>();
        setGpioState(pin, state);
    }
    suppressSave = false; // Re-enable saving after restore
}
