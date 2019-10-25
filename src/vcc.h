#ifndef _VCC_h
#define _VCC_h

#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_err.h>
#include <esp_adc_cal.h>
#include <driver/rtc_io.h>
#include "utils.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate !!! not sure if this works !!!

struct Vcc {
    gpio_num_t  _pin;
    esp_adc_cal_characteristics_t *_adc_chars;
    adc_unit_t _unit;
    adc_channel_t _channel;
    adc_atten_t _atten;
    adc_bits_width_t _width;
    int _no_of_samples;
    gpio_num_t  _extVCC;
    gpio_num_t _devider_pin;

    Vcc(gpio_num_t pin)  
        : _pin(pin),
        _devider_pin(GPIO_NUM_13)
    {
        io_num_get_adc_unit(this->_pin, &this->_unit);
        io_num_get_adc_channel(this->_pin, &this->_channel);
        this->_atten = ADC_ATTEN_DB_11;
        this->_width = ADC_WIDTH_BIT_12;
        this->_no_of_samples = 64;
    }

    void extVCCSet(gpio_num_t pin){
        this->_extVCC = pin;
    }

    void extVCCOn(){
        gpio_pad_select_gpio(this->_extVCC);
        gpio_set_direction(this->_extVCC, GPIO_MODE_OUTPUT);
        gpio_set_level(this->_extVCC, HIGH);
    }

    void extVCCOff(){
        gpio_set_level(this->_extVCC, LOW);
    }

    void extDeviderOn(){
        gpio_reset_pin(this->_devider_pin);
        gpio_set_direction(this->_devider_pin, GPIO_MODE_OUTPUT);
        gpio_pullup_dis(this->_devider_pin);
        gpio_pulldown_dis(this->_devider_pin);
        gpio_set_level(this->_devider_pin, HIGH);
    }

    void extDeviderOff(){
        gpio_set_level(this->_devider_pin, LOW);
        gpio_set_direction(this->_devider_pin, GPIO_MODE_INPUT);
    }

    uint32_t readVCC(){
        extDeviderOn();
        this->_adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(this->_unit, this->_atten, this->_width, DEFAULT_VREF, this->_adc_chars);

        gpio_set_direction(this->_pin, GPIO_MODE_INPUT);
        if (this->_unit == ADC_UNIT_1) {
            adc1_config_channel_atten((adc1_channel_t)this->_channel, this->_atten);
            adc1_config_width(this->_width);
        }else{
            adc2_config_channel_atten((adc2_channel_t)this->_channel, this->_atten);
        }
        gpio_pullup_dis (this->_pin);
        gpio_pulldown_dis (this->_pin);

        uint32_t adc_reading = 0;
        for (int i = 0; i < this->_no_of_samples; i++) {
            if (this->_unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)this->_channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)this->_channel, this->_width, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= this->_no_of_samples;
        
        extDeviderOff();
        free(this->_adc_chars);
        return esp_adc_cal_raw_to_voltage(adc_reading, this->_adc_chars);
    }

    //GPIO number (gpios 25,26,27 supported)
    esp_err_t calibrate(gpio_num_t gpio){
        return adc2_vref_to_gpio(gpio);
    }
};  

#endif