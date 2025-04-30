#pragma once

#include "framework/FrameworkModel.h"

class PicoModel : public FrameworkModel
{

    public:
        PicoModel();

        // Define your model properties and methods here
        // For example, you might have properties for temperature, LED state, etc.

        // Add methods to manipulate these properties
        float getTemperature();

        void setLedState(bool state);
        bool getLedState();

        bool getGpioState(int pin);
        void setGpioState(int pin, bool state);

        void restoreState();
        
    private:
        std::vector<int> activePins = { 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19 };  // Pins in use - staying away from sd card pins

};

