#ifndef _SENDERCOAP_h
#define _SENDERCOAP_h

#if defined(SEND_COAP)
const char hash[]  PROGMEM = "ecd71870d1963316a97e3ac3408c9835ad8cf0f3c1bc703527c30265534f75ae";
const char S_IOT[] PROGMEM = "IOT";
const char S_CAP[] PROGMEM = "CAP";
const char S_JSN[] PROGMEM = "JSN";

 #ifdef __cplusplus
  extern "C" {
 #endif
 
  uint8_t temprature_sens_read();
 
#ifdef __cplusplus
}
#endif
 
uint8_t temprature_sens_read();

#if defined(WIFI)
#include "Logging.h"
#include <ArduinoJson.h>
#include <WiFiUDP.h>
#include "TinyCoapProto.h"

Coap CoapMessage;
WiFiUDP udp;

uint8_t buffer[BUF_MAX_SIZE];

int8_t udp_send(const char *host, uint16_t port, CoapPacket &Packet){
    udp.beginPacket(host, port);
    IPAddress ip = udp.remoteIP();
    int len = Packet.ToArray(buffer);
    udp.write(buffer, len);
    return udp.endPacket();
}

bool udp_receive(CoapPacket &cp, unsigned long udp_timeout_ms = 2000UL){   
    bool received = false;
    int receivedLen = -1;
    unsigned long started = millis();
    while(!received && (millis() - started) < udp_timeout_ms){
        int packetLen = udp.parsePacket();
        if (packetLen > 0){
            receivedLen = udp.read(buffer, packetLen >= BUF_MAX_SIZE?BUF_MAX_SIZE:packetLen);
            received = true;
        }
    }

    if(received&&receivedLen > 0){
        return CoapMessage.parsePackets(buffer, receivedLen, cp);
    }
    return false;
}

bool send_coap(const Settings &sett, const SlaveData &data, const CalculatedData &cdata){
    if (strlen(sett.coap_hostname)> 4){
        LOG_NOTICE("CAP", "Connection to: " << sett.coap_hostname);
        String url = sett.coap_hostname;
        size_t found = url.indexOf(":");
        String protocol=url.substring(0,found); 

        String url_new= url.substring(found+3); //url_new is the url excluding the http part
        size_t found1 = url_new.indexOf(":");
        String host = url_new.substring(0,found1);

        size_t found2 = url_new.indexOf("/");
        String port = url_new.substring(found1+1,found2);
        String path = url_new.substring(found2);

        LOG_DEBUG("CAP", "Protocol: " << protocol);
        LOG_DEBUG("CAP", "Host: " << host);
        LOG_DEBUG("CAP", "Port: " << port);
        LOG_DEBUG("CAP", "Path: " << path);

        LOG_INFO("CAP", "Delta0:" << cdata.delta0);
        LOG_INFO("CAP", "Delta1:" << cdata.delta1);

        StaticJsonBuffer<1000> jsonBuffer;            
        JsonObject& root = jsonBuffer.createObject();
        root["id"] = getChipId();
        root["ky"] = sett.key;
        root["tg"] = sett.name;
        root["dp"] = sett.description;
        root["vs"] = data.version;
        root["ve"] = FIRMWARE_VERSION;
        root["bt"] = data.service;  // 2 - reset pin, 3 - power
        root["rs"] = data.resets;
        root["vt"] = (float)(data.voltage / 1000.0);
        root["gd"] = data.diagnostic;
        root["c0"] = cdata.channel0;
        root["c1"] = cdata.channel1;
        root["d0"] = cdata.delta0;
        root["d1"] = cdata.delta1;
        root["ri"] = WiFi.RSSI();

        root["tr"] = data.WAKE_EVERY_MIN;
        root["mf"] = data.memoryFree;
        root["ws"] = data.waterSensor;

        //root["ESPResetReason"] = ESP.getResetReason();
        root["mf"] = ESP.getFreeHeap();
        root["s0"] = sett.sn0;
        root["s1"] = sett.sn1;
        root["tp"] = (temprature_sens_read() - 32) / 1.8;
            
        String output;
        root.printTo(output);
        LOG_DEBUG(FPSTR(S_JSN), output);

        CoapPacket cp;
        CoapMessage.post(host.c_str(), port.toInt(), cp, "data", (char*)output.c_str(), output.length(), COAP_CONTENT_TYPE::COAP_APPLICATION_JSON);
        
        char numstr[21];
        sprintf(numstr, "id=%d", getChipId());
        cp.SetQueryString(numstr);
        
        char query[100];
        sprintf(query, "hash=%s", (PGM_P)FPSTR(hash));
        cp.SetQueryString(query);
        if(udp_send(host.c_str(), port.toInt(), cp)>0){
            LOG_DEBUG("CAP", F("Send COAP message success, message id: ") << cp.messageid);
        }else{
            LOG_ERROR("CAP", F("Send COAP message failed!"));
            return false;
        }

        if(udp_receive(cp)){
            LOG_DEBUG("CAP", F("Received CoAP message, MsgID: ")<< cp.messageid << "\t Code: " << cp.code);
            switch (cp.code)
            {
            case COAP_CHANGED:
                LOG_DEBUG("CAP", F("Send data to COAP server successs!"));
                //break;
                return true;
            case COAP_UNAUTHORIZED:
                LOG_ERROR("CAP", F("COAP_UNAUTHORIZED"));
                return false;
            default:
                LOG_ERROR("CAP", F("COAP code is undefined"));
                return false;
            }
            
        }else{
            LOG_ERROR("CAP", F("Parse received data failed!"));
            return false;
        }
    } else {
        LOG_ERROR("CAP", "Connection to server failed");
    }
    return false;
}

