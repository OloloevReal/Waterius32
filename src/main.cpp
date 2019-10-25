#include "Logging.h"
#include "utils.h"
#include "ulp.h"
#include "button.h"
#include "counter.h"
#include "led.h"
#include "wifi_settings.h"
#include "setup_ap.h"
#include "sender_json.h"
#include "sender_blynk.h"
#include "sender_coap.h"
#include "vcc.h"

/*
__        ___  _____ _____ ____  ___ _   _ ____       _________  
\ \      / / \|_   _| ____|  _ \|_ _| | | / ___|     |___ /___ \ 
 \ \ /\ / / _ \ | | |  _| | |_) || || | | \___ \ _____ |_ \ __) |
  \ V  V / ___ \| | | |___|  _ < | || |_| |___) |_____|__) / __/ 
   \_/\_/_/   \_\_| |_____|_| \_\___|\___/|____/     |____/_____|

developed by Nikolay
original Waterius project -> https://github.com/dontsovcmc/waterius
*/

/*
GPIO-25 - Setup button

GPIO-32 - analog counter_0, using internal PullUp
GPIO-33 - analog counter_1, using internal PullUp

GPIO-02 - BUILD IN LED
GPIO-05 - HIGH level activate additional power supply for WiFi/BT purpose
GPIO-13 - Activate Input voltage measurement
GPIO-27 - Measuring Input voltage, before LDO

GPIO-04 - the sourse of reference voltage for sensors
GPIO-34 - Water Sensor 0
GPIO-35 - Water Sensor 1
GPIO-36 - Water Sensor 2
GPIO-39 - Water Sensor 3

GPIO-26 - ACT LDO NB-IOT Modem EN/DIS
GPIO-16 - RX NB-IOT
GPIO-17 - TX NB-IOT
GPIO-18 - PWR NB-IOT
*/

SlaveData data;       // Данные от ULP
Settings sett;        // Настройки соединения и предыдущие показания из EEPROM //TODO: swap to NVS
CalculatedData cdata; // Вычисляемые данные

static Button btnSetup(GPIO_NUM_25);
static Counter Counter_0(GPIO_NUM_32);
static Counter Counter_1(GPIO_NUM_33);
static Vcc vcc(GPIO_NUM_27);
static Led ledBuiltIn((gpio_num_t)LED);
static Led NBIoT(GPIO_NUM_18); // Временно, для активации модема

//int32_t waterCount = 0;
unsigned long btnTimeStart = 0;
unsigned long btnTimeLast = 0;
bool btnInterrupt = false;

void btnHandlerDown(){
    if ((millis() - btnTimeLast) > 250)
    {
        btnInterrupt = false;
    }
    if (!btnInterrupt)
    {
        btnInterrupt = true;
        btnTimeLast = btnTimeStart = millis();
        LOG_DEBUG("ESP", "btnTimeStart: " << btnTimeStart);
    }

    ledBuiltIn.LEDTurnOn();
    delayMicroseconds(25000);
    ledBuiltIn.LEDTurnOff();

    btnTimeLast = millis();
    unsigned long delta = btnTimeLast - btnTimeStart;
    if (delta >= BTN_LONG_PRESS)
    {
        LOG_DEBUG("ESP", "Interrupt Btn Down!");
        detachInterrupt(btnSetup._pin);
        stop_ap(); //Closing configuration portal
    }
}


void deepSleep()
{
    esp_sleep_enable_timer_wakeup(ESP_WAKEUP_TIMER * ESP_WAKEUP_TIMER_FACTOR); //TODO: replace ESP_WAKEUP_TIMER to value from settings
    esp_sleep_enable_ulp_wakeup();
    RTC_SLOW_MEM[RTC_MEM_BTN] = 0;  //allows to ulp wakeup main cpu if btn pressed
    RTC_SLOW_MEM[RTC_MEM_FLAG] = 0; //clear flag register
    LOG_NOTICE("ESP", "Going to deep sleep");
    esp_deep_sleep_start();
}

void load_resets(SlaveData *data)
{
    nvs_handle my_handle;
    if (NVSInit(&my_handle) != ESP_OK)
    {
        LOG_ERROR("NVS", "Error Init NVS");
    }
    else
    {
        esp_err_t err = nvs_get_u8(my_handle, "RST", &data->resets);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error get RST: " << err);
        }
        NVSClose(my_handle);
    }
}

