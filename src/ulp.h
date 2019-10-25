#ifndef _ULP_h
#define _ULP_h

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp32/ulp.h>
#include "Logging.h"
#include "button.h"
#include "counter.h"
#include <ulp_macro.c>

/*
Set #define CONFIG_ULP_COPROC_RESERVE_MEM 1024
in sdkconfig.h
*/

/*
RTC_MEM_FLAG
8 bit 

| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
---------------------------------
| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

0 - counter_0
1 - counter_1
2 - water_sernsor_0
3 - water_sernsor_1
4 - water_sernsor_2
5 - water_sernsor_3
6 - not used
7 - btn
*/

#define I_CLEAR_R() \
  I_MOVI(R0, 0), \
  I_MOVI(R1, 0), \
  I_MOVI(R2, 0), \
  I_MOVI(R3, 0)

// read bit number bit_mask from addr and move to R0
// Using R0
#define BIT_RD(addr, bit_mask) \
  I_LD(R0, addr, 0), \
  I_ANDI(R0, R0, bit_mask&0xFFFF), \
  I_BL(2, 1), \
  I_MOVI(R0, 1)

// Using R1
#define BIT_EN(addr, bit_mask) \
    I_LD(R1, addr, 0), \
    I_ORI(R1, R1, bit_mask&0xFFFF), \
    I_ST(R1, addr, 0)

// Using R1
#define BIT_DIS(addr, bit_mask) \
    I_LD(R1, addr, 0), \
    I_ANDI(R1, R1, ~bit_mask&0xFFFF), \
    I_ST(R1, addr, 0)

// R0 -> [addr] apply bit mask
// Using R0, R1
#define BIT_ST(addr, bit_mask) \
  I_BL(5, 1), \
    BIT_EN(addr, bit_mask), \
  I_BGE(4, 1), \
    BIT_DIS(addr, bit_mask)


/*
#define RTC_MEM_ADC_CH0_RAW       0x700
#define RTC_MEM_ADC_CH0           0x701
#define RTC_MEM_ADC_CH0_CHECKS_1  0x702
#define RTC_MEM_ADC_CH0_CHECKS_2  0x703
*/

#define I_IS_IMPULS(reg_val, addr, bit) \
  I_MOVR(R0, reg_val), \
  I_BGE(26, LIMIT_NAMUR_CLOSED), \
    I_MOVR(R3, R0), \
    I_MOVI(R2, addr + 2), \
    I_LD(R0, R2, 0), \
    I_BGE(3, TRIES1), \
      I_ADDI(R0, R0, 1), \
      I_ST(R0, R2, 0), \
        I_MOVR(R1, R0), \
        I_SUBI(R0, R0, TRIES1), \
        I_BGE(13, 1), \
          I_ADDI(R1, R1, 1), \
          I_ST(R1, R2, 0), \
          I_MOVI(R0, addr + 1), \
          I_LD(R2, R0, 0), \
          I_ADDI(R2, R2, 1), \
          I_ST(R2, R0, 0), \
          I_MOVI(R0, RTC_MEM_FLAG), \
          BIT_EN(R0, bit), \
          I_MOVR(R0, R3), \
    I_BL(21, LIMIT_NAMUR_CLOSED), \
      I_MOVI(R1, addr + 3), \
      I_MOVI(R0, 0), \
      I_ST(R0, R1, 0), \
      I_BGE(17, 0), \
  I_MOVI(R2, addr + 2), \
  I_LD(R0, R2, 0), \
  I_BL(12, TRIES1), \
    I_MOVI(R2, addr + 3), \
    I_LD(R0, R2, 0), \
    I_BL(6, TRIES2), \
      I_MOVI(R0, 0), \
      I_ST(R0, R2, 0), \
      I_MOVI(R2, addr + 2), \
      I_ST(R0, R2, 0), \
      I_BGE(6, 0), \
    I_ADDI(R0, R0, 1), \
    I_ST(R0, R2, 0), \
    I_BGE(3, 0), \
  I_MOVI(R0, 0), \
  I_ST(R0, R2, 0)
