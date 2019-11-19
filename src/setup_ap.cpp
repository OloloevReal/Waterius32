
#include "setup_ap.h"
#include "Logging.h"
#include "wifi_settings.h"

#include <WiFiClient.h>
#include <EEPROM.h>
#include "utils.h"


#define AP_NAME "Waterius32_" FIRMWARE_VERSION

WiFiManager wm;
WiFiCustomParameter wcp;
WiFiManagerParameter custom_field;

void resetSettings(){
    wm.resetSettings();
}
const char HTTP_FORM_CHECKBOX[] PROGMEM = "<br/><label for='{n}'>{t}</label><input type='checkbox' name='{n}' value='{v}' {c}>";
//FIXME: remove console.log
const char WATERIUS_CALLBACK[] PROGMEM = "<script>\
    let timerId = setTimeout(function run() {\
        const xhr = new XMLHttpRequest();\
        xhr.open('GET', '/states');\
        xhr.send();\
        xhr.onreadystatechange = function (e) {\
            if(xhr.readyState === 4 && xhr.status === 200) {\
                var data = JSON.parse(xhr.responseText);\
                Object.keys(data).forEach(function(key) {\
                    console.log(key + \": \" + data[key]);\
                    document.getElementById(key).innerHTML = data[key];\
                });\
                document.getElementById(\"api_token\").value = data[\"api_token\"];\
            };\
        };\
        timerId = setTimeout(run, 1000);\
    }, 1000);\
</script>";

const char WATERIUS_HANDLER[] PROGMEM = "Получено: \
                                        </br>\
                                            CH0: <a id=\"value0\"></a> имп. \
                                            Counts: <a id=\"count0\"></a>\
                                            State: <a id=\"state0\"></a>\
                                        </br>\
                                            CH1: <a id=\"value1\"></a> имп. \
                                            Counts: <a id=\"count1\"></a>\
                                            State: <a id=\"state1\"></a>\
                                        </br>";

void initParameter(Settings &sett, const SlaveData &data, const CalculatedData &cdata)
{
    wcp.sett_wifi = &sett;
    wcp.data_wifi = data;
    wcp.cdata_wifi = cdata;
    
    wcp.param_key = new WiFiManagerParameter( "key", "Ключ:",  sett.key, KEY_LEN-1);
    wcp.param_hostname = new WiFiManagerParameter( "host", "Адрес сервера:",  sett.hostname, HOSTNAME_LEN-1);
    wcp.param_email = new WiFiManagerParameter( "email", "Адрес эл. почты:",  sett.email, EMAIL_LEN-1);
    wcp.param_email_title = new WiFiManagerParameter( "title", "Заголовок:",  sett.email_title, EMAIL_TITLE_LEN-1);
    wcp.param_email_template = new WiFiManagerParameter( "template", "Тело письма:",  sett.email_template, EMAIL_TEMPLATE_LEN-1);
    wcp.param_hostname_json = new WiFiManagerParameter( "hostname_json", "Адрес сервера для JSON:",  sett.hostname_json, EMAIL_TITLE_LEN-1);
    wcp.param_name = new WiFiManagerParameter( "Name", "Имя устройства:", sett.name, HOSTNAME_LEN-1);
    wcp.param_description = new WiFiManagerParameter( "Description", "Описание:", sett.description, EMAIL_TITLE_LEN-1);
    wcp.param_channel0_start = new FloatParameter( "channel0", "Вход 0 ГВС (м3):",  cdata.channel0);
    wcp.param_sn0 = new WiFiManagerParameter( "sn0", "Серийный номер:",  sett.sn0, SN_LEN-1);
    wcp.param_channel1_start = new FloatParameter( "channel1", "Вход 1 ХВС (м3):",  cdata.channel1);
    wcp.param_sn1 = new WiFiManagerParameter( "sn1", "Серийный номер:",  sett.sn1, SN_LEN-1);
    wcp.param_litres_per_imp = new LongParameter( "factor", "Литров на импульс:",  sett.liters_per_impuls);
    wcp.wake_every_min = new IntParameter( "wake_every_min", "Период отправки, мин:",  sett.wake_every_min);
    wcp.param_coap_hostname = new WiFiManagerParameter( "coap", "COAP сервер:",  sett.coap_hostname, HOSTNAME_LEN-1);

    LOG_NOTICE( "ESP", "Init Key:" << sett.key  << " " << wcp.param_key->getValue());
    LOG_NOTICE( "ESP", "Init host:" << sett.hostname << " " << wcp.param_hostname->getValue());
    LOG_NOTICE( "ESP", "Init email:" << sett.email << " " << wcp.param_email->getValue());
    LOG_NOTICE( "ESP", "Init title:" << sett.email_title << " " << wcp.param_email_title->getValue());
    LOG_NOTICE( "ESP", "Init template:" << sett.email_template << " " << wcp.param_email_template->getValue());
    LOG_NOTICE( "ESP", "Init hostname_json:" << sett.hostname_json << " " << wcp.param_hostname_json->getValue());
    LOG_NOTICE( "ESP", "Init name:" << sett.name << " " << wcp.param_name->getValue());
    LOG_NOTICE( "ESP", "Init description:" << sett.description << " " << wcp.param_description->getValue());
    LOG_NOTICE( "ESP", "Init channel0:" << cdata.channel0 << " " << wcp.param_channel0_start->getValue());
    LOG_NOTICE( "ESP", "Init channel1:" << cdata.channel1 << " " << wcp.param_channel1_start->getValue());
    LOG_NOTICE( "ESP", "Init factor:" << sett.liters_per_impuls << " " << wcp.param_litres_per_imp->getValue());
    LOG_NOTICE( "ESP", "Init coap_hostname:" << sett.coap_hostname << " " << wcp.param_coap_hostname->getValue());
}

