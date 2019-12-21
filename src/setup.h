#ifndef _SETUP_h
#define _SETUP_h

#include <Arduino.h>

#define FIRMWARE_VERSION "0.1.18"

#define LED 2

#define BTN_SHORT_PRESS     1000
#define BTN_LONG_PRESS      5000
#define BTN_VERY_LONG_PRESS 15000
#define BTN_MAX_PRESS       20000 //TODO: add cheking for max press

#define LIMIT_CLOSED        230 // 3v3, pullup 40k, 3.6 kOm
#define LIMIT_NAMUR_CLOSED  360 // 3v3, pullup 40k, 5.2 kOm
#define LIMIT_NAMUR_OPEN    800 // 3v3, pullup 40k, 9.2 kOm

/*
!!! Keep out !!! 
Doesn't change RTC_MEM_* adresses - asm macros uses address shifts
*/
#define RTC_MEM_PROG_START          0x000
#define RTC_MEM_ADC_CH0_RAW         0x700   // ADC Value
#define RTC_MEM_ADC_CH0             0x701   // Counter value
#define RTC_MEM_ADC_CH0_CHECKS_1    0x702
#define RTC_MEM_ADC_CH0_CHECKS_2    0x703   // don't use with old macro I_IS_IMPULS
#define RTC_MEM_ADC_CH1_RAW         0x704   // ADC Value
#define RTC_MEM_ADC_CH1             0x705   // Counter value
#define RTC_MEM_ADC_CH1_CHECKS_1    0x706
#define RTC_MEM_ADC_CH1_CHECKS_2    0x707   // don't use with old macro I_IS_IMPULS
#define RTC_MEM_SENSORS             0x708
#define RTC_MEM_WS_0_RAW            0x709
#define RTC_MEM_WS_1_RAW            0x70A
#define RTC_MEM_WS_2_RAW            0x70B
#define RTC_MEM_WS_3_RAW            0x70C
//#define RTC_MEM_ADC_DIS             0x70A
#define RTC_MEM_FLAG                0x7FD
#define RTC_MEM_FLAG_CP             0x7FE
#define RTC_MEM_BTN                 0x7FF
#define RTC_MEM_BUFFER_TOP          0x800

#define TRIES1 2            // Сколько раз проверяем вход, пока не вернем замыкание LOW->HIGH
#define TRIES2 4            // Повтор проверок смены уровня HIGH -> LOW

#define WS_ENABLED          // Enable water sensors
//#define NB_IOT            // Enable NB-IOT instead of WiFi
#define USE_EXT_VCC         // Enable additional power

#define ESP_CONNECT_TIMEOUT 15000
#define ESP_WAKEUP_TIMER 2                          // min
#define ESP_WAKEUP_TIMER_FACTOR 60 * 1000 * 1000    // msec

/*
    Уровень логирования
*/
#define LOGLEVEL 7

/*
    Время ответа сервера
*/
#define SERVER_TIMEOUT 5000UL // ms

#define WIFI
//#define NBIOT

/*
    Включить отправку данных в приложение Blynk.cc
*/
//#define SEND_BLYNK

/*
    Включить отправку данных на HTTP сервер
*/
//#define SEND_JSON

/*
    Включить отправку данных на сервер по COAP
*/
#define SEND_COAP


/*
    При первом включении заполним 10 литров на импульс
*/
#define LITRES_PER_IMPULS_DEFAULT 10

#define KEY_LEN 34
#define HOSTNAME_LEN 32
#define SN_LEN 16

#define EMAIL_LEN 32
#define EMAIL_TITLE_LEN 64
#define EMAIL_TEMPLATE_LEN 200

#define VER_1 1
#define VER_2 2
#define VER_3 3
#define CURRENT_VERSION VER_3

enum mode_e
{
    SYNC_MODE = 0,
    SETUP_MODE = 1,
    TRANSMIT_MODE = 2,
    CLEAR_MODE = 3,
    UPGRADE_MODE = 4,
    WATERLEAK_MODE = 5
};

struct CalculatedData
{
    float channel0;
    float channel1;
    uint32_t delta0;
    uint32_t delta1;
};

/*
Настройки хранящиеся EEPROM
*/
struct Settings
{
    uint8_t version;  //Версия конфигурации
    uint8_t reserved; //Для кратности x16 bit

    /*
    SEND_BLYNK: уникальный ключ устройства blynk
    SEND_TCP: не используется
    */
    char key[KEY_LEN];

    /*
    SEND_BLYNK: сервер blynk.com или свой blynk сервер
    SEND_TCP: ip адрес или имя хоста куда слать данные
    */
    char hostname[HOSTNAME_LEN];

    /*
    SEND_BLYNK: Если email не пустой, то отсылается e-mail
    SEND_TCP: не используется    
    */
    char email[EMAIL_LEN];

    /*
    SEND_BLYNK: Заголовок письма. {V0}-{V4} заменяются на данные 
    SEND_TCP: не используется    
    */
    char email_title[EMAIL_TITLE_LEN];

    /*
    SEND_BLYNK: Шаблон эл. письма. {V0}-{V4} заменяются на данные 
    SEND_TCP: не используется    
    */
    char email_template[EMAIL_TEMPLATE_LEN];

    /*
    http сервер для отправки данных в виде JSON
    в виде: http://host:port/path
    */
    char hostname_json[EMAIL_TITLE_LEN];

    /*
    Название устройства в JSON
    */
    char name[HOSTNAME_LEN];

    /*
    Описание устройства, например, место или адрес установки
     */
    char description[EMAIL_TITLE_LEN];

    /*
    Показания счетчиках в кубометрах, 
    введенные пользователем при настройке
    */
    float channel0_start;
    float channel1_start;

    /*
    Кол-во литров на 1 импульс
    */
    uint16_t liters_per_impuls;

    /*
    Кол-во импульсов Attiny85 соответствующие показаниям счетчиков, 
    введенных пользователем при настройке
    */
    uint32_t impulses0_start;
    uint32_t impulses1_start;

    /*
    Не понятно, как получить от Blynk прирост показаний, 
    поэтому сохраним их в памяти каждое включение
    */
    uint32_t impulses0_previous;
    uint32_t impulses1_previous;
    bool watersensor_previous;

    uint16_t wake_every_min;

    /*
    SN подключенных счетчиков
    */
    char sn0[SN_LEN];
    char sn1[SN_LEN];

    uint8_t sensors;

    char coap_hostname[HOSTNAME_LEN];

    /*
    Контрольная сумма, чтобы гарантировать корректность чтения настроек
    */
    uint8_t crc_cfg;
};

/*
Данные принимаемые от Attiny
*/
struct SlaveData
{
    uint8_t version;    //Версия ПО Attiny
    uint8_t service;    //Причина загрузки Attiny
    uint32_t voltage;   //Напряжение питания в мВ
    uint8_t state0;     //Состояние входа 0
    uint8_t state1;     //Состояние входа 1
    uint32_t impulses0; //Импульсов, канал 0
    uint32_t impulses1; //Импульсов, канал 1
    uint8_t diagnostic; //1 - ок, 0 - нет связи с Attiny
    uint16_t WAKE_EVERY_MIN;
    uint16_t memoryFree;
    uint8_t resets;
    bool waterSensor;
    //Кратно 16bit https://github.com/esp8266/Arduino/issues/1825
};

#endif