#ifndef _SENDERBLYNK_h
#define _SENDERBLYNK_h

#include <WiFi.h>
#include <BlynkSimpleEsp32_SSL.h>
#include "Logging.h"

#ifdef SEND_BLYNK
bool send_blynk(const Settings &sett, const SlaveData &data, const CalculatedData &cdata)
{
    Blynk.config(sett.key, sett.hostname, BLYNK_DEFAULT_PORT_SSL);
    if (Blynk.connect(SERVER_TIMEOUT)) {
        
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

        return true;
    } else {
        LOG_ERROR("BLK", "connect error");
    } 

    return false;
}        

#endif //#ifdef SEND_BLYNK

#endif