void sync_data(SlaveData *data)
{
    data->impulses0 = get_CH0_count();
    data->impulses1 = get_CH1_count();
    data->state0 = get_CH0_state();
    data->state1 = get_CH1_state();

    /*
    save impulses0, impulses1 and load resets
    */
    nvs_handle my_handle;
    if (NVSInit(&my_handle) != ESP_OK)
    {
        LOG_ERROR("NVS", "Error Init NVS");
    }
    else
    {
        esp_err_t err = nvs_set_u32(my_handle, "IM0", data->impulses0);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error set IM0: " << err);
        }
        err = nvs_set_u32(my_handle, "IM1", data->impulses1);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error set IM1: " << err);
        }
        err = nvs_set_u8(my_handle, "RST", data->resets);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error set RST: " << err);
        }
        NVSCommit(my_handle);
        NVSClose(my_handle);
    }
}

void restore_data(struct SlaveData *data)
{
    /*
    load impulses0, impulses1, resets from NVS to SlaveData
    */
    nvs_handle my_handle;
    if (NVSInit(&my_handle) != ESP_OK)
    {
        LOG_ERROR("NVS", "Error Init NVS");
    }
    else
    {
        esp_err_t err = nvs_get_u32(my_handle, "IM0", &data->impulses0);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error get IM0: " << err);
        }
        err = nvs_get_u32(my_handle, "IM1", &data->impulses1);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error get IM1: " << err);
        }
        NVSCommit(my_handle);
        NVSClose(my_handle);
    }
    RTC_SLOW_MEM[RTC_MEM_ADC_CH0] = data->impulses0;
    RTC_SLOW_MEM[RTC_MEM_ADC_CH1] = data->impulses1;
}

void calculate_values(Settings &sett, SlaveData *data, CalculatedData *cdata)
{
    LOG_NOTICE("ESP", "loaded impulses=" << data->impulses0 << " " << data->impulses1);

    if (sett.liters_per_impuls > 0)
    {
        //incorrect cdata->delta0 values if data.impulse less than sett.impulses_previous
        if (data->impulses0 < sett.impulses0_previous)
        {
            LOG_WARNING("ESP", "Impulses0 less than impulses0_previous");
            data->impulses0 = sett.impulses0_previous;
            RTC_SLOW_MEM[RTC_MEM_ADC_CH0] = data->impulses0;
        }

        if (data->impulses1 < sett.impulses1_previous)
        {
            LOG_WARNING("ESP", "Impulses1 less than impulses1_previous");
            data->impulses1 = sett.impulses1_previous;
            RTC_SLOW_MEM[RTC_MEM_ADC_CH1] = data->impulses1;
        }

        cdata->channel0 = sett.channel0_start + (data->impulses0 - sett.impulses0_start) / 1000.0 * sett.liters_per_impuls;
        cdata->channel1 = sett.channel1_start + (data->impulses1 - sett.impulses1_start) / 1000.0 * sett.liters_per_impuls;
        LOG_NOTICE("ESP", "new values=" << cdata->channel0 << " " << cdata->channel1);
        cdata->delta0 = (data->impulses0 - sett.impulses0_previous) * sett.liters_per_impuls;
        cdata->delta1 = (data->impulses1 - sett.impulses1_previous) * sett.liters_per_impuls;
        LOG_NOTICE("ESP", "delta values=" << cdata->delta0 << " " << cdata->delta1);
    }
}