/*
For comment purpose only duplicated
*/
// #define I_IS_IMPULS(reg_val, addr, bit) \
//   I_MOVR(R0, reg_val), \               // R0 < RTC_MEM_ADC_CH0_RAW (ADC Value)
//   I_BGE(26, LIMIT_NAMUR_CLOSED), \     // if  RTC_MEM_ADC_CH0_RAW >= LIMIT_NAMUR_CLOSED goto +26
//     I_MOVR(R3, R0), \                  // R3 <- R0 (RTC_MEM_ADC_CH0_RAW)
//     I_MOVI(R2, addr + 2), \            // R2 <- @RTC_MEM_ADC_CH0_CHECKS_1
//     I_LD(R0, R2, 0), \                 // R0 <- RTC_MEM_ADC_CH0_CHECKS_1
//     I_BGE(3, TRIES1), \                // if RTC_MEM_ADC_CH0_CHECKS_1 >= TRIES1 goto +3
//       I_ADDI(R0, R0, 1), \             // R0 <- R0 + 1
//       I_ST(R0, R2, 0), \               // R0 -> R2 (@RTC_MEM_ADC_CH0_CHECKS_1)
//         I_MOVR(R1, R0), \              // R1 <- R0 (RTC_MEM_ADC_CH0_CHECKS_1)
//         I_SUBI(R0, R0, TRIES1), \      // R0 <- R0 - TRIES1
//         I_BGE(13, 1), \                // if R0 >= 1 goto +13
//           I_ADDI(R1, R1, 1), \         // R1 <- R1 + 1 (RTC_MEM_ADC_CH0_CHECKS_1++)
//           I_ST(R1, R2, 0), \           // R1 -> R2 (@RTC_MEM_ADC_CH0_CHECKS_1)
//           I_MOVI(R0, addr + 1), \      // R0 <- @RTC_MEM_ADC_CH0
//           I_LD(R2, R0, 0), \           // R2 <- R0 (RTC_MEM_ADC_CH0)
//           I_ADDI(R2, R2, 1), \         // R2 <- R2 + 1 (RTC_MEM_ADC_CH0++)
//           I_ST(R2, R0, 0), \           // R2 -> R0 (@RTC_MEM_ADC_CH0)
//           I_MOVI(R0, RTC_MEM_FLAG), \  // R0 <- @RTC_MEM_FLAG
//           BIT_EN(R0, bit), \           // ENABLE BIT_# into R0
//           I_MOVR(R0, R3), \            // R0 <- R3 (RTC_MEM_ADC_CH0_RAW)
//     I_BL(21, LIMIT_NAMUR_CLOSED), \    // if R0 (RTC_MEM_ADC_CH0_RAW) < LIMIT_NAMUR_CLOSED goto +21
//       I_MOVI(R1, addr + 3), \          // R1 <- @RTC_MEM_ADC_CH0_CHECKS_2
//       I_MOVI(R0, 0), \                 // R0 <- 0
//       I_ST(R0, R1, 0), \               // R0 -> @RTC_MEM_ADC_CH0_CHECKS_2
//       I_BGE(17, 0), \                  // if R0 >= 0 goto +17
//   I_MOVI(R2, addr + 2), \              // R2 <- @RTC_MEM_ADC_CH0_CHECKS_1
//   I_LD(R0, R2, 0), \                   // R0 <- R2 (RTC_MEM_ADC_CH0_CHECKS_1)
//   I_BL(12, TRIES1), \                  // if R0 (RTC_MEM_ADC_CH0_CHECKS_1) < TRIES1 goto +3
//     I_MOVI(R2, addr + 3), \            // R2 <- @RTC_MEM_ADC_CH0_CHECKS_2
//     I_LD(R0, R2, 0), \                 // R0 <- R2 (RTC_MEM_ADC_CH0_CHECKS_2)
//     I_BL(6, TRIES2), \                 // if R0 (RTC_MEM_ADC_CH0_CHECKS_2) < TRIES2 goto +6
//       I_MOVI(R0, 0), \                 // R0 <- 0
//       I_ST(R0, R2, 0), \               // R0 -> R2 (@RTC_MEM_ADC_CH0_CHECKS_2)
//       I_MOVI(R2, addr + 2), \          // R2 <- @RTC_MEM_ADC_CH0_CHECKS_1
//       I_ST(R0, R2, 0), \               // R0 -> R2 (@RTC_MEM_ADC_CH0_CHECKS_1)
//       I_BGE(6, 0), \                   // if R0 >= 0 goto +6
//     I_ADDI(R0, R0, 1), \               // R0 <- R0 + 1 (RTC_MEM_ADC_CH0_CHECKS_2++)
//     I_ST(R0, R2, 0), \                 // R0 -> R2 (@RTC_MEM_ADC_CH0_CHECKS_2)
//     I_BGE(3, 0), \                     // if R0 >= 0 goto +3
//   I_MOVI(R0, 0), \                     // R0 <- 0
//   I_ST(R0, R2, 0)                      // R0 -> R2 (@RTC_MEM_ADC_CH0_CHECKS_2)


