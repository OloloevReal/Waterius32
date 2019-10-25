#ifndef _WIFI_h
#define _WIFI_h

#include "setup.h"

#include <Arduino.h>
#include <WiFiClient.h>
#include "nvs.h"
#include "nvs_flash.h"

/*
Сохраняем конфигурацию в EEPROM
*/
void storeConfig(const Settings &sett);

/*
Читаем конфигурацию из EEPROM
*/
bool loadConfig(Settings &sett);

/*
Очищаем конфигурацию в EEPROM
*/
bool clearConfig(const Settings &sett);

esp_err_t NVSInit(nvs_handle *out_handle);
esp_err_t NVSCommit(nvs_handle handle);
void NVSClose(nvs_handle handle);


#endif

