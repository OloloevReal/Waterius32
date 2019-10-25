#ifndef _BUTTON_h
#define _BUTTON_h

#include <Arduino.h>
#include <led.h>

#define LEDBlinkOff     250
#define LEDBlinkOn      250

const char STR_NO_PRESS[]         PROGMEM = "NO_PRESS";
const char STR_SHORT_PRESS[]      PROGMEM = "SHORT_PRESS";
const char STR_LONG_PRESS[]       PROGMEM = "LONG_PRESS";
const char STR_VERY_LONG_PRESS[]  PROGMEM = "VERY_LONG_PRESS";
const char STR_UNDEFINED[]        PROGMEM = "UNDEFINED";

enum ButtonState_e
{
    NO_PRESS,
    SHORT_PRESS,
    LONG_PRESS,
    VERY_LONG_PRESS
};

struct Button
{    
    gpio_num_t _pin;
    uint8_t _checks;
    ButtonState_e lastState;
    Led *_led;

    Button(gpio_num_t pin)
        : _pin(pin)
        , _checks(0)
        , lastState(NO_PRESS)
    {
    }

    void SetLed(Led *led){
        this->_led = led;
    }

    inline uint16_t State() 
    {
        return rtc_gpio_get_level(this->_pin) && 0xFFFF;
    }

    inline bool Pressed() 
    {
        if (this->State() == HIGH)
        {	//защита от дребезга
            delayMicroseconds(20000); //TODO: move to setup.h
            return this->State() == HIGH;
        }
        return false;
    }

    inline ButtonState_e Wait_button_release()
    {
        this->_led->LEDTurnOn(); //FIXME: LED is blink briefly if NO_PRESS
        this->lastState = ButtonState_e::NO_PRESS;
        gpio_set_direction(this->_pin, GPIO_MODE_INPUT);
        unsigned long press_time = millis();
        while(this->Pressed()){ //TODO: add checks BTN_MAX_PRESS
            this->handler(millis() - press_time);
        }
        rtc_gpio_init(this->_pin);
        rtc_gpio_set_direction(this->_pin, RTC_GPIO_MODE_INPUT_ONLY);
        rtc_gpio_pullup_dis(this->_pin);
        rtc_gpio_pulldown_dis(this->_pin);

        press_time = millis() - press_time;
        this->_led->LEDTurnOff();
        return this->value2state(press_time);
    }

    inline void handler(unsigned long value){
        if (this->_led->LEDAllowed()){
            ButtonState_e state = value2state(value);
            if (this->lastState != state){
                LOG_DEBUG("BTN", "State changed from " << this->lastState << " (" << this->state2char(this->lastState) << ")" << " to " << state << " (" << this->state2char(state) << ")");
                this->lastState = state;
                switch (state)
                {
                case ButtonState_e::SHORT_PRESS:
                    this->_led->LEDBlink(state);
                    break;
                case ButtonState_e::LONG_PRESS:
                    this->_led->LEDBlink(state);
                    break;
                case ButtonState_e::VERY_LONG_PRESS:
                    this->_led->LEDBlink(state);
                    break;
                default:
                    break;
                }
            }
        }
    }

    inline int get_rtc_num() 
    {
        return rtc_gpio_desc[this->_pin].rtc_num;
    }   

    inline enum ButtonState_e value2state(unsigned long value) 
    {
        if (value >= BTN_VERY_LONG_PRESS) {
            return ButtonState_e::VERY_LONG_PRESS;
        } else if (value >= BTN_LONG_PRESS) {
            return ButtonState_e::LONG_PRESS;
        } else if (value >= BTN_SHORT_PRESS) {
            return ButtonState_e::SHORT_PRESS;
        } else {
            return ButtonState_e::NO_PRESS;
        }
    }

    //TODO: replace to array 
    inline const char* state2char(ButtonState_e state){
        switch (state)
        {
        case ButtonState_e::NO_PRESS:
            return STR_NO_PRESS;
        case ButtonState_e::SHORT_PRESS:
            return STR_SHORT_PRESS;
        case ButtonState_e::LONG_PRESS:
            return STR_LONG_PRESS;
        case ButtonState_e::VERY_LONG_PRESS:
            return STR_VERY_LONG_PRESS;
        default:
            return STR_UNDEFINED;
        }
    }
};

#endif