/*
 simple solution
*/
  // #define I_IS_IMPULS(reg_val, addr, bit) \
  // I_MOVR(R0, reg_val), \
  // I_BGE(22, LIMIT_NAMUR_CLOSED), \
  //   I_MOVR(R3, R0), \
  //   I_MOVI(R2, addr + 2), \
  //   I_LD(R0, R2, 0), \
  //   I_BGE(3, TRIES1), \
  //     I_ADDI(R0, R0, 1), \
  //     I_ST(R0, R2, 0), \
  //     I_MOVR(R1, R0), \
  //     I_SUBI(R0, R0, TRIES1), \
  //     I_BGE(16, 1), \
  //     I_ADDI(R1, R1, 1), \
  //     I_ST(R1, R2, 0), \
  //     I_MOVI(R0, addr + 1), \
  //     I_LD(R2, R0, 0), \
  //     I_ADDI(R2, R2, 1), \
  //     I_ST(R2, R0, 0), \
  //     I_MOVI(R0, RTC_MEM_FLAG), \
  //     BIT_EN(R0, bit), \
  //     I_MOVR(R0, R3), \
  //   I_BL(4, LIMIT_NAMUR_CLOSED), \
  // I_MOVI(R0, addr + 2), \
  // I_MOVI(R2, 0), \
  // I_ST(R2, R0, 0)

