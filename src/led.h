#ifndef _LED_h
#define _LED_h

#include <Arduino.h>

#define LEDBlinkOff     250
#define LEDBlinkOn      250

struct Led
{    
    gpio_num_t _pin;
    Led(gpio_num_t pin)
        : _pin(pin)
    {

    }

    void LEDSet(gpio_num_t pin){
        this->_pin = pin;
    }

    bool LEDAllowed(){
        return this->_pin > 0 ? true : false;
    }

    void LEDTurnOn(){
        if(this->LEDAllowed()){
            gpio_pad_select_gpio(this->_pin);
            gpio_set_direction(this->_pin, GPIO_MODE_OUTPUT);
            gpio_set_level(this->_pin, HIGH);
        }
    }

    void LEDTurnOff(){
        if(this->LEDAllowed()){
            gpio_set_level(this->_pin, LOW);
        }
    }

    //TODO: swap to Ticker.h after fixed https://github.com/espressif/arduino-esp32/pull/2849
    void LEDBlink(int8_t num){
        if(this->LEDAllowed()){
            for (int i = 0; i < num; i++){
                gpio_set_level(this->_pin, LOW);
                delay(LEDBlinkOff);
                gpio_set_level(this->_pin, HIGH);
                delay(LEDBlinkOn);
            }
        }
    }

};

#endif