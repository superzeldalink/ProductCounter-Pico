// SOME CODES ARE PREDEFINED AND INITIALIZED. DO NOT EDIT.
/*========================= INCLUDES ===================================*/
#include "pico/stdlib.h"
#include "perf_counter.h"

#if defined(__PICO_USE_LCD_1IN3__) && __PICO_USE_LCD_1IN3__
#include "DEV_Config.h"
#include "LCD_1In3.h"
#include "GLCD_Config.h"
#endif

#include <stdio.h>
#include "RTE_Components.h"

#if defined(RTE_Compiler_EventRecorder) && defined(USE_EVR_FOR_STDOUR)
  #include <EventRecorder.h>
#endif

/*========================= MACROS =====================================*/
#define TOP	(0x1FFF)

/*========================= MACROFIED FUNCTIONS ========================*/
#define ABS(__N)((__N) < 0 ? -(__N) : (__N))
#define _BV(__N)((uint32_t) 1 << (__N))

/*============================== CONSTANTS =============================*/
#define IR 16
#define BUZZER 17
#define DIO 18
#define RCLK 19
#define SCLK 20

/*========================= TYPES ======================================*/
/*========================= GLOBAL VARIABLES ===========================*/
unsigned char LED_0F[] = {
// 0     1      2    3     4     5      6    7     8     9
  0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
// A     b     C     d     E     F     -    off
  0x8C, 0xBF, 0xC6, 0xA1, 0x86, 0x8E, 0xbf, 0xFF
};

unsigned char LED[4] = {};
/*========================= LOCAL VARIABLES ============================*/
/*========================= PROTOTYPES =================================*/
/*========================= IMPLEMENTATION =============================*/
void SysTick_Handler(void) {}

static void system_init(void) {
  extern void SystemCoreClockUpdate();

  SystemCoreClockUpdate();
  /*! \note if you do want to use SysTick in your application, please use
   *!       init_cycle_counter(true);
   *!       instead of
   *!       init_cycle_counter(false);
   */
  init_cycle_counter(false);

  #if defined(RTE_Compiler_EventRecorder) && defined(USE_EVR_FOR_STDOUR)
  EventRecorderInitialize(0, 1);
  #endif

  gpio_init(DIO);
  gpio_set_dir(DIO, GPIO_OUT);

  gpio_init(SCLK);
  gpio_set_dir(SCLK, GPIO_OUT);

  gpio_init(RCLK);
  gpio_set_dir(RCLK, GPIO_OUT);

  gpio_init(BUZZER);
  gpio_set_dir(BUZZER, GPIO_OUT);

  gpio_init(IR);
  gpio_set_dir(IR, GPIO_IN);

  #if defined(__PICO_USE_LCD_1IN3__) && __PICO_USE_LCD_1IN3__
  DEV_Delay_ms(100);

  if (DEV_Module_Init() != 0) {
    //assert(0);
  }

  DEV_SET_PWM(50);
  /* LCD Init */

  LCD_1IN3_Init(HORIZONTAL);
  LCD_1IN3_Clear(GLCD_COLOR_BLUE);

  for (int n = 0; n < KEY_NUM; n++) {
    dev_key_init(n);
  }
  #endif
}

void LED_OUT(unsigned char X) {
  unsigned char i;
  for (i = 8; i >= 1; i--) {
    if (X & 0x80) {
      gpio_put(DIO, 1);
    } else {
      gpio_put(DIO, 0);
    }
    X <<= 1;
    gpio_put(SCLK, 0);
    gpio_put(SCLK, 1);
  }
}

void LED4_Display(void) {
  unsigned char * led_table;
  unsigned char i;

  led_table = LED_0F + LED[0];
  i = * led_table;
  LED_OUT(i);
  LED_OUT(0x08); // 1000
  gpio_put(RCLK, 0);
  gpio_put(RCLK, 1); // store in shift register

  led_table = LED_0F + LED[1];
  i = * led_table;
  LED_OUT(i);
  LED_OUT(0x04); // 0100
  gpio_put(RCLK, 0);
  gpio_put(RCLK, 1);

  led_table = LED_0F + LED[2];
  i = * led_table;
  LED_OUT(i);
  LED_OUT(0x02); //0010
  gpio_put(RCLK, 0);
  gpio_put(RCLK, 1);

  led_table = LED_0F + LED[3];
  i = * led_table;
  LED_OUT(i);
  LED_OUT(0x01); //0001
  gpio_put(RCLK, 0);
  gpio_put(RCLK, 1);
}

void Num2LED(int num) {
  LED[3] = num % 10;
  LED[2] = (num /= 10) % 10;
  LED[1] = (num /= 10) % 10;
  LED[0] = num / 10;
  LED4_Display();
}

int main(void) {
  system_init();
  gpio_put(BUZZER, 0);
  sleep_ms(100);
  int count = 0;

  while (true) {
    Num2LED(count);
    if (gpio_get(IR) == 0) {
      count++;
      gpio_put(BUZZER, 1);
      sleep_ms(100);
      gpio_put(BUZZER, 0);

      while (gpio_get(IR) == 0)
        Num2LED(count);
    }
  }

  return 0;
}