void ulp_init(const Button *btnSetup, const Counter *Counter_0, const Counter *Counter_1){
    ulp_set_wakeup_period(0, 100 * 1000); //128 ms

    const ulp_insn_t program[] = {
       
        //TODO: Save inputs state to memory

        //ADC COUNTERS HANDLER
        //GPIO_NUM_32
        M_LABEL(100),
        I_CLEAR_R(),
        I_WR_REG_BIT(rtc_gpio_desc[Counter_0->_pin].reg, rtc_gpio_desc2[Counter_0->_pin].pulldown, 0),  // Pull-Down Disable
        I_WR_REG_BIT(rtc_gpio_desc[Counter_0->_pin].reg, rtc_gpio_desc2[Counter_0->_pin].pullup, 1),    // Pull-Up Enable
        M_LABEL(1),
          I_ADC(R1, 0, (uint32_t)Counter_0->_adc),  // R1 <- ADC1_CH4
          I_ADDR(R2, R2, R1),                       // R2 += R1
          I_ADDI(R0, R0, 1),                        // R0 ++
        M_BL(1, 4),                                 // if (R0 < 4) goto M_LABEL(1)
        I_RSHI(R1, R2, 2),                          // R1 = R2 / 4 (average value)
        I_MOVI(R0, RTC_MEM_ADC_CH0_RAW),            // R0 <- @RTC_MEM_ADC_CH0_RAW
        I_ST(R1, R0, 0),                            // R1 -> RTC_SLOW_MEM[RTC_MEM_ADC_CH0_RAW]
        I_WR_REG_BIT(rtc_gpio_desc[Counter_0->_pin].reg, rtc_gpio_desc2[Counter_0->_pin].pullup, 0),    // Pull-Up Disable
        I_WR_REG_BIT(rtc_gpio_desc[Counter_0->_pin].reg, rtc_gpio_desc2[Counter_0->_pin].pulldown, 1),  // Pull-Down Enable

        I_IS_IMPULS(R1, RTC_MEM_ADC_CH0_RAW, BIT0),
        
        //GPIO_NUM_33
        M_LABEL(200),
        I_CLEAR_R(),
        I_WR_REG_BIT(rtc_gpio_desc[Counter_1->_pin].reg, rtc_gpio_desc2[Counter_1->_pin].pulldown, 0),  // Pull-Down Disable
        I_WR_REG_BIT(rtc_gpio_desc[Counter_1->_pin].reg, rtc_gpio_desc2[Counter_1->_pin].pullup, 1),    // Pull-Up Enable
        M_LABEL(2),
          I_ADC(R1, 0, (uint32_t)Counter_1->_adc),  // R1 <- ADC1_CH5
          I_ADDR(R2, R2, R1),                       // R2 += R1
          I_ADDI(R0, R0, 1),                        // R0 ++
        M_BL(2, 4),                                 // if (R0 < 4) goto M_LABEL(2)
        I_RSHI(R1, R2, 2),                          // R1 = R2 / 4 (average value)
        I_MOVI(R0, RTC_MEM_ADC_CH1_RAW),            // R0 <- @RTC_MEM_ADC_CH1_RAW
        I_ST(R1, R0, 0),                            // R1 -> RTC_SLOW_MEM[RTC_MEM_ADC_CH1_RAW]
        I_WR_REG_BIT(rtc_gpio_desc[Counter_1->_pin].reg, rtc_gpio_desc2[Counter_1->_pin].pullup, 0),    // Pull-Up Disable
        I_WR_REG_BIT(rtc_gpio_desc[Counter_1->_pin].reg, rtc_gpio_desc2[Counter_1->_pin].pulldown, 1),  // Pull-Down Enable

        I_IS_IMPULS(R1, RTC_MEM_ADC_CH1_RAW, BIT1), // FIXME: set BIT1 instead of BIT31

        //SETUP BUTTON HANDLER
        M_LABEL(300),
        I_CLEAR_R(),
        I_RD_REG(RTC_GPIO_IN_REG,RTCIO_GPIO25_CHANNEL+14, RTCIO_GPIO25_CHANNEL+14), // R0 <- GPIO25 // TODO: RTCIO_GPIO25_CHANNEL replace with some variable
        M_BL(7, 1),                                 // IF R0 < 1 goto M_LABEL(7)
        //I_MOVR(R3, R0),                           // R3 <- R0
        I_MOVI(R1, RTC_MEM_BTN),                    // R1 <- @RTC_MEM_BTN
        I_LD(R0, R1, 0),                            // R0 <- R1
        M_BGE(7, 1),                                // IF R0 >= 1 goto M_LABEL(7)
        I_MOVI(R0, 1),                              // R0 <- 1
        I_ST(R0, R1, 0),                            // R0 -> R1
        //I_MOVR(R0, R3),                           // R0 <- R3
        I_MOVI(R0, RTC_MEM_FLAG),
        BIT_EN(R0, BIT7),

        M_LABEL(7),

        I_MOVI(R1, RTC_MEM_FLAG),
        I_LD(R1, R1, 0),
        I_ANDI(R0, R1, 0xFF),
        M_BL(9, 1),                     // IF R0 < 1 goto M_LABEL(9)

        M_LABEL(8),
            I_RD_REG(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP_S, RTC_CNTL_RDY_FOR_WAKEUP_S),
            M_BXZ(8),
        I_WAKE(),

        M_LABEL(9),
            I_HALT()
    };

    size_t size = sizeof(program) / sizeof(ulp_insn_t);
    LOG_DEBUG("ULP", "Size of ulp program: " << size);
    if (unsafe_ulp_process_macros_and_load(RTC_MEM_PROG_START, program, &size) != ESP_OK) {
        LOG_ERROR("ULP", "Error loading ULP code!");
        return;
    }
    if (ulp_run(RTC_MEM_PROG_START) != ESP_OK) {
        LOG_ERROR("ULP", "Error running ULP code!");
        return;
    }
  LOG_NOTICE("ULP", "ULP run!");
}

#endif