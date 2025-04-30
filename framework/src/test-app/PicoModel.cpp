#include <pico/cyw43_arch.h>
#include <hardware/adc.h>

#include "PicoModel.h"

    // Define your model properties and methods here
    // For example, you might have properties for temperature, LED state, etc.

    // Add methods to manipulate these properties
   
    PicoModel::PicoModel()
        : FrameworkModel(nullptr, "pico_model.json") // Initialize with a storage manager and path
    {
        // Constructor implementation can be empty if no initialization is needed
    }
   
    float PicoModel::getTemperature() {
        adc_init();
        adc_set_temp_sensor_enabled(true);
        adc_select_input(4);
        float voltage = adc_read() * (3.3f / (1 << 12));
        return 27.0f - (voltage - 0.706f) / 0.001721f; // Convert voltage to temperature
    }
    // LED state management 

    void PicoModel::setLedState(bool state){
        cyw43_arch_gpio_put(0, state ? 1 : 0); // Set GPIO 0 for LED state
    }
    bool PicoModel::getLedState(){
        return cyw43_arch_gpio_get(0); // Get GPIO 0 state for LED
    }

    bool PicoModel::getGpioState(int pin){  
        bool state = gpio_get(pin);
        return state; // Return the state of the specified GPIO pin
    }
        
    void PicoModel::setGpioState(int pin, bool state){
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, state);    
    }

    // We can use the base class methods for JSON operations
    bool PicoModel::load(){
        return FrameworkModel::load(); // we will use the load model from the base class
    }

    bool PicoModel::save(){
        return FrameworkModel::save(); // we will use the save model from the base class
    } 