void handleRoute(){
  LOG_DEBUG("WiFI", "[HTTP] handle route");
  String msg;
  msg.concat("{");
  msg.concat("\"value0\": ");
  msg.concat(get_CH0());
  msg.concat(", \"value1\": ");
  msg.concat(get_CH1());
  msg.concat(", \"count0\": ");
  msg.concat(get_CH0_count());
  msg.concat(", \"count1\": ");
  msg.concat(get_CH1_count());
  msg.concat(", \"state0\": ");
  msg.concat(get_CH0_state());
  msg.concat(", \"state1\": ");
  msg.concat(get_CH1_state());
  msg.concat("}");
  wm.server->send(200, "text/plain", msg.c_str());
}

void bindServerCallback(){
  wm.server->on("/states", handleRoute);
  // wm.server->on("/info",handleRoute); // can override wm!
}

void connect_wl(){
    LOG_NOTICE("WIF", "Starting Wifi");
    WiFi.mode(WIFI_STA);
    WiFi.begin();

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < ESP_CONNECT_TIMEOUT)
    {
        LOG_NOTICE("WIF", "Wifi status: " << WiFi.status());
        delay(200);
    };
}

void disconnect_wl(){
    LOG_NOTICE("WIF", "Disconnecting Wifi");
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}

wl_status_t status_wl(){
    return WiFi.status();
}

void setup_ap(Settings &sett, const SlaveData &data, const CalculatedData &cdata) 
{
    LOG_NOTICE( "ESP", "I2C-begined: mode SETUP" );

    //std::vector<const char *> menu = {"param","sep", "wifi","sep","exit","erase","restart"};
    std::vector<const char *> menu = {"param","sep", "wifi","sep","exit"};
    wm.setMenu(menu);
    wm.setClass("invert");

    LOG_NOTICE( "AP", "User requested captive portal" );
    
    initParameter(sett, data, cdata);

    WiFiManagerParameter custom_html(WATERIUS_HANDLER);
    WiFiManagerParameter javascript_callback(WATERIUS_CALLBACK);

    wm.setWebServerCallback(bindServerCallback);
    wm.addParameter(&custom_html);
    wm.addParameter(&javascript_callback);

    wm.addParameter( wcp.param_name );
    wm.addParameter( wcp.param_description );
#ifdef SEND_BLYNK
    WiFiManagerParameter blynk_text("<p>Blynk:</p>");
    wm.addParameter( &blynk_text );
    wm.addParameter( wcp.param_key );
    wm.addParameter( wcp.param_hostname );

    WiFiManagerParameter email_text("<p>Email:</p>");
    wm.addParameter( &email_text );
    wm.addParameter( wcp.param_email );
    wm.addParameter( wcp.param_email_title );
    wm.addParameter( wcp.param_email_template );
#endif
    WiFiManagerParameter counters_text("<p>Счетчики:</p>");
    wm.addParameter( &counters_text );
    wm.addParameter( wcp.param_channel0_start );
    wm.addParameter( wcp.param_sn0 );
    wm.addParameter( wcp.param_channel1_start );
    wm.addParameter( wcp.param_sn1 );
    wm.addParameter( wcp.param_litres_per_imp );
#ifdef SEND_JSON
    WiFiManagerParameter json_text("<p>JSON:</p>");
    wm.addParameter( &json_text );
    wm.addParameter( wcp.param_hostname_json );
#endif

#ifdef SEND_COAP
    // WiFiManagerParameter coap_hostname("<p>COAP сервер:</p>");
    // wm.addParameter( &coap_hostname );
    wm.addParameter( wcp.param_coap_hostname );
#endif

#ifdef WS_ENABLED
    String item PROGMEM = "<br/><b>Mark to activate sensor:</b>";
    item += formatCheckbox("WS1", "Sensor 1", "0", (bitRead(wcp.sett_wifi->sensors, 0)==1?true:false));
    item += formatCheckbox("WS2", "Sensor 2", "1", (bitRead(wcp.sett_wifi->sensors, 1)==1?true:false));
    item += formatCheckbox("WS3", "Sensor 3", "2", (bitRead(wcp.sett_wifi->sensors, 2)==1?true:false));
    item += formatCheckbox("WS4", "Sensor 4", "3", (bitRead(wcp.sett_wifi->sensors, 3)==1?true:false));

    new (&custom_field) WiFiManagerParameter(item.c_str()); // custom html input
    
    wm.addParameter(&custom_field);
#endif
    wm.addParameter( wcp.wake_every_min );

    wm.setSaveParamsCallback(saveParamCallback);
    //wm.setConfigPortalBlocking(true);
    wm.setConfigPortalTimeout(300);
    wm.setConnectTimeout(15);
    
    LOG_NOTICE( "AP", "start config portal" );

    // Запуск веб сервера на 192.168.4.1
    wm.startConfigPortal( AP_NAME );   

    // Успешно подключились к Wi-Fi, можно засыпать
    LOG_NOTICE( "AP", "Connected to wifi. Save settings, go to sleep" );
}