void setup()
{
    LOG_BEGIN(115200);
    mode_e mode = mode_e::SYNC_MODE;
    btnSetup.SetLed(&ledBuiltIn);
#ifdef USE_EXT_VCC
    vcc.extVCCSet(GPIO_NUM_5);
#endif

    load_resets(&data);
    LOG_NOTICE("ESP", F("Booted"));
    LOG_NOTICE("ESP", F("ChipId=") << getChipId());
    LOG_DEBUG("ESP", F("Reset_reason(CPU-0)=") << rtc_get_reset_reason(0));
    LOG_DEBUG("ESP", F("Reset_reason(CPU-1)=") << rtc_get_reset_reason(0));
    LOG_DEBUG("ESP", F("Wakeup_cause=") << rtc_get_wakeup_cause());
    LOG_NOTICE("ESP", F("Resets=") << data.resets);
    esp_sleep_wakeup_cause_t reason = print_wakeup_reason();

    /*
    Если причина пробуждения ESP_SLEEP_WAKEUP_TIMER или ESP_SLEEP_WAKEUP_ULP - штатная работа
    Если отличная от перечисленных, то начинаем с самого начала, инициализация, запуск ULP и т.д.
     */

    if (reason != ESP_SLEEP_WAKEUP_TIMER && reason != ESP_SLEEP_WAKEUP_ULP && reason != ESP_SLEEP_WAKEUP_TOUCHPAD)
    {
        LOG_NOTICE("ESP", "Init, first run");
        for (int i = RTC_MEM_PROG_START; i < RTC_MEM_BUFFER_TOP; ++i)
            RTC_SLOW_MEM[i] = 0x0000;
        Counter_0.init();
        Counter_1.init();
        restore_data(&data);
        data.resets++;
        LOG_DEBUG("ESP", F("Resets=") << data.resets);
        ulp_init(&btnSetup, &Counter_0, &Counter_1);
    }
    else
    {
        LOG_NOTICE("ESP", "Not first run, normal work");
    }

    LOG_DEBUG("ESP", "RTC_MEM_BTN: ");
    Serial.println(RTC_SLOW_MEM[RTC_MEM_BTN] & 0xFFFF, BIN);
    LOG_DEBUG("ESP", "RTC_MEM_FLAG: ");
    Serial.println(RTC_SLOW_MEM[RTC_MEM_FLAG] & 0xFFFF, BIN);

    LOG_NOTICE("ULP", "CNT-0:" << (RTC_SLOW_MEM[RTC_MEM_ADC_CH0_RAW] & 0xFFFF) << "\tCount_0: " << (RTC_SLOW_MEM[RTC_MEM_ADC_CH0] & 0xFFFF));
    LOG_NOTICE("ULP", "CNT-1:" << (RTC_SLOW_MEM[RTC_MEM_ADC_CH1_RAW] & 0xFFFF) << "\tCount_1: " << (RTC_SLOW_MEM[RTC_MEM_ADC_CH1] & 0xFFFF));

    /*
        Checks state button if wake up reason doesn't equal to ESP_SLEEP_WAKEUP_TIMER
        If ESP_SLEEP_WAKEUP_TIMER set mode is TRANSMIT_MODE
    */
    ButtonState_e btnSetupState = ButtonState_e::NO_PRESS;
    if (reason == ESP_SLEEP_WAKEUP_TIMER)
    {
        ledBuiltIn.LEDTurnOn();
        delay(1000);
        ledBuiltIn.LEDTurnOff();
        mode = mode_e::TRANSMIT_MODE;
    }
    else
    {
        LOG_DEBUG("BTN", "Waiting button state ...");
        btnSetupState = btnSetup.Wait_button_release();
        LOG_DEBUG("BTN", "Button state: " << btnSetupState);
    }

    /*
        NO_PRESS        0: just deep sleep again; LED - turn off
        SHORT_PRESS     1: run sending data; LED - turn on and blink once
        LONG_PRESS      2: run setting mode (WiFi Manager); LED - turn on and blink twice
        VERY_LONG_PRESS 3: run clear mode; LED - turn on and blink three times
    */

    switch (btnSetupState)
    {
    case ButtonState_e::SHORT_PRESS:
        LOG_NOTICE("BTN", "SHORT_PRESS");
        mode = mode_e::TRANSMIT_MODE;
        break;
    case ButtonState_e::LONG_PRESS:
        LOG_NOTICE("BTN", "LONG_PRESS");
        mode = mode_e::SETUP_MODE;
        break;
    case ButtonState_e::VERY_LONG_PRESS:
        LOG_NOTICE("BTN", "VERY_LONG_PRESS");
        mode = mode_e::CLEAR_MODE;
        break;
    case ButtonState_e::NO_PRESS:
        LOG_NOTICE("BTN", "NO_PRESS");
        break;
    default:
        LOG_ERROR("BTN", "Undefined Setup button state");
        break;
    }

    //save collected ULP data from RTC_LOW_MEM to NVS store
    long time_start = millis();
    sync_data(&data);
    long time_finish = millis();
    LOG_DEBUG("ESP", "Time to sync data: " << (time_finish - time_start));

    uint8_t new_crc = 0;
    switch (mode)
    {
    case mode_e::SYNC_MODE:
        LOG_NOTICE("ESP", "SYNC_MODE");
        break;
    case mode_e::WATERLEAK_MODE:
        //The same action as a TRANSMIT_MODE
        //break;
    case mode_e::TRANSMIT_MODE:
        ledBuiltIn.LEDTurnOn();
        
        data.version = 32;
        data.waterSensor = false;
        
        loadConfig(sett);
        calculate_values(sett, &data, &cdata);

        data.WAKE_EVERY_MIN = sett.wake_every_min;

        LOG_NOTICE("ESP", "hostname:" << sett.hostname);
        LOG_NOTICE("ESP", "name:" << sett.name);
        LOG_NOTICE("ESP", "hostname_json:" << sett.hostname_json);
        LOG_NOTICE("ESP", "key:" << sett.key);
        LOG_NOTICE("ESP", "description:" << sett.description);
        LOG_NOTICE("ESP", "channel0_start:" << sett.channel0_start);
        LOG_NOTICE("ESP", "channel1_start:" << sett.channel1_start);
        LOG_NOTICE("ESP", "impulses0_start:" << sett.impulses0_start);
        LOG_NOTICE("ESP", "impulses0_previous:" << sett.impulses0_previous);
        LOG_NOTICE("ESP", "impulses1_start:" << sett.impulses1_start);
        LOG_NOTICE("ESP", "impulses1_previous:" << sett.impulses1_previous);
        LOG_NOTICE("ESP", "sensors:" << sett.sensors);

#ifdef USE_EXT_VCC
        //activating additional power supply
        LOG_NOTICE("ESP", "Activating additional power supply");
        vcc.extVCCOn();
#endif
        data.voltage = vcc.readVCC();
        LOG_NOTICE("PWR", "Power (mV): " << vcc.readVCC());
        //delay(100);
        ledBuiltIn.LEDTurnOff();

        connect_wl();
        if (status_wl() == WL_CONNECTED)
        {
            LOG_NOTICE("WIF", "Connected, got IP address: " << WiFi.localIP().toString());

#ifdef SEND_JSON
            if (send_json(sett, data, cdata))
            {
                LOG_NOTICE("JSN", "send ok");
                // if (IsSetTimer and WakeUpTimer > 0){
                //     LOG_NOTICE("JSN", "Need to set a new timer");
                //     sett.wake_every_min = WakeUpTimer;
                //     LOG_NOTICE("ESP", "Send to Attiny a new timer=" << sett.wake_every_min);
                //     masterI2C.sendTimer(sett.wake_every_min);
                // }
            }
#endif

#ifdef SEND_BLYNK
            if (strlen(sett.key) > 2 && send_blynk(sett, data, cdata))
            {
                LOG_NOTICE("BLK", "send ok");
            }
#endif
        }
        disconnect_wl();
#ifdef SEND_COAP
        if (send_coap(sett, data, cdata))
        {
            LOG_NOTICE("CAP", "send coap ok");
        }else LOG_ERROR("CAP", "Send coap data failed");
#endif
#ifdef USE_EXT_VCC
        LOG_NOTICE("ESP", "Disable additional power supply");
        vcc.extVCCOff();
#endif

        //Сохраним текущие значения в памяти.
        sett.impulses0_previous = data.impulses0;
        sett.impulses1_previous = data.impulses1;
        sett.watersensor_previous = data.waterSensor;

        //caclulate CRC for config
        sett.crc_cfg = 0;
        new_crc = crc_8((unsigned char *)&sett, sizeof(sett));
        sett.crc_cfg = new_crc;
        LOG_NOTICE("CFG", "CRC calculated=" << new_crc << ", CRC Stored=" << sett.crc_cfg);
        storeConfig(sett);
        gpio_set_level(GPIO_NUM_18, LOW);
        break;
    case mode_e::SETUP_MODE:
        LOG_NOTICE("ESP", "SETUP_MODE");
#ifdef USE_EXT_VCC
        //activating additional power supply
        LOG_DEBUG("ESP", "Activating additional power supply");
        vcc.extVCCOn();
#endif
        ledBuiltIn.LEDTurnOn(); //FIXME: led doesn't work if used btnHandlerDown

        loadConfig(sett);

        btnTimeStart = 0; //clear start attachInterruptTime
        btnInterrupt = false;
        attachInterrupt(btnSetup._pin, btnHandlerDown, ONHIGH);
        
        LOG_NOTICE("ESP", "Running wifi setup portal");
        setup_ap(sett, data, cdata);

#ifdef USE_EXT_VCC
        //disable additional power supply
        LOG_DEBUG("ESP", "Disable additional power supply");
        vcc.extVCCOff();
#endif
        ledBuiltIn.LEDTurnOff();
        break;
    case mode_e::CLEAR_MODE:
        LOG_NOTICE("ESP", "CLEAR_MODE");
        LOG_NOTICE("ESP", "Restore factory settings");
        if (clearConfig(sett))
            LOG_NOTICE("ESP", "Config clear OK");
        else
            LOG_ERROR("ESP", "Config clear Failed");

        resetSettings();
        LOG_NOTICE("ESP", "Result: " << nvs_flash_erase());
        esp_restart();
        break;
    case mode_e::UPGRADE_MODE:
        /* code */
        break;
    default:
        LOG_ERROR("ESP", "Unknown mode: " << mode);
        break;
    }

    //delay(1000); // FIXME: this is a work around to fix problem incorrect ADC read value while wakeup from deep sleep
    deepSleep();
}

void loop()
{
    //Will never be used
}