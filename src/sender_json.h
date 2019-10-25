#ifndef _SENDERJSON_h
#define _SENDERJSON_h

#include "Logging.h"
#include <ArduinoJson.h>
//#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>


#ifdef SEND_JSON

WiFiClientSecure client;
HTTPClient http;

uint8_t     UpdateCheck = 0;
String      UpdateURL;
bool IsSetTimer = false;
uint16_t WakeUpTimer = 0;

static const char digicert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFjTCCA3WgAwIBAgIRANOxciY0IzLc9AUoUSrsnGowDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTYxMDA2MTU0MzU1
WhcNMjExMDA2MTU0MzU1WjBKMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDEjMCEGA1UEAxMaTGV0J3MgRW5jcnlwdCBBdXRob3JpdHkgWDMwggEi
MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCc0wzwWuUuR7dyXTeDs2hjMOrX
NSYZJeG9vjXxcJIvt7hLQQWrqZ41CFjssSrEaIcLo+N15Obzp2JxunmBYB/XkZqf
89B4Z3HIaQ6Vkc/+5pnpYDxIzH7KTXcSJJ1HG1rrueweNwAcnKx7pwXqzkrrvUHl
Npi5y/1tPJZo3yMqQpAMhnRnyH+lmrhSYRQTP2XpgofL2/oOVvaGifOFP5eGr7Dc
Gu9rDZUWfcQroGWymQQ2dYBrrErzG5BJeC+ilk8qICUpBMZ0wNAxzY8xOJUWuqgz
uEPxsR/DMH+ieTETPS02+OP88jNquTkxxa/EjQ0dZBYzqvqEKbbUC8DYfcOTAgMB
AAGjggFnMIIBYzAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADBU
BgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEBATAwMC4GCCsGAQUFBwIB
FiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQub3JnMB0GA1UdDgQWBBSo
SmpjBH3duubRObemRWXv86jsoTAzBgNVHR8ELDAqMCigJqAkhiJodHRwOi8vY3Js
LnJvb3QteDEubGV0c2VuY3J5cHQub3JnMHIGCCsGAQUFBwEBBGYwZDAwBggrBgEF
BQcwAYYkaHR0cDovL29jc3Aucm9vdC14MS5sZXRzZW5jcnlwdC5vcmcvMDAGCCsG
AQUFBzAChiRodHRwOi8vY2VydC5yb290LXgxLmxldHNlbmNyeXB0Lm9yZy8wHwYD
VR0jBBgwFoAUebRZ5nu25eQBc4AIiMgaWPbpm24wDQYJKoZIhvcNAQELBQADggIB
ABnPdSA0LTqmRf/Q1eaM2jLonG4bQdEnqOJQ8nCqxOeTRrToEKtwT++36gTSlBGx
A/5dut82jJQ2jxN8RI8L9QFXrWi4xXnA2EqA10yjHiR6H9cj6MFiOnb5In1eWsRM
UM2v3e9tNsCAgBukPHAg1lQh07rvFKm/Bz9BCjaxorALINUfZ9DD64j2igLIxle2
DPxW8dI/F2loHMjXZjqG8RkqZUdoxtID5+90FgsGIfkMpqgRS05f4zPbCEHqCXl1
eO5HyELTgcVlLXXQDgAWnRzut1hFJeczY1tjQQno6f6s+nMydLN26WuU4s3UYvOu
OsUxRlJu7TSRHqDC3lSE5XggVkzdaPkuKGQbGpny+01/47hfXXNB7HntWNZ6N2Vw
p7G6OfY+YQrZwIaQmhrIqJZuigsrbe3W+gdn5ykE9+Ky0VgVUsfxo52mwFYs1JKY
2PGDuWx8M6DlS6qQkvHaRUo0FMd8TsSlbF0/v965qGFKhSDeQoMpYnwcmQilRh/0
ayLThlHLN81gSkJjVrPI0Y8xCVPB4twb1PFUd2fPM3sA1tJ83sZ5v8vgFv2yofKR
PB0t6JzUA81mSqM3kxl5e+IZwhYAyO0OTg3/fs8HqGTNKd9BqoUwSRBzp06JMg5b
rUCGwbCUDI0mxadJ3Bz4WxR6fyNpBK2yAinWEsikxqEt
-----END CERTIFICATE-----
)EOF";

