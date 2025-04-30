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

        // We can use the base class methods for JSON operations
        bool load(); // we will use the load model from the base class
        bool save(); // we will use the save model from the base class
        
    private:

        // There are no "shadowing variables" for state in this class, so no need
        // to define any private variables here.
        
};

