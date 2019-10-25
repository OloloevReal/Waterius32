#ifndef _SETUPAP_h
#define _SETUPAP_h

#include "setup.h"
#include <Arduino.h>
#include <WiFiManager.h>    

void resetSettings();

/*
Запускаем вебсервер для настройки подключения к Интернету и ввода текущих показаний
*/
void setup_ap(Settings &sett, const SlaveData &data, const CalculatedData &cdata);
void connect_wl();
wl_status_t status_wl();
String formatCheckbox(const char* name, const char* text, const char* value, const bool checked);
void stop_ap();
void saveParamCallback();
/*
Дополнение к WifiManager: классы упрощающие работу
*/

class IPAddressParameter : public WiFiManagerParameter {
public:
    IPAddressParameter():WiFiManagerParameter()
    {}
    IPAddressParameter(const char *id, const char *placeholder, IPAddress address)
        : WiFiManagerParameter("") {
        init(id, placeholder, address.toString().c_str(), 16, "", WFM_LABEL_BEFORE);
    }
    
    bool getValue(IPAddress &ip) {
        return ip.fromString(WiFiManagerParameter::getValue());
    }

    IPAddress getValue() {
        IPAddress ip;
        ip.fromString(WiFiManagerParameter::getValue());
        return ip;
    }
};


class IntParameter : public WiFiManagerParameter {
public:
    IntParameter():WiFiManagerParameter()
    {}
    IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 5)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }
    long getValue() {
        return String(WiFiManagerParameter::getValue()).toInt();
    }
};

class LongParameter : public WiFiManagerParameter {
public:
    LongParameter():WiFiManagerParameter()
    {}
    LongParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }
    long getValue() {
        return String(WiFiManagerParameter::getValue()).toInt();
    }
};

class FloatParameter : public WiFiManagerParameter {
public:
    FloatParameter(): WiFiManagerParameter()
    {}
    FloatParameter(const char *id, const char *placeholder, float value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    float getValue() {
        return String(WiFiManagerParameter::getValue()).toFloat();
    }
};

struct WiFiCustomParameter{
    WiFiManagerParameter *param_key;
    WiFiManagerParameter *param_hostname;
    WiFiManagerParameter *param_email;
    WiFiManagerParameter *param_email_title;
    WiFiManagerParameter *param_email_template;
    WiFiManagerParameter *param_hostname_json;
    WiFiManagerParameter *param_name;
    WiFiManagerParameter *param_description;
    FloatParameter *param_channel0_start;
    FloatParameter *param_channel1_start;
    LongParameter *param_litres_per_imp;
    WiFiManagerParameter *mqtt_host;
    LongParameter *mqtt_port;
    WiFiManagerParameter *mqtt_user;
    WiFiManagerParameter *mqtt_password;
    IntParameter *wake_every_min;
    WiFiManagerParameter *param_sn0;
    WiFiManagerParameter *param_sn1;

    Settings *sett_wifi;
    SlaveData data_wifi;
    float channel0_wifi;
    float channel1_wifi;
};

String getParam(String name);

#endif