String formatCheckbox(const char* name, const char* text, const char* value, const bool checked){
    String item = FPSTR(HTTP_FORM_CHECKBOX);
    item.replace(FPSTR(T_n), name);
    item.replace(FPSTR(T_t), text);
    item.replace(FPSTR(T_v), value);

    if (checked)
        item.replace(FPSTR(T_c), "checked");
    else
        item.replace(FPSTR(T_c), "");

    return item;
}

void stop_ap(){
    wm.stopConfigPortal();
    LOG_NOTICE( "AP", "stop config portal" );
}

void saveParamCallback()
{
    LOG_NOTICE( "CFG", "saveParamCallback");

    //wm.stopConfigPortal();

       // Переписываем введенные пользователем значения в Конфигурацию
    strncpy0(wcp.sett_wifi->key, wcp.param_key->getValue(), KEY_LEN);
    strncpy0(wcp.sett_wifi->hostname, wcp.param_hostname->getValue(), HOSTNAME_LEN);
    strncpy0(wcp.sett_wifi->email, wcp.param_email->getValue(), EMAIL_LEN);
    strncpy0(wcp.sett_wifi->email_title, wcp.param_email_title->getValue(), EMAIL_TITLE_LEN);
    strncpy0(wcp.sett_wifi->email_template, wcp.param_email_template->getValue(), EMAIL_TEMPLATE_LEN);

    // JSON
    strncpy0(wcp.sett_wifi->hostname_json, wcp.param_hostname_json->getValue(), EMAIL_TITLE_LEN);
    strncpy0(wcp.sett_wifi->name, wcp.param_name->getValue(), HOSTNAME_LEN);
    strncpy0(wcp.sett_wifi->description, wcp.param_description->getValue(), EMAIL_TITLE_LEN);

    // Текущие показания счетчиков
    wcp.sett_wifi->channel0_start = wcp.param_channel0_start->getValue();
    wcp.sett_wifi->channel1_start = wcp.param_channel1_start->getValue();

    // Серийные номер
    strncpy0(wcp.sett_wifi->sn0, wcp.param_sn0->getValue(), SN_LEN);
    strncpy0(wcp.sett_wifi->sn1, wcp.param_sn1->getValue(), SN_LEN);
    
    // Предыдущие показания счетчиков. Вносим текущие значения.
    wcp.sett_wifi->impulses0_previous = wcp.data_wifi.impulses0;
    wcp.sett_wifi->impulses1_previous = wcp.data_wifi.impulses1;

    wcp.sett_wifi->liters_per_impuls = wcp.param_litres_per_imp->getValue();

    // Запоминаем кол-во импульсов Attiny соответствующих текущим показаниям счетчиков
    wcp.sett_wifi->impulses0_start = wcp.data_wifi.impulses0;
    wcp.sett_wifi->impulses1_start = wcp.data_wifi.impulses1;

    LOG_NOTICE( "AP", "impulses0=" << wcp.sett_wifi->impulses0_start );
    LOG_NOTICE( "AP", "impulses1=" << wcp.sett_wifi->impulses1_start );

    wcp.sett_wifi->wake_every_min = wcp.wake_every_min->getValue();
    LOG_NOTICE( "AP", "wake_every_min=" << wcp.sett_wifi->wake_every_min );

    strncpy0(wcp.sett_wifi->coap_hostname, wcp.param_coap_hostname->getValue(), HOSTNAME_LEN);

#ifdef WS_ENABLED
    uint8_t sensors = 0;
    if (getParam("WS1").toInt() > 0){
        bitSet(sensors, 0);
    }
    if (getParam("WS2").toInt() > 0){
        bitSet(sensors, 1);
    }
    if (getParam("WS3").toInt() > 0){
        bitSet(sensors, 2);
    }
    if (getParam("WS4").toInt() > 0){
        bitSet(sensors, 3);
    }
    LOG_DEBUG("CFG", "Sensors=" << sensors);
    wcp.sett_wifi->sensors = sensors;
#endif

    //caclulace CRC for config
    wcp.sett_wifi->crc_cfg = 0;
    uint8_t new_crc = crc_8((unsigned char*)wcp.sett_wifi, sizeof(*wcp.sett_wifi));
    wcp.sett_wifi->crc_cfg = new_crc;
    LOG_NOTICE( "CFG", "CRC calculated=" << new_crc << ", CRC Stored=" << wcp.sett_wifi->crc_cfg);

    storeConfig(*wcp.sett_wifi);
}

String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}
