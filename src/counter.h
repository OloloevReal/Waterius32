#ifndef _COUNTER_h
#define _COUNTER_h

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <driver/adc.h>

#define TRIES 2  // Сколько раз проверяем вход, пока не вернем замыкание

enum CounterState_e
{
    CLOSE,
    NAMUR_CLOSE,
    NAMUR_OPEN,
    OPEN
};

struct Counter 
{
    gpio_num_t  _pin;       // дискретный вход
    uint8_t _checks;
    adc1_channel_t _adc;    // adc number

    uint8_t state; // состояние входа

    Counter(gpio_num_t pin)  
      : _pin(pin)
      , _checks(0)
    {
       if(io_num_get_adc1(_pin, &_adc) != ESP_OK){
           LOG_ERROR("CNT", "Error definition ID of ADC");
       }
    }

    inline void init(){
        rtc_gpio_set_direction(this->_pin, RTC_GPIO_MODE_INPUT_ONLY);
        adc1_config_channel_atten(this->_adc, ADC_ATTEN_DB_11);
        adc1_config_width(ADC_WIDTH_BIT_12);
        rtc_gpio_pullup_en(this->_pin);
        rtc_gpio_pulldown_dis(this->_pin);
        adc1_ulp_enable();
    }
};

#endif