// Set time via NTP, as required for x.509 validation
bool setClock() 
{
	configTime(3 * 3600, 0, "ru.pool.ntp.org", "time.nist.gov");

	LOG_NOTICE("NTP", "Waiting for NTP time sync: ");
	uint32_t start = millis();
	time_t now = time(nullptr);
	uint8_t cnt = 0;
	
	while (now < 8 * 3600 * 2  && millis() - start < ESP_CONNECT_TIMEOUT) {
		delay(500);
		now = time(nullptr);
		if (cnt < 6){
			cnt++;		
		}else{
			cnt = 0;
			LOG_NOTICE("NTP", "One else attempt to sync time");
			configTime(3 * 3600, 0, "ru.pool.ntp.org", "time.nist.gov");
		}
	}
	
	if (millis() - start >= ESP_CONNECT_TIMEOUT) {
		return false;
	}

	struct tm timeinfo;
	gmtime_r(&now, &timeinfo);
	LOG_NOTICE("NTP", "Current time: " << asctime(&timeinfo));
	return true;
}

/*
Функция отправляющая данные в JSON на сервер.
URL HTTP сервера: sett.hostname_json
*/
bool send_json(const Settings &sett, const SlaveData &data, const CalculatedData &cdata)
{
    bool connect = false;
    IsSetTimer = false;
    WakeUpTimer = 0;

	client.setCACert(digicert);


    setClock(); //Set daytime using SNTP for SSL purpose

    //http.begin()
	
    if (strlen(sett.hostname_json)> 4)
    {
        LOG_NOTICE("JSN", "Connection to: " << sett.hostname_json);
        String url = sett.hostname_json;
        size_t found = url.indexOf(":");
        String protocol=url.substring(0,found); 

        String url_new= url.substring(found+3); //url_new is the url excluding the http part
        size_t found1 = url_new.indexOf(":");
        String host = url_new.substring(0,found1);

        size_t found2 = url_new.indexOf("/");
        String port = url_new.substring(found1+1,found2);
        String path = url_new.substring(found2);

        LOG_NOTICE("JSN", "Protocol: " << protocol);
        LOG_NOTICE("JSN", "Host: " << host);
        LOG_NOTICE("JSN", "Port: " << port);
        LOG_NOTICE("JSN", "Path: " << path);
		//LOG_DEBUG("JSN", "SSL: " << client.connect(host.c_str(), port.toInt()));
        http.setTimeout(SERVER_TIMEOUT);
        connect = http.begin(dynamic_cast<WiFiClientSecure&>(client), url + "/data");
        if (connect) {

            http.addHeader("Host", host);
            http.addHeader("Content-Type", "application/json"); 
            http.addHeader("Connection", "close");
            http.addHeader("Action", "data");
            http.addHeader("ID", String(getChipId()));

            LOG_INFO("JSN", "Delta0:" << cdata.delta0);
            LOG_INFO("JSN", "Delta1:" << cdata.delta1);

			StaticJsonBuffer<1000> jsonBuffer;            
			JsonObject& root = jsonBuffer.createObject();
            root["id"] = getChipId();
            root["key"] = sett.key;
            root["tag"] = sett.name;
            root["description"] = sett.description;
            root["version"] = data.version;
            root["version_esp"] = FIRMWARE_VERSION;
            root["boot"] = data.service;  // 2 - reset pin, 3 - power
            root["resets"] = data.resets;
            root["voltage"] = (float)(data.voltage / 1000.0);
            root["good"] = data.diagnostic;
            root["ch0"] = cdata.channel0;
            root["ch1"] = cdata.channel1;
            root["delta0"] = cdata.delta0;
            root["delta1"] = cdata.delta1;
            root["rssi"] = WiFi.RSSI();

            root["timer"] = data.WAKE_EVERY_MIN;
            root["memoryFree"] = data.memoryFree;
            root["waterSensor"] = data.waterSensor;

            //root["ESPResetReason"] = ESP.getResetReason();
            root["ESPFreeHeap"] = ESP.getFreeHeap();
            root["sn0"] = sett.sn0;
            root["sn1"] = sett.sn1;
            

            String output;
            root.printTo(output);
            LOG_NOTICE("JSN", output);

            int httpCode = http.POST(output);   //Send the request
            
            if (httpCode == 200) {
                LOG_NOTICE("JSN", httpCode);
                //Get the response payload
                String payload = http.getString();
                jsonBuffer.clear();
                JsonObject& root = jsonBuffer.parseObject(payload);
                UpdateCheck = 0;
                UpdateURL = "";
                if (root.success()){
                    LOG_NOTICE("JSN", "JSON parsed success, key/value: " << root.size()); 
                    output = "";
                    root.printTo(output);
                    LOG_NOTICE("JSN", "Response:" << output);
                    UpdateCheck = root.get<uint8_t>("checkUpdate");
                    UpdateURL = root.get<String>("UpdateURL");
                    IsSetTimer = root.get<bool>("IsSetTimer");
                    WakeUpTimer = root.get<uint16_t>("WakeUpTimer");

                }else{
                    LOG_ERROR("JSN", "parsing JSON response failed, " << root.success());
                }
            } else {
                LOG_ERROR("JSN", httpCode);
            }
            
            //LOG_NOTICE("JSN", payload);
            http.end();  //Close connection
            return true;
            
        } else {
            LOG_ERROR("JSN", "Connection to server failed");
        }
    }
      
    return false;
}

