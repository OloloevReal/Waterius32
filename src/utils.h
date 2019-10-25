#ifndef _UTILS_h
#define _UTILS_h

#include <Arduino.h>
#include "Logging.h"
#include <rom/rtc.h>
#include "string.h"
#include <driver/adc.h>
#include <esp32/ulp.h>
#include <driver/rtc_io.h>


#define		CRC_START_8		0x00

const uint8_t sht75_crc_table[] PROGMEM = {

	0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,
	67,  114, 33,  16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109,
	134, 183, 228, 213, 66,  115, 32,  17,  63,  14,  93,  108, 251, 202, 153, 168,
	197, 244, 167, 150, 1,   48,  99,  82,  124, 77,  30,  47,  184, 137, 218, 235,
	61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215, 64,  113, 34,  19,
	126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,  80,
	187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149,
	248, 201, 154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214,
	122, 75,  24,  41,  190, 143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,
	57,  8,   91,  106, 253, 204, 159, 174, 128, 177, 226, 211, 68,  117, 38,  23,
	252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,  22,  129, 176, 227, 210,
	191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243, 160, 145,
	71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105,
	4,   53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,
	193, 240, 163, 146, 5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239,
	130, 179, 224, 209, 70,  119, 36,  21,  59,  10,  89,  104, 255, 206, 157, 172
};

/*
Запишем 0 в конце буфера принудительно.
*/
inline void strncpy0(char *dest, const char *src, const size_t len)
{   
    strncpy(dest, src, len-1);
    dest[len-1] = '\0';
}
        

inline uint8_t crc_8(const unsigned char *input_str, size_t num_bytes) {

	size_t a;
	uint8_t crc;
	const unsigned char *ptr;

	crc = CRC_START_8;
	ptr = input_str;

	if (ptr != NULL) for (a = 0; a < num_bytes; a++) {
		//crc = sht75_crc_table[(*ptr++) ^ crc];
		crc = pgm_read_byte(&(sht75_crc_table[(*ptr++) ^ crc]));
	}

	return crc;
}

//Get ESP32 Chip ID in ESP8266 style - last three words of MAC
uint32_t getChipId();

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
esp_sleep_wakeup_cause_t print_wakeup_reason();

void print_reset_reason(RESET_REASON reason);

typedef struct {
  uint32_t pullup;    /*!< shift pullup enable */
  uint32_t pulldown;  /*!< shift pulldown enable */
} rtc_gpio_desc2_t;

const rtc_gpio_desc2_t rtc_gpio_desc2[GPIO_PIN_COUNT] = {
  {RTC_IO_TOUCH_PAD1_RUE_S, RTC_IO_TOUCH_PAD1_RDE_S}, //0
  {0, 0}, //1
  {RTC_IO_TOUCH_PAD2_RUE_S, RTC_IO_TOUCH_PAD2_RDE_S}, //2
  {0, 0}, //3
  {RTC_IO_TOUCH_PAD0_RUE_S, RTC_IO_TOUCH_PAD0_RDE_S}, //4
  {0, 0}, //5
  {0, 0}, //6
  {0, 0}, //7
  {0, 0}, //8
  {0, 0}, //9
  {0, 0}, //10
  {0, 0}, //11
  {RTC_IO_TOUCH_PAD5_RUE_S, RTC_IO_TOUCH_PAD5_RDE_S}, //12
  {RTC_IO_TOUCH_PAD4_RUE_S, RTC_IO_TOUCH_PAD4_RDE_S}, //13
  {RTC_IO_TOUCH_PAD6_RUE_S, RTC_IO_TOUCH_PAD6_RDE_S}, //14
  {RTC_IO_TOUCH_PAD3_RUE_S, RTC_IO_TOUCH_PAD3_RDE_S}, //15
  {0, 0}, //16
  {0, 0}, //17
  {0, 0}, //18
  {0, 0}, //19
  {0, 0}, //20
  {0, 0}, //21
  {0, 0}, //22
  {0, 0}, //23
  {0, 0}, //24
  {RTC_IO_PDAC1_RUE_S, RTC_IO_PDAC1_RDE_S}, //25
  {RTC_IO_PDAC2_RUE_S, RTC_IO_PDAC2_RDE_S}, //26
  {RTC_IO_TOUCH_PAD7_RUE_S, RTC_IO_TOUCH_PAD7_RDE_S}, //27
  {0, 0}, //28
  {0, 0}, //29
  {0, 0}, //30
  {0, 0}, //31
  {RTC_IO_X32P_RUE_S, RTC_IO_X32P_RDE_S}, //32
  {RTC_IO_X32N_RUE_S, RTC_IO_X32N_RDE_S}, //33
  {0, 0}, //34
  {0, 0}, //35
  {0, 0}, //36
  {0, 0}, //37
  {0, 0}, //38
  {0, 0}, //39
};

esp_err_t io_num_get_adc1(gpio_num_t gpio_num, adc1_channel_t *channel);
esp_err_t io_num_get_adc_unit(gpio_num_t gpio_num, adc_unit_t *unit);
esp_err_t io_num_get_adc_channel(gpio_num_t gpio_num, adc_channel_t *channel);

uint16_t get_CH0();
uint16_t get_CH1();
uint16_t get_CH0_count();
uint16_t get_CH1_count();
uint16_t get_CH0_state();
uint16_t get_CH1_state();

void readVCC();

#endif