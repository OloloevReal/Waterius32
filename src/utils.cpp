#include "utils.h"


//Get ESP32 Chip ID in ESP8266 style - last three words of MAC
uint32_t getChipId(){
  uint8_t efuse_mac[6];
  esp_efuse_mac_get_default(efuse_mac);
  uint32_t chipID = ((uint32_t)efuse_mac[3] << 16) | ((uint32_t)efuse_mac[4] << 8) | efuse_mac[5];
  return chipID;
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
esp_sleep_wakeup_cause_t print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : LOG_NOTICE("ESP", "Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : LOG_NOTICE("ESP","Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : LOG_NOTICE("ESP","Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : LOG_NOTICE("ESP","Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : LOG_NOTICE("ESP","Wakeup caused by ULP program"); break;
    default : LOG_NOTICE("ESP","Wakeup was not caused by deep sleep: " << wakeup_reason); break;
  }
  return wakeup_reason;
}

void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET");break;          /**<1,  Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}

// typedef enum {
//     ADC1_CHANNEL_0 = 0, /*!< ADC1 channel 0 is GPIO36 */
//     ADC1_CHANNEL_1,     /*!< ADC1 channel 1 is GPIO37 */
//     ADC1_CHANNEL_2,     /*!< ADC1 channel 2 is GPIO38 */
//     ADC1_CHANNEL_3,     /*!< ADC1 channel 3 is GPIO39 */
//     ADC1_CHANNEL_4,     /*!< ADC1 channel 4 is GPIO32 */
//     ADC1_CHANNEL_5,     /*!< ADC1 channel 5 is GPIO33 */
//     ADC1_CHANNEL_6,     /*!< ADC1 channel 6 is GPIO34 */
//     ADC1_CHANNEL_7,     /*!< ADC1 channel 7 is GPIO35 */
//     ADC1_CHANNEL_MAX,
// } adc1_channel_t;

// typedef enum {
//     ADC2_CHANNEL_0 = 0, /*!< ADC2 channel 0 is GPIO4 */
//     ADC2_CHANNEL_1,     /*!< ADC2 channel 1 is GPIO0 */
//     ADC2_CHANNEL_2,     /*!< ADC2 channel 2 is GPIO2 */
//     ADC2_CHANNEL_3,     /*!< ADC2 channel 3 is GPIO15 */
//     ADC2_CHANNEL_4,     /*!< ADC2 channel 4 is GPIO13 */
//     ADC2_CHANNEL_5,     /*!< ADC2 channel 5 is GPIO12 */
//     ADC2_CHANNEL_6,     /*!< ADC2 channel 6 is GPIO14 */
//     ADC2_CHANNEL_7,     /*!< ADC2 channel 7 is GPIO27 */
//     ADC2_CHANNEL_8,     /*!< ADC2 channel 8 is GPIO25 */
//     ADC2_CHANNEL_9,     /*!< ADC2 channel 9 is GPIO26 */
//     ADC2_CHANNEL_MAX,
// } adc2_channel_t;

esp_err_t io_num_get_adc1(gpio_num_t gpio_num, adc1_channel_t *channel)
{
  switch (gpio_num) { 
    case GPIO_NUM_36:
      *channel = ADC1_CHANNEL_0;
      break;
    case GPIO_NUM_37:
      *channel = ADC1_CHANNEL_1;
      break;
    case GPIO_NUM_38:
      *channel = ADC1_CHANNEL_2;
      break;
    case GPIO_NUM_39:
      *channel = ADC1_CHANNEL_3;
      break;
    case GPIO_NUM_32:
      *channel = ADC1_CHANNEL_4;
      break;
    case GPIO_NUM_33:
      *channel = ADC1_CHANNEL_5;
      break;
    case GPIO_NUM_34:
      *channel = ADC1_CHANNEL_6;
      break;
    case GPIO_NUM_35:
      *channel = ADC1_CHANNEL_7;
      break;
    default:
        return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}

esp_err_t io_num_get_adc_channel(gpio_num_t gpio_num, adc_channel_t *channel)
{
  switch (gpio_num) { 
    case GPIO_NUM_36:
      *channel = ADC_CHANNEL_0;
      break;
    case GPIO_NUM_37:
      *channel = ADC_CHANNEL_1;
      break;
    case GPIO_NUM_38:
      *channel = ADC_CHANNEL_2;
      break;
    case GPIO_NUM_39:
      *channel = ADC_CHANNEL_3;
      break;
    case GPIO_NUM_32:
      *channel = ADC_CHANNEL_4;
      break;
    case GPIO_NUM_33:
      *channel = ADC_CHANNEL_5;
      break;
    case GPIO_NUM_34:
      *channel = ADC_CHANNEL_6;
      break;
    case GPIO_NUM_35:
      *channel = ADC_CHANNEL_7;
      break;
    case GPIO_NUM_4:
      *channel = ADC_CHANNEL_0;
      break;
    case GPIO_NUM_0:
      *channel = ADC_CHANNEL_1;
      break;
    case GPIO_NUM_2:
      *channel = ADC_CHANNEL_2;
      break;
    case GPIO_NUM_15:
      *channel = ADC_CHANNEL_3;
      break;
    case GPIO_NUM_13:
      *channel = ADC_CHANNEL_4;
      break;
    case GPIO_NUM_12:
      *channel = ADC_CHANNEL_5;
      break;
    case GPIO_NUM_14:
      *channel = ADC_CHANNEL_6;
      break;
    case GPIO_NUM_27:
      *channel = ADC_CHANNEL_7;
      break;
    case GPIO_NUM_25:
      *channel = ADC_CHANNEL_8;
      break;
    case GPIO_NUM_26:
      *channel = ADC_CHANNEL_9;
      break;
    default:
        return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}

esp_err_t io_num_get_adc_unit(gpio_num_t gpio_num, adc_unit_t *unit)
{
  switch (gpio_num) { 
    case GPIO_NUM_36:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_37:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_38:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_39:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_32:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_33:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_34:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_35:
      *unit = ADC_UNIT_1;
      break;
    case GPIO_NUM_4:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_0:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_2:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_15:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_13:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_12:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_14:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_27:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_25:
      *unit = ADC_UNIT_2;
      break;
    case GPIO_NUM_26:
      *unit = ADC_UNIT_2;
      break;
    default:
        return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}

uint16_t get_CH0(){
  return (RTC_SLOW_MEM[RTC_MEM_ADC_CH0_RAW] & 0xFFFF);
};

uint16_t get_CH1(){
  return (RTC_SLOW_MEM[RTC_MEM_ADC_CH1_RAW] & 0xFFFF);
};

uint16_t get_CH0_count(){
  return (RTC_SLOW_MEM[RTC_MEM_ADC_CH0] & 0xFFFF);
};

uint16_t get_CH1_count(){
  return (RTC_SLOW_MEM[RTC_MEM_ADC_CH1] & 0xFFFF);
};

uint16_t get_CH0_state(){
  return 0;
}

uint16_t get_CH1_state(){
  return 0;
}