// bool send_json_Update(const Settings &sett, const SlaveData &data){
//     if (strlen(sett.hostname_json)> 4){

//         BearSSL::WiFiClientSecure client;
//         BearSSL::X509List cert;
//         cert.append(digicert);

//         setClock(); //Set daytime using SNTP for SSL purpose

//         client.setTrustAnchors(&cert);

//         LOG_NOTICE("JSN", "Connection to: " << sett.hostname_json);
//         String url = sett.hostname_json;
//         size_t found = url.indexOf(":");
//         String protocol=url.substring(0,found); 

//         String url_new=url.substring(found+3); //url_new is the url excluding the http part
//         size_t found1 =url_new.indexOf(":");
//         String host =url_new.substring(0,found1);

//         size_t found2 = url_new.indexOf("/");
//         String port = url_new.substring(found1+1, found2);
//         String path = url_new.substring(found2);

//         LOG_NOTICE("JSN", "Protocol: " << protocol);
//         LOG_NOTICE("JSN", "Host: " << host);
//         LOG_NOTICE("JSN", "Port: " << port);
//         LOG_NOTICE("JSN", "Path: " << path);

//         http.setTimeout(SERVER_TIMEOUT);
//         bool connect = http.begin(dynamic_cast<WiFiClient&>(client), url + "/data");
//         if (connect) {
//             http.addHeader("Host", host);
//             http.addHeader("Content-Type", "application/json"); 
//             http.addHeader("Connection", "close");
//             http.addHeader("Action", "update");
//             http.addHeader("ID", String(system_get_chip_id()));


//             StaticJsonBuffer<50> jsonBuffer; 
//             JsonObject& root = jsonBuffer.createObject();
//             root["id"] = system_get_chip_id();
//             root["name"] = sett.name;
//             root["version"] = data.version;
//             root["version_esp"] = FIRMWARE_VERSION;

//             String output;
//             root.printTo(output);

//             int httpCode = http.POST(output);

//             if (httpCode == 200) {
//                 LOG_NOTICE("JSN", "Response code:" << httpCode);
//                 return true;
//             } else {
//                 LOG_ERROR("JSN", "Response code:" << httpCode);
//                 return false;
//             }

//         }else{
//             LOG_ERROR("JSN", "Connection to server failed");
//             return false;
//         }

//     }else{
//         return false;
//     }
    
// }

#endif  // SEND_JSON
#endif