#elif defined(NBIOT)

#include "Logging.h"
#include <ArduinoJson.h>

#define TINY_GSM_MODEM_SIM7020E
#define TINY_GSM_DEBUG Serial
#include "TinyCoap.h"

#include <TinyGsmClient.h>
#ifndef SerialAT
    #define SerialAT Serial2
#endif
TinyGsm modem(SerialAT);
TinyCoap tinyCoap;

void modemOpen(){
    LOG_INFO(FPSTR(S_IOT), F("Switch on power SIM7020E"));
    gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_18, HIGH);
    delay(800);
    gpio_set_level(GPIO_NUM_18, LOW);
    delay(1000);
    SerialAT.begin(115200);
}

void modemClosed(){
    LOG_INFO(FPSTR(S_IOT), F("Switch off power SIM7020E"));
    gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_18, HIGH);
    delay(1000);
    gpio_set_level(GPIO_NUM_18, LOW);
    SerialAT.flush();
}

bool send_coap(const Settings &sett, const SlaveData &data, const CalculatedData &cdata)
{
    bool result = false;
    modemOpen();
    modem.radioOn();
    delay(6000);

    if (strlen(sett.coap_hostname)> 4){
        LOG_INFO(FPSTR(S_IOT), F("Connection to: ") << sett.coap_hostname);
        String url = sett.coap_hostname;
        size_t found = url.indexOf(":");
        String protocol=url.substring(0,found); 

        String url_new= url.substring(found+3);
        size_t found1 = url_new.indexOf(":");
        String host = url_new.substring(0,found1);

        size_t found2 = url_new.indexOf("/");
        String port = url_new.substring(found1+1,found2);
        String path = url_new.substring(found2);

        LOG_DEBUG(FPSTR(S_CAP), F("Protocol: ") << protocol);
        LOG_DEBUG(FPSTR(S_CAP), F("Host: ") << host);
        LOG_DEBUG(FPSTR(S_CAP), F("Port: ") << port);
        LOG_DEBUG(FPSTR(S_CAP), F("Path: ") << path);

        LOG_INFO(FPSTR(S_CAP), F("Delta0: ") << cdata.delta0);
        LOG_INFO(FPSTR(S_CAP), F("Delta1: ") << cdata.delta1);

        LOG_INFO(FPSTR(S_IOT), F("Starting SIM7020E modem, init connection"));

        if(tinyCoap.begin(modem, NULL, NULL, NULL, host.c_str())){
            LOG_INFO(FPSTR(S_IOT), F("[MODEM] Networks state and connection is OK"));
            LOG_INFO(FPSTR(S_IOT), F("[MODEM] IMEI: ") << modem.getIMEI());
            LOG_INFO(FPSTR(S_IOT), F("[MODEM] IMSI: ") << modem.getIMSI());
            LOG_INFO(FPSTR(S_IOT), F("[MODEM] IsGprsConnected: ") << modem.isGprsConnected());
            LOG_INFO(FPSTR(S_IOT), F("[MODEM] IP: ") << modem.localIP().toString());

            StaticJsonBuffer<1000> jsonBuffer;            
            JsonObject& root = jsonBuffer.createObject();
            root["id"] = getChipId();
            root["ky"] = sett.key;
            root["tg"] = sett.name;
            root["dp"] = sett.description;
            root["vs"] = data.version;
            root["ve"] = FIRMWARE_VERSION;
            root["bt"] = data.service;  // 2 - reset pin, 3 - power
            root["rs"] = data.resets;
            root["vt"] = (float)(data.voltage / 1000.0);
            root["gd"] = data.diagnostic;
            root["c0"] = cdata.channel0;
            root["c1"] = cdata.channel1;
            root["d0"] = cdata.delta0;
            root["d1"] = cdata.delta1;
            root["ri"] = modem.getSignalQuality(); //WiFi.RSSI();

            root["tr"] = data.WAKE_EVERY_MIN;
            root["mf"] = data.memoryFree;
            root["ws"] = data.waterSensor;

            //root["ESPResetReason"] = ESP.getResetReason();
            root["mf"] = ESP.getFreeHeap();
            root["s0"] = sett.sn0;
            root["s1"] = sett.sn1;
            root["tp"] = (temprature_sens_read() - 32) / 1.8;
                
            String output;
            root.printTo(output);
            LOG_DEBUG(FPSTR(S_JSN), output);

            char query[100];
            sprintf(query, "id=%d&hash=%s", getChipId(), (PGM_P)FPSTR(hash));

            tinyCoap.setWaitResponse(true);
            result = tinyCoap.post("data", (char*)output.c_str(), output.length(), 
                            query,
                            COAP_CONTENT_TYPE::COAP_APPLICATION_JSON);
            if(result){
                LOG_INFO(FPSTR(S_IOT),F("Send COAP message success!"));
            }else{
                LOG_ERROR(FPSTR(S_IOT),F("Send COAP message failed!"));
            }
        }else{
            LOG_ERROR(FPSTR(S_IOT), F("[MODEM] Networks state or connection is FAILED"));
            result = false;
        }

    }else{
        result = false;
    }

    // modem.radioOff();
    // modem.sendAT("&W");
    modemClosed();
    return result;
}
#endif
#endif
#endif