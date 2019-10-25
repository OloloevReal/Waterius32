#ifndef _SENDERCOAP_h
#define _SENDERCOAP_h

#ifdef SEND_COAP

#include "Logging.h"

#define TINY_GSM_DEBUG Serial
#include "TinyCoap.h"

#define TINY_GSM_MODEM_SIM7020E

#include <TinyGsmClient.h>
#define SerialAT Serial2
TinyGsm modem(SerialAT);

TinyCoap tinyCoap;



bool send_coap(const Settings &sett, const SlaveData &data, const CalculatedData &cdata)
{
    LOG_INFO("IOT", "Switch on power SIM7020E");
    gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_18, HIGH);
    delay(800);
    gpio_set_level(GPIO_NUM_18, LOW);
    delay(2000);
    SerialAT.begin(115200);
    // You should get Auth Token in the Blynk App.
    // Go to the Project Settings (nut icon).
    char auth[] = "";

    // Your GPRS credentials
    // Leave empty, if missing user or pass
    char apn[]  = "";
    char user[] = "";
    char pass[] = "";

    LOG_INFO("IOT", "Starting SIM7020E modem, init connection");
    tinyCoap.begin(auth, modem, apn, user, pass, "kopilka.us.to"); //"95.28.179.153"
    Serial.printf("[MODEM] IMEI: ");
    Serial.println(modem.getIMEI().c_str());
    Serial.printf("[MODEM] IMSI: ");
    Serial.println(modem.getIMSI().c_str());
    Serial.printf("[MODEM] IsGprsConnected: ");
    Serial.println(modem.isGprsConnected());
    Serial.printf("[MODEM] IP: ");
    Serial.println(modem.localIP().toString().c_str());

    if(tinyCoap.ping())
        LOG_NOTICE("CAP", "Send Ping OK");
    else
        LOG_ERROR("CAP", "Send Ping Failed");

    LOG_INFO("IOT", "Switch off power SIM7020E");
    gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_18, HIGH);
    delay(1000);
    gpio_set_level(GPIO_NUM_18, LOW);
    SerialAT.flush();
    return true;
}

#endif
#endif