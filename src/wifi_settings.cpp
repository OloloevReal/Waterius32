
#include "wifi_settings.h"
#include "Logging.h"

#include "Blynk/BlynkConfig.h"
#include <IPAddress.h>
#include <EEPROM.h>
#include "utils.h"

/* Очищаем конфигурацию в EEPROM */
bool clearConfig(const Settings &sett){
    EEPROM.begin( sizeof(sett) );
    for (int addr = 0; addr < sizeof(sett); addr++){
        EEPROM.write(addr, 0);
    }

    //To prevent reading CRC = 0
    EEPROM.write(0, 255);
    if (EEPROM.commit())
    {
       return true;
    }
    else
    {
        return false;
    }
}

/* Сохраняем конфигурацию в EEPROM */
void storeConfig(const Settings &sett)
{
    EEPROM.begin( sizeof(sett) );
    EEPROM.put(0, sett);
    
    if (EEPROM.commit())
    {
        LOG_NOTICE("CFG", "Config stored OK");    
    }
    else
    {
        LOG_ERROR("CFG", "Config stored FAILED");
    }
}


/* Загружаем конфигурацию в EEPROM. true - успех. */
bool loadConfig(struct Settings &sett) 
{
    EEPROM.begin( sizeof(sett) );
    EEPROM.get(0, sett);

    //caclulace CRC for config
    uint8_t old_crc = sett.crc_cfg;
    sett.crc_cfg = 0;
    uint8_t new_crc = crc_8((unsigned char*)&sett, sizeof(sett));
    LOG_DEBUG( "CFG", "CRC calculated=" << new_crc << ", CRC Stored=" << old_crc);

    if (old_crc == new_crc)
    {
        LOG_NOTICE("CFG", "CRC ok");

        // Для безопасной работы с буферами,  в библиотеках может не быть нет проверок
        sett.key[KEY_LEN-1] = '\0';
        sett.hostname[HOSTNAME_LEN-1] = '\0';
        sett.email[EMAIL_LEN-1] = '\0';
        sett.email_title[EMAIL_TITLE_LEN-1] = '\0';
        sett.email_template[EMAIL_TEMPLATE_LEN-1] = '\0'; 
        sett.hostname_json[EMAIL_TITLE_LEN-1] = '\0';
        sett.name[HOSTNAME_LEN-1] = '\0';
        sett.sn0[SN_LEN-1] = '\0';
        sett.sn1[SN_LEN-1] = '\0';
        sett.description[EMAIL_TITLE_LEN - 1] = '\0';
       
        LOG_NOTICE( "CFG", "email=" << sett.email  << ", hostname=" << sett.hostname);
        LOG_NOTICE( "CFG", "hostname_json=" << sett.hostname_json);
        LOG_NOTICE( "CFG", "json_tag=" << sett.name);

        // Всегда одно и тоже будет
        LOG_NOTICE( "CFG", "channel0_start=" << sett.channel0_start << ", impulses0_start=" << sett.impulses0_start << ", factor=" << sett.liters_per_impuls );
        LOG_NOTICE( "CFG", "channel1_start=" << sett.channel1_start << ", impulses1_start=" << sett.impulses1_start);
        LOG_NOTICE( "CFG", "sensors=" << sett.sensors);
        
        return true;
    }
    else 
    {    // Конфигурация не была сохранена в EEPROM, инициализируем с нуля
        LOG_WARNING( "CFG", "crc failed=" << sett.crc_cfg );

        // Заполняем нулями всю конфигурацию
        memset(&sett, 0, sizeof(sett));

        sett.version = CURRENT_VERSION;  //для совместимости в будущем
        sett.liters_per_impuls = LITRES_PER_IMPULS_DEFAULT;
        
        String hostname = BLYNK_DEFAULT_DOMAIN;
        strncpy0(sett.hostname, hostname.c_str(), HOSTNAME_LEN);

        String email_title = "Новые показания {DEVICE_NAME}";
        strncpy0(sett.email_title, email_title.c_str(), EMAIL_TITLE_LEN);

        String email_template = "Горячая: {V0}м3, Холодная: {V1}м3<br>За день:<br>Горячая: +{V3}л, Холодная: +{V4}л<br>Напряжение:{V2}В";
        strncpy0(sett.email_template, email_template.c_str(), EMAIL_TEMPLATE_LEN);

        String hostname_json = "https://kopilka.us.to:4400/waterius/";
        strncpy0(sett.hostname_json, hostname_json.c_str(), EMAIL_TITLE_LEN);

        String name = "Waterius_";
        strncpy0(sett.name, name.c_str(), HOSTNAME_LEN);

        strncpy0(sett.sn0, String("0000000").c_str(), SN_LEN);
        strncpy0(sett.sn1, String("0000000").c_str(), SN_LEN);

        sett.impulses0_previous = 0;
        sett.impulses1_previous = 0;
        sett.watersensor_previous = false;

        sett.wake_every_min = 60;
        sett.sensors = 0;

        //caclulace CRC for config
        sett.crc_cfg = 0;
        uint8_t new_crc = crc_8((unsigned char*)&sett, sizeof(sett));
        LOG_NOTICE( "CFG", "CRC calculated=" << new_crc << ", CRC Stored=" << sett.crc_cfg);
        sett.crc_cfg = new_crc;
        
        LOG_NOTICE("CFG", "version=" << sett.version << ", hostname=" << hostname);
        return false;
    }
}



esp_err_t NVSInit(nvs_handle *out_handle){
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        LOG_ERROR("NVS", "ESP_ERR_NVS_NO_FREE_PAGES");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err == ESP_OK){
        err = nvs_open("storage", NVS_READWRITE, out_handle);
        if (err != ESP_OK)
        {
            LOG_ERROR("NVS", "Error opening NVS handle: " << err);
        }
    }
    return err;
}

esp_err_t NVSCommit(nvs_handle handle){
    esp_err_t err = nvs_commit(handle);
    if (err != ESP_OK)
    {
        LOG_ERROR("NVS", "Error commit: " << err);
    }
    return err;
}

void NVSClose(nvs_handle handle){
    nvs_close(handle);
}

         