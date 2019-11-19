#ifndef _SENDER_BLYNK_NBIOT_h
#define _SENDER_BLYNK_NBIOT_h

#if defined(NBIOT) && defined(SEND_BLYNK)

#define TINY_GSM_MODEM_SIM7020E
//#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
//#define TINY_GSM_DEBUG BLYNK_PRINT
#define TINY_GSM_DEBUG Serial

#include <Arduino.h>
#include "Logging.h"

#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>

#ifndef SerialAT
    #define SerialAT Serial2
#endif
TinyGsm modem(SerialAT);

void modemOpen(){
    LOG_INFO("BLK", "Switch on power SIM7020E");
    gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_18, HIGH);
    delay(800);
    gpio_set_level(GPIO_NUM_18, LOW);
    delay(1000);
    SerialAT.begin(115200);
}

void modemClosed(){
    LOG_INFO("BLK", "Switch off power SIM7020E");
    gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_18, HIGH);
    delay(1000);
    gpio_set_level(GPIO_NUM_18, LOW);
    SerialAT.flush();
}

bool connectNetwork(TinyGsm& gsm, const char* apn, const char* user, const char* pass){
    BLYNK_LOG1(BLYNK_F("Modem init..."));
    if (!gsm.begin()) {
        BLYNK_F("Cannot init");
        return false;
    }

    switch (gsm.getSimStatus()) {
    case SIM_ERROR:  BLYNK_LOG1(BLYNK_F("SIM is missing")); return false;
    case SIM_LOCKED: BLYNK_LOG1(BLYNK_F("SIM is PIN-locked")); return false;
    default: break;
    }

    BLYNK_LOG1(BLYNK_F("Connecting to network..."));
    if (gsm.waitForNetwork()) {
        String op = gsm.getOperator();
        BLYNK_LOG2(BLYNK_F("Network: "), op);
    } else {
        BLYNK_LOG1(BLYNK_F("Register in network failed"));
        return false;
    }

    BLYNK_LOG3(BLYNK_F("Connecting to "), apn, BLYNK_F(" ..."));
    if (!gsm.gprsConnect(apn, user, pass)) {
        BLYNK_LOG1(BLYNK_F("Connect GPRS failed"));
        return false;
    }

    BLYNK_LOG1(BLYNK_F("Connected to GPRS"));
    return true;
}

bool send_blynk(const Settings &sett, const SlaveData &data, const CalculatedData &cdata)
{
    modemOpen();
    //modem.radioOff();
    modem.radioOn();
    delay(1000);

    LOG_NOTICE("BLK", F("Modem batt voltage: ") << modem.getBattVoltage());

    uint8_t conn_trys = 2;
    Blynk.config(modem, sett.key, "139.59.206.133");
    unsigned long started = millis();
    while(!connectNetwork(modem, NULL, NULL, NULL) && 
        (millis() - started < SERVER_TIMEOUT * 10) &&
        conn_trys-- > 0){
        LOG_DEBUG("BLK", F("Radio turn off"));
        modem.radioOff();
        LOG_DEBUG("BLK", F("Radio turn on"));
        modem.radioOn();
    }

    //LOG_DEBUG("BLK", F("Query DNS: ") << modem.queryDNS("blynk-cloud.com"));

    if (modem.isPDPConnected() && Blynk.connect(SERVER_TIMEOUT * 20)) {
        LOG_NOTICE( "BLK", "run");

        LOG_NOTICE( "BLK", "delta0: " << cdata.delta0);
        LOG_NOTICE( "BLK", "delta1: " << cdata.delta1);

        Blynk.virtualWrite(V0, cdata.channel0);
        Blynk.virtualWrite(V1, cdata.channel1);
        Blynk.virtualWrite(V2, (float)(data.voltage / 1000.0));
        Blynk.virtualWrite(V3, cdata.delta0);
        Blynk.virtualWrite(V4, cdata.delta1);
        Blynk.virtualWrite(V5, data.resets);

        if(data.waterSensor && !sett.watersensor_previous){
            Blynk.notify("Waterleak sensor: Alarm!!!\r\n" + String(sett.name) + ": " + String(sett.description));
        }else if (!data.waterSensor && sett.watersensor_previous)
        {
            Blynk.notify("Waterleak sensor: Normal\r\n" + String(sett.name) + ": " + String(sett.description));
        }
        

        LOG_NOTICE( "BLK", "virtualWrite OK");
        
        // Если заполнен параметр email отправим эл. письмо
        if (strlen(sett.email) > 4) {
            LOG_NOTICE( "BLK", "send email: " << sett.email);

            String msg = sett.email_template;
            String title = sett.email_title;
            String v0(cdata.channel0, 1);   //.1 для образца СМС сообщения
            String v1(cdata.channel1, 1);   //.1 для образца СМС сообщения
            String v2((float)(data.voltage / 1000.0), 3);
            String v3(cdata.delta0, DEC);
            String v4(cdata.delta1, DEC);
            String v5(data.resets, DEC);
            
            msg.replace("{V0}", v0);
            msg.replace("{V1}", v1);
            msg.replace("{V2}", v2);
            msg.replace("{V3}", v3);
            msg.replace("{V4}", v4);
            msg.replace("{V5}", v5);
            
            title.replace("{V0}", v0);
            title.replace("{V1}", v1);
            title.replace("{V2}", v2);
            title.replace("{V3}", v3);
            title.replace("{V4}", v4);
            title.replace("{V5}", v5);

            // Blynk.email(sett.email, title, msg);

            // LOG_NOTICE("BLK", "email was send");
            // LOG_NOTICE("BLK", title);
            // LOG_NOTICE("BLK", msg);
        }

        Blynk.disconnect();
        LOG_NOTICE("BLK", "disconnected");
        modem.radioOff();
        modem.sendAT("&W");
        modemClosed();
        return true;
    } else {
        LOG_ERROR("BLK", "connect error");
    }
    modem.radioOff();
    modem.sendAT("&W");
    modemClosed();
    return false;
}

#endif
#endif