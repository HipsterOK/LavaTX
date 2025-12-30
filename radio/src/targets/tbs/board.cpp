/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

 #include "opentx.h"
 #include "../../io/crsf/crsf_utilities.h"
 #include "hall90393.h"
 #include "led_driver.h"
 
 #if defined(__cplusplus) && !defined(SIMU)
 extern "C"
 {
 #endif
 #include "usb_dcd_int.h"
 #include "usb_bsp.h"
 #if defined(__cplusplus) && !defined(SIMU)
 }
 #endif
 
static uint8_t boardOffState = 0;
static uint8_t isDisableBoardOff();
static uint8_t checkDefaultWord();

// Глобальный флаг состояния питания
uint8_t g_powerEnabled = 0;
 
 HardwareOptions hardwareOptions;
 
 static uint32_t trampoline[TRAMPOLINE_INDEX_COUNT] = {0};
 
 void watchdogInit(unsigned int duration)
 {
   IWDG->KR = 0x5555; // Unlock registers
   IWDG->PR = 3;      // Divide by 32 => 1kHz clock
   IWDG->KR = 0x5555; // Unlock registers
   IWDG->RLR = duration;
   IWDG->KR = 0xAAAA; // reload
   IWDG->KR = 0xCCCC; // start
 }
 
 #if defined(SPORT_UPDATE_PWR_GPIO)
 void sportUpdateInit()
 {
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = SPORT_UPDATE_PWR_GPIO_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(SPORT_UPDATE_PWR_GPIO, &GPIO_InitStructure);
 }
 
 void sportUpdatePowerOn()
 {
   GPIO_SPORT_UPDATE_PWR_GPIO_ON(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN);
 }
 
 void sportUpdatePowerOff()
 {
   GPIO_SPORT_UPDATE_PWR_GPIO_OFF(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN);
 }
 #endif
 
 void intmoduleStop()
 {
 }
 
 #if defined(RADIO_TANGO)
 #define VBATT_W (LCD_W - 40)
 #define VBATT_H (45)
 #define VBATT_X (((LCD_W - VBATT_W) / 2) - 2)
 #define VBATT_Y (((LCD_H - VBATT_H) / 2) + 5)
 #define GET_BAT_BARS() (limit<int8_t>(2, 20 * (g_vbat100mV + 56 - 90) / (30 - 78 + 56), 20))
 #define MAX_BARS() (limit<int8_t>(2, 20 * (BATTERY_MAX + 56 - 90) / (30 - 78 + 56), 20))
 
 static void drawChargingState(void)
 {
   static uint8_t old_count = 0;
   static uint8_t blink_count = 0;
   static uint32_t charging_time = g_tmr10ms;
 
   lcdDrawText(VBATT_X - 5, VBATT_Y - 13, "CHARGING: ", MIDSIZE);
   putsVBat(VBATT_X + 90, VBATT_Y - 13, RIGHT | MIDSIZE);
   lcdDrawSolidFilledRect(VBATT_X - 4, VBATT_Y, VBATT_W + 4, VBATT_H);
   uint8_t count = GET_BAT_BARS();
 
   for (uint8_t i = 0; i < count; i += 2)
   {
     lcdDrawSolidFilledRect(VBATT_X + 2 + (VBATT_W / (MAX_BARS() - 1) * (i)), VBATT_Y + 7, VBATT_W / MAX_BARS() + 1, VBATT_H - 14);
   }
   lcdDrawSolidFilledRect(VBATT_X + VBATT_W, VBATT_Y + 10, 9, VBATT_H - 10 * 2);
 
   if (old_count != count)
   {
     old_count = count;
     blink_count = (count % 2) ? count + 1 : count;
   }
   for (uint8_t i = (old_count % 2) ? old_count + 1 : old_count; i < blink_count; i += 2)
   {
     lcdDrawSolidFilledRect(VBATT_X + 2 + (VBATT_W / (MAX_BARS() - 1) * (i)), VBATT_Y + 7, VBATT_W / MAX_BARS() + 1, VBATT_H - 14);
   }
   if ((g_tmr10ms - charging_time) > 6)
   {
     if ((blink_count++) >= MAX_BARS())
     {
       blink_count = old_count;
     }
     charging_time = g_tmr10ms;
   }
 }
 
 static void drawFullyCharged(void)
 {
   lcdDrawText(VBATT_X, VBATT_Y - 13, "CHARGED: ", MIDSIZE);
   putsVolts(VBATT_X + 70, VBATT_Y - 13, 42, MIDSIZE);
 
   lcdDrawSolidFilledRect(VBATT_X - 4, VBATT_Y, VBATT_W + 4, VBATT_H);
 
   for (uint8_t i = 0; i < MAX_BARS(); i += 2)
   {
     lcdDrawSolidFilledRect(VBATT_X + 2 + (VBATT_W / (MAX_BARS() - 1) * (i)), VBATT_Y + 7, VBATT_W / MAX_BARS() + 1, VBATT_H - 14);
   }
   lcdDrawSolidFilledRect(VBATT_X + VBATT_W, VBATT_Y + 10, 9, VBATT_H - 10 * 2);
 }
 #endif
 
 #define PWR_PRESSED_CNT 3
 #define WIFI_PWR_CHARGING_TIMEOUT 6000
 #define BACKLIGHT_TIMEOUT 500
 #define BATT_ADC_SAMPLING_TIME 10
 #define ANIMATION_UPDATE_TIME 50
 #define KEY_PRESS_UPDATE_TIME 10
 #define CHARGING_TO_CHARGED_DELAY 500
 #define NUM_OF_KEY_GROUPS 6
 static void runPwrOffCharging(void)
 {
   tmr10ms_t tmrAdc = 0;
   tmr10ms_t tmrWait = 0;
   tmr10ms_t tmrPressed = 0;
   uint8_t pwrPressedCnt = 0;
   uint32_t lastChargingTimestamp = 0;
 #if defined(CHARGING_ANIMATION)
   tmr10ms_t tmrBacklight = 0;
   uint16_t keysState[NUM_OF_KEY_GROUPS];
   GPIO_TypeDef *keysPort[NUM_OF_KEY_GROUPS] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF};
   uint16_t keysPin[NUM_OF_KEY_GROUPS] = {KEYS_GPIOA_PINS, KEYS_GPIOB_PINS, KEYS_GPIOC_PINS, KEYS_GPIOD_PINS, KEYS_GPIOE_PINS, KEYS_GPIOF_PINS};
 #endif
 #if defined(CHARGING_LEDS)
   bool isLedCharging = false;
   bool isLedCharged = false;
 #endif
 
   // initialize usb state & voltage adc
   usbPlugged();
   getADC();
   checkBattery();
 
   while (1)
   {
     // update battery voltage
     if (g_tmr10ms - tmrAdc >= BATT_ADC_SAMPLING_TIME)
     {
       getADC();
       checkBattery();
       tmrAdc = g_tmr10ms;
     }
 
     // check if there is a power on event
     if (g_tmr10ms - tmrPressed >= KEY_PRESS_UPDATE_TIME)
     {
       tmrPressed = g_tmr10ms;
       if (pwrPressed())
         pwrPressedCnt++;
       else
         pwrPressedCnt = 0;
     }
 
     // quit the power off charging loop
     if (!usbPlugged() || pwrPressedCnt >= PWR_PRESSED_CNT)
       break;
     else if (pwrPressedCnt)
       continue;
 
     // turn off ESP power if it's a long charging process
 #if defined(ESP_SERIAL)
     if (g_tmr10ms >= WIFI_PWR_CHARGING_TIMEOUT && WIFI_IS_ON())
       WIFI_OFF();
 #endif
 
     // backlight - полностью отключена в режиме зарядки/выключения
 #if defined(CHARGING_ANIMATION)
       BACKLIGHT_DISABLE();
 #endif
 
     // charging
     if (IS_CHARGING_STATE() && !IS_CHARGING_FAULT() && usbPlugged())
     {
       lastChargingTimestamp = g_tmr10ms;
 #if defined(CHARGING_LEDS)
       if (isLedCharging == false)
       {
         isLedCharging = true;
         LED_CHARGING_IN_PROGRESS();
       }
 #endif
       if (g_tmr10ms - tmrWait >= ANIMATION_UPDATE_TIME)
       {
 #if defined(CHARGING_ANIMATION)
         lcdClear();
         drawChargingState();
         lcdRefresh();
 #endif
         TRACE("state: charging  vbatt: %.1fV", (float)g_vbat100mV / 10);
         tmrWait = g_tmr10ms;
       }
     }
     // charged
     else if (!IS_CHARGING_STATE() && !IS_CHARGING_FAULT() && usbPlugged() && g_tmr10ms - lastChargingTimestamp >= CHARGING_TO_CHARGED_DELAY)
     {
 #if defined(CHARGING_LEDS)
       if (isLedCharged == false)
       {
         isLedCharged = true;
         LED_CHARGING_DONE();
       }
 #endif
       if (g_tmr10ms - tmrWait >= ANIMATION_UPDATE_TIME)
       {
 #if defined(CHARGING_ANIMATION)
         lcdClear();
         drawFullyCharged();
         lcdRefresh();
 #endif
         TRACE("state: charged  vbatt: %.1fV", (float)g_vbat100mV / 10);
         tmrWait = g_tmr10ms;
       }
     }
   }
 
   // Если мы здесь, значит USB был отключен - очищаем экран
   TRACE("USB unplugged, clearing display...\n");
   lcdClear();
   lcdOff();
 }
 static char g_battDebugMsg[40];
 
 static void showBatteryDebugPopup()
 {
   snprintf(g_battDebugMsg, sizeof(g_battDebugMsg),
            "RAW=%u ANA=%u",
            adcValues[TX_VOLTAGE],
            anaIn(TX_VOLTAGE));  // anaIn = отфильтрованное значение (фильтр)
 
   POPUP_WARNING(g_battDebugMsg);
 }
 
 void boardInit()
 {
   TRACE("BOARD_INIT: Starting board initialization\n");
  // === ИНИЦИАЛИЗАЦИЯ ПИТАНИЯ ===
  // Сначала инициализируем минимум для работы с питанием
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

  // Инициализация питания (PWR_ON начинает с LOW)
  pwrInit();

  // ПРОВЕРЯЕМ КНОПКУ ПИТАНИЯ - если не нажата, то НЕ ВКЛЮЧАЕМ ПИТАНИЕ
  if (!pwrPressed()) {
    TRACE("BOARD_INIT: Power button not pressed, system will not start\n");
    // Устанавливаем флаг, что питание не включено
    g_powerEnabled = 0;
    // Минимальная инициализация без питания
    BACKLIGHT_DISABLE();
    return; // Выходим из boardInit() без включения питания
  }

  // КНОПКА НАЖАТА - ВКЛЮЧАЕМ ПИТАНИЕ И ЗАПУСКАЕМ СИСТЕМУ
  TRACE("BOARD_INIT: Power button pressed, enabling power and starting system\n");
  g_powerEnabled = 1; // Устанавливаем флаг, что питание включено

  // ВКЛЮЧАЕМ ПИТАНИЕ
  pwrOn();

  // ЗАДЕРЖКА ДЛЯ СТАБИЛИЗАЦИИ ПИТАНИЯ (увеличена для надежности)
  volatile uint32_t delay = 200000;  // 200ms для полной стабилизации TPS63060
  while (delay--) { __ASM volatile("nop"); }

  // ИНИЦИАЛИЗИРУЕМ LCD ПОСЛЕ ВКЛЮЧЕНИЯ ПИТАНИЯ
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

  // ОЧИСТИМ ДИСПЛЕЙНЫЙ БУФЕР СРАЗУ ПОСЛЕ ВКЛЮЧЕНИЯ GPIO
  // Это гарантирует чистую инициализацию без мусора
  memset(displayBuf, 0x00, sizeof(displayBuf));
  TRACE("LCD: displayBuf cleared at startup\n");

  delaysInit();
  TRACE("LCD: delaysInit completed\n");

  lcdInit();
  TRACE("LCD: lcdInit completed\n");

  // Дополнительная задержка после инициализации LCD
  delay = 50000;  // 50ms
  while (delay--) { __ASM volatile("nop"); }
  TRACE("LCD: delay after init completed\n");

  // Очистить экран и продолжить нормальную работу
  lcdClear();
  TRACE("LCD: Screen cleared for normal operation\n");

  lcdRefresh();
  TRACE("LCD: Normal refresh completed\n");

  // Для монохромного дисплея подсветка не нужна
  // BACKLIGHT_ENABLE();
  TRACE("LCD: LCD initialization completed\n");

  TRACE("Power enabled and LCD initialized - system starting\n");
 
   // Теперь продолжаем нормальную инициализацию RCC
   // RCC для GPIOC/D/E уже включен выше для LCD
 
   // Инициализация SD_DETECT (PC5) как вход с pull-up
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = SD_DETECT_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(SD_DETECT_GPIO_PORT, &GPIO_InitStructure);
 
   // Теперь продолжаем нормальную инициализацию
   bool skipCharging = false;
 #if defined(RADIO_TANGO)
   RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2 |
                              KEYS_RCC_AHB1Periph | RCC_AHB1Periph_GPIOA | // LCD GPIO на PA
                              AUDIO_RCC_AHB1Periph |
                              ADC_RCC_AHB1Periph |
                              SD_RCC_AHB1Periph | HAPTIC_RCC_AHB1Periph |
                              TELEMETRY_RCC_AHB1Periph | LED_RCC_AHB1Periph |
                              AUX_SERIAL_RCC_AHB1Periph,
                          ENABLE);
 
   // 2. Разделение clock команд для APB1 и APB2
   RCC_APB1PeriphClockCmd(AUDIO_RCC_APB1Periph | RCC_APB1Periph_SPI2 |
                        INTERRUPT_xMS_RCC_APB1Periph |
                        TIMER_2MHz_RCC_APB1Periph |
                        TELEMETRY_RCC_APB1Periph | AUX_SERIAL_RCC_APB1Periph,
                        ENABLE);
 
   // 3. Добавление SPI1 (LCD) на APB2
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1| RCC_APB2Periph_SYSCFG | RCC_APB2Periph_TIM10 | ADC_RCC_APB2Periph, ENABLE);
 
 #elif defined(RADIO_MAMBO)
   RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph | KEYS_RCC_AHB1Periph | LCD_RCC_AHB1Periph |
                              AUDIO_RCC_AHB1Periph | BACKLIGHT_RCC_AHB1Periph | ADC_RCC_AHB1Periph |
                              SD_RCC_AHB1Periph | HAPTIC_RCC_AHB1Periph | EXTMODULE_RCC_AHB1Periph |
                              TELEMETRY_RCC_AHB1Periph | AUX_SERIAL_RCC_AHB1Periph,
                          ENABLE);
 
   RCC_APB1PeriphClockCmd(LCD_RCC_APB1Periph | AUDIO_RCC_APB1Periph | BACKLIGHT_RCC_APB1Periph |
                              INTERRUPT_xMS_RCC_APB1Periph | TIMER_2MHz_RCC_APB1Periph | LED_RCC_AHB1Periph | TELEMETRY_RCC_APB1Periph |
                              AUX_SERIAL_RCC_APB1Periph | MIXER_SCHEDULER_TIMER_RCC_APB1Periph,
                          ENABLE);
 
   RCC_APB2PeriphClockCmd(ADC_RCC_APB2Periph | EXTMODULE_RCC_APB2Periph | ROTARY_ENCODER_RCC_APB2Periph ,
                          ENABLE);
 #endif
 
   keysInit();
 
   // we need to initialize g_FATFS_Obj here, because it is in .ram section (because of DMA access)
   // and this section is un-initialized
   memset(&g_FATFS_Obj, 0, sizeof(g_FATFS_Obj));
 
 #if defined(ROTARY_ENCODER_NAVIGATION)
   rotaryEncoderInit();
 #endif
   adcInit();  // delaysInit() и lcdInit() уже вызваны выше
 
 #if defined(RADIO_MAMBO)
   backlightInit();
 #endif
   audioInit(); // Включаем audio
   init2MhzTimer();
   init5msTimer();
   backlightInit(); // Инициализируем подсветку после CRSF
 
   // Инициализация статусных LED
   statusLedInit();
 
   // Индикация: CRSF инициализирован - включаем LED_NO_LINK (красный, связи нет)
   statusLedNoLinkOn();
 
   usbInit(); // Включаем USB
 
   // Индикация: прошли USB - включаем LED_PWR_ON (зеленый, пульт включен)
   statusLedPwrOnOn();
 
 #if defined(CHARGING_LEDS)
   ledInit();
 #endif
 #if defined(USB_CHARGER)
   usbChargerInit();
 #endif
   #if defined(RADIO_TANGO)
       ledPowerOn();   // зелёный LED: Пульт включён
   #endif
   __enable_irq();
 
   hardwareOptions.pcbrev = crsfGetHWID() & ~HW_ID_MASK;
 
 #if defined(RTCLOCK) && !defined(COPROCESSOR)
   rtcInit(); // RTC must be initialized before rambackupRestore() is called
 #endif
 
 #if defined(DEBUG) && defined(AUX_SERIAL_GPIO)
   auxSerialInit(0, 0); // default serial mode (None if DEBUG not defined)
   TRACE("\n%s board started :)", MY_DEVICE_NAME);
   TRACE("RCC->CSR = %08x\n", RCC->CSR);
 #endif
 
   // skipCharging = true только если был UNEXPECTED_SHUTDOWN (неожиданное выключение)
   // и USB подключен - тогда показываем зарядку
   if (UNEXPECTED_SHUTDOWN() && usbPlugged())
   {
     skipCharging = true;
     // clear software reset mark
     if (WAS_RESET_BY_SOFTWARE())
     {
       RCC_ClearFlag();
     }
   }
 
 #if defined(HAPTIC)
   hapticInit();
 #endif
 
 #if defined(DEBUG)
   DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP | DBGMCU_TIM1_STOP | DBGMCU_TIM2_STOP | DBGMCU_TIM3_STOP | DBGMCU_TIM6_STOP | DBGMCU_TIM8_STOP | DBGMCU_TIM10_STOP | DBGMCU_TIM13_STOP | DBGMCU_TIM14_STOP, ENABLE);
 #endif
 
 #if defined(PWR_BUTTON_PRESS)
   // PWR_ON всегда HIGH - простая логика
 #endif
 
   if (!UNEXPECTED_SHUTDOWN())
     sdInit();
 
   if (skipCharging || usbPlugged())
   {
     runPwrOffCharging();
     // После зарядки проверяем, был ли USB отключен
     if (!usbPlugged()) {
       TRACE("USB disconnected during charging, clearing screen and powering off...\n");
 
       // ПРАВИЛЬНАЯ ОЧИСТКА ЭКРАНА ЗАРЯДКИ С ФИНАЛЬНЫМ ОБНОВЛЕНИЕМ
 
       // 1. Очищаем буфер дисплея
       lcdClear();
 
       // 2. Отправляем пустой буфер на дисплей один раз
       lcdRefresh();
 
       // 3. Отключаем дисплей
       lcdOff();
 
       // 4. Короткая задержка для стабильности
       volatile uint32_t delay = 50000;
       while (delay--) { __ASM volatile("nop"); }
 
       // 5. Финальная очистка буфера (останется в памяти на случай включения)
       lcdClear();
 
       boardOff(); // Переходим в выключенное состояние
     }
   }
   crsfInit(); // Включаем CRSF с правильными настройками

   // Питание уже включено после нажатия кнопки, все системы готовы
   TRACE("BOARD_INIT: All systems initialized, device is ready\n");
 
 }
 
 void boardOff()
 {
   TRACE("BOARD_OFF: boardOff() called - starting power off sequence\n");
   static int powerOffCount = 0;
   powerOffCount++;
   TRACE("ULTRA LOW POWER MODE #%d\n", powerOffCount);
 
   // МИНИМАЛЬНОЕ ОТКЛЮЧЕНИЕ - максимальная экономия энергии
 
   // ПРАВИЛЬНАЯ ОЧИСТКА ЭКРАНА ПРИ ВЫКЛЮЧЕНИИ С ФИНАЛЬНЫМ ОБНОВЛЕНИЕМ
   lcdClear();          // Очищаем буфер
   lcdRefresh();        // Отправляем пустой буфер на дисплей
   lcdOff();            // Физически отключаем дисплей
 
   // Короткая задержка для стабильности
   volatile uint32_t delay = 50000;
   while (delay--) { __ASM volatile("nop"); }
 
   // ЭКСТРЕМАЛЬНОЕ ОТКЛЮЧЕНИЕ ВСЕХ LED
 
   // 1. GPIO LED на GPIOE - переводим в input pull-down режим
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 
   // LED_LOW_BATT (PE8), LED_PWR_ON (PE9), LED_LINK_OK (PE10), LED_NO_LINK (PE11)
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
   GPIO_Init(GPIOE, &GPIO_InitStructure);
 
   // 2. Отключаем RGB LED (WS2812) на PB11
   #if defined(CHARGING_LEDS)
     ledOff();  // Отключаем RGB LED
   #endif
 
   // 3. Отключаем подсветку кнопок (PE12) - input pull-down
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
   GPIO_Init(GPIOE, &GPIO_InitStructure);
 
   // 4. Вызываем все функции отключения LED для надежности
   statusLedAllOff();
 
   // 5. Дополнительное принудительное отключение через ODR
   GPIOE->ODR &= ~(GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);
   GPIOB->ODR &= ~(GPIO_Pin_11);  // PB11 для RGB LED
 
   // 6. Гарантируем, что пины в LOW состоянии
   GPIO_ResetBits(GPIOE, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);
   GPIO_ResetBits(GPIOB, GPIO_Pin_11);
 
   // Отключаем звук и подсветку
 #if defined(AUDIO_MUTE_GPIO_PIN)
     GPIO_SetBits(AUDIO_MUTE_GPIO, AUDIO_MUTE_GPIO_PIN);
 #endif
   BACKLIGHT_DISABLE();
 
   // ВКЛЮЧАЕМ GPIOB CLOCK перед отключением питания (важно!)
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 
   // ВЫКЛЮЧАЕМ ПИТАНИЕ ЧЕРЕЗ TPS63060DSCR (PB12)
   TRACE("Calling pwrOff() to disable PB12\n");
   pwrOff(); // Устанавливает PB12 в LOW через прямой доступ к регистрам
   TRACE("pwrOff() completed\n");
 
   // Короткая задержка для TPS63060 shutdown
   volatile uint32_t shutdown_delay = 10000;
   while (shutdown_delay--) { __ASM volatile("nop"); }
 
   // Отключаем периферию
   crossfirePowerOff();
   crossfireTasksStop();
   usbStop();
 
   // ОТКЛЮЧАЕМ ВСЕ СИСТЕМЫ для максимальной экономии
   SysTick->CTRL = 0;  // SysTick off
   IWDG->KR = 0x0000;  // Watchdog off
   WWDG->CR = 0x7F;    // Window watchdog off
   WWDG->CFR = 0x7F;
 
   // Отключаем ВСЕ таймеры
   TIM1->CR1 &= ~TIM_CR1_CEN;
   TIM2->CR1 &= ~TIM_CR1_CEN;
   TIM3->CR1 &= ~TIM_CR1_CEN;
   TIM4->CR1 &= ~TIM_CR1_CEN;
   TIM5->CR1 &= ~TIM_CR1_CEN;
   TIM6->CR1 &= ~TIM_CR1_CEN;
   TIM7->CR1 &= ~TIM_CR1_CEN;
   TIM8->CR1 &= ~TIM_CR1_CEN;
   TIM9->CR1 &= ~TIM_CR1_CEN;
   TIM10->CR1 &= ~TIM_CR1_CEN;
   TIM11->CR1 &= ~TIM_CR1_CEN;
   TIM12->CR1 &= ~TIM_CR1_CEN;
   TIM13->CR1 &= ~TIM_CR1_CEN;
   TIM14->CR1 &= ~TIM_CR1_CEN;
 
   // Отключаем ADC
   ADC1->CR2 &= ~ADC_CR2_ADON;
 
  // ПОЛНОЕ ОТКЛЮЧЕНИЕ - СБРАСЫВАЕМ ФЛАГ И ДЕЛАЕМ RESET
  TRACE("POWER OFF COMPLETE - RESET FOR CLEAN STATE\n");

  // Сбрасываем флаг питания
  extern uint8_t g_powerEnabled;
  g_powerEnabled = 0;

  TRACE("POWER OFF: g_powerEnabled reset to 0, system reset\n");

  // Полный системный reset для возврата в состояние ожидания кнопки
  NVIC_SystemReset();
     // Убеждаемся, что GPIOA включен для USB detection
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
 
     // DEBUG: Проверяем состояние GPIOA и USB
     static uint32_t debugCounter = 0;
     debugCounter++;
     if (debugCounter % 1000 == 0) {  // Каждые 10 секунд
       TRACE("ULTRA LOW POWER: GPIOA enabled, checking USB...\n");
   }
 
     // Проверяем зарядку (GPIOA Pin 9)
     if (usbPlugged()) {
       TRACE("USB CHARGER DETECTED - POWER ON AND RESTART\n");
 
       // ВКЛЮЧАЕМ GPIOB CLOCK перед включением питания
       RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 
       // ВКЛЮЧАЕМ ПИТАНИЕ обратно перед перезапуском
       // Система перезапустится и в boardInit() включит питание плавно
       pwrOn(); // Устанавливает PB12 в HIGH через прямой доступ к регистрам
 
       // Задержка для стабилизации питания TPS63060
       volatile uint32_t delay = 50000;
       while (delay--) { __ASM volatile("nop"); }
 
       TRACE("USB CHARGER: System restart with power enabled\n");
     NVIC_SystemReset();
     }
 
     // Проверяем кнопку (PA3) - убеждаемся что GPIOB включен
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
     if (pwrPressed()) {
       TRACE("POWER BUTTON PRESSED - POWER ON AND WAKE UP\n");
 
       // ВКЛЮЧАЕМ GPIOB CLOCK перед включением питания
       RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 
       // ВКЛЮЧАЕМ ПИТАНИЕ обратно перед перезапуском
       // Система перезапустится и в boardInit() включит питание плавно
       pwrOn(); // Устанавливает PB12 в HIGH через прямой доступ к регистрам
 
       // Задержка для стабилизации питания TPS63060
       volatile uint32_t delay = 50000;
       while (delay--) { __ASM volatile("nop"); }
 
       TRACE("POWER ON: System restart with power enabled\n");
       NVIC_SystemReset();
     }
 
     // Минимальная задержка 10мс
     volatile uint32_t poll_delay = 10000;
     while (poll_delay--) { __ASM volatile("nop"); }
   }
 
 // Interrupt handler не нужен для polling подхода
 
 uint16_t getBatteryVoltage()
 {
  int32_t instant_vbat = anaIn(TX_VOLTAGE); // using filtered ADC value on purpose
   float batt_scale = 0;
 #if defined(RADIO_TANGO)
   if (IS_PCBREV_01())
     batt_scale = BATT_SCALE;
   else 
     batt_scale = BATT_SCALE2;
 #elif defined(RADIO_MAMBO)
   batt_scale = BATT_SCALE;
 #endif
 
   instant_vbat = instant_vbat / batt_scale + g_eeGeneral.txVoltageCalibration;
   return (uint16_t)instant_vbat;
 }
 
 uint8_t getBoardOffState()
 {
   return boardOffState;
 }
 
 // EXTI обработчик больше не нужен - используем polling
 
 void boardReboot2bootloader(uint32_t isNeedFlash, uint32_t HwId, uint32_t sn)
 {
   usbStop();
   crossfirePowerOff();
   RTOS_SET_FLAG(get_task_flag(XF_TASK_FLAG));
   writeBackupReg(BKREG_PREPARE_FWUPDATE, isNeedFlash);
   writeBackupReg(BKREG_HW_ID, HwId);
   writeBackupReg(BKREG_SERIAL_NO, sn);
   boardSetSkipWarning();
   NVIC_SystemReset();
 }
 
 static uint8_t isDisableBoardOff()
 {
   uint8_t value = (uint8_t)readBackupReg(BKREG_SKIP_BOARD_OFF);
   writeBackupReg(BKREG_SKIP_BOARD_OFF, 0);
   boardOffState = bkregGetStatusFlag(DEVICE_RESTART_WITHOUT_WARN_FLAG);
   bkregClrStatusFlag(DEVICE_RESTART_WITHOUT_WARN_FLAG);
   boardOffState |= value;
   if (!checkDefaultWord())
   {
     boardOffState = 0;
   }
   return boardOffState;
 }
 
 static uint8_t checkDefaultWord()
 {
   union
   {
     uint8_t b[4];
     uint32_t word;
   } defaultWord;
   defaultWord.b[0] = (uint8_t)'t';
   defaultWord.b[1] = (uint8_t)'a';
   defaultWord.b[2] = (uint8_t)'n';
   defaultWord.b[3] = (uint8_t)'g';
   uint32_t value = (uint32_t)readBackupReg(BKREG_DEFAULT_WORD);
   if (value != defaultWord.word)
   {
     writeBackupReg(BKREG_DEFAULT_WORD, defaultWord.word);
     return 0;
   }
   return 1;
 }
 
 void trampolineInit(void)
 {
   memset(trampoline, 0, sizeof(uint32_t) * TRAMPOLINE_INDEX_COUNT);
   trampoline[RTOS_WAIT_FLAG_TRAMPOILINE] = (uint32_t)(&CoWaitForSingleFlag);
   trampoline[RTOS_CLEAR_FLAG_TRAMPOILINE] = (uint32_t)(&CoClearFlag);
   crossfireSharedData.trampoline = trampoline;
 }
 
 void loadDefaultRadioSettings(void)
 {
   // this is to reset incorrect radio settings. should be removed later.
 #if defined(RADIO_TANGO)
   g_eeGeneral.backlightMode = g_eeGeneral.backlightMode < e_backlight_mode_keys ? e_backlight_mode_keys : g_eeGeneral.backlightMode;
 #endif
   g_eeGeneral.lightAutoOff = g_eeGeneral.lightAutoOff < BACKLIGHT_TIMEOUT_MIN ? 6 : g_eeGeneral.lightAutoOff;
   g_eeGeneral.switchConfig = DEFAULT_SWITCH_CONFIG;
   g_eeGeneral.jitterFilter = 0;
 
 #define MARK_1 0x0000A55A
 #define MARK_1_MASK 0x0000FFFF
 #define MARK_2 0xA55A0000
 #define MARK_2_MASK 0xFFFF0000
   uint32_t temp_buf = (uint32_t)readBackupReg(BKREG_TEMP_BUFFER_ADDR);
   if (((temp_buf & MARK_1_MASK) != MARK_1) && (g_eeGeneral.pwrOnSpeed != 0) && (g_eeGeneral.pwrOnSpeed != 0))
   {
     g_eeGeneral.pwrOnSpeed = 0;
     g_eeGeneral.pwrOffSpeed = 0;
     temp_buf = (temp_buf & ~MARK_1_MASK) | MARK_1;
     writeBackupReg(BKREG_TEMP_BUFFER_ADDR, temp_buf);
   }
 
   if (((temp_buf & MARK_2_MASK) != MARK_2) && ((g_eeGeneral.vBatWarn < BATTERY_WARN) || (g_eeGeneral.vBatMin < -90 + BATTERY_MIN) || (g_eeGeneral.vBatMax > -120 + BATTERY_MAX)))
   {
     if (g_eeGeneral.vBatWarn < BATTERY_WARN)
       g_eeGeneral.vBatWarn = BATTERY_WARN;
     if (g_eeGeneral.vBatMin < -90 + BATTERY_MIN)
       g_eeGeneral.vBatMin = -90 + BATTERY_MIN;
     if (g_eeGeneral.vBatMax > -120 + BATTERY_MAX)
       g_eeGeneral.vBatMax = -120 + BATTERY_MAX;
     temp_buf = (temp_buf & ~MARK_2_MASK) | MARK_2;
     writeBackupReg(BKREG_TEMP_BUFFER_ADDR, temp_buf);
   }
 
   storageDirty(EE_GENERAL);
   storageCheck(true);
 }
 
 #if defined(DEBUG)
 #define UART_INT_MODE_TX 1
 #define UART_INT_MODE_RX 2
 extern Fifo<uint8_t, 512> auxSerialTxFifo;
 extern "C" void AUX_SERIAL_USART_IRQHandler(void)
 {
   DEBUG_INTERRUPT(INT_SER2);
   bool xf_active = false;
   bool (*uart_irq)(uint8_t, uint8_t);
   bool xf_valid = false;
   uint8_t data = 0;
 
   if (crossfireSharedData.trampoline[DEBUG_UART_IRQ_TRAMPOLINE])
   {
     uart_irq = (bool (*)(uint8_t, uint8_t))crossfireSharedData.trampoline[DEBUG_UART_IRQ_TRAMPOLINE];
     xf_valid = true;
   }
 
   // Send
   if (USART_GetITStatus(AUX_SERIAL_USART, USART_IT_TXE) != RESET)
   {
     if (xf_valid)
       xf_active = uart_irq(UART_INT_MODE_TX, 0);
     if (!xf_active)
     {
       if (!auxSerialTxFifo.isEmpty())
       {
         /* Write one byte to the transmit data register */
         auxSerialTxFifo.pop(data);
         USART_SendData(AUX_SERIAL_USART, data);
       }
       else
       {
         USART_ITConfig(AUX_SERIAL_USART, USART_IT_TXE, DISABLE);
       }
     }
   }
 
   // Receive
   uint32_t status = AUX_SERIAL_USART->SR;
   while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
   {
     uint8_t data = AUX_SERIAL_USART->DR;
     if (!(status & USART_FLAG_ERRORS))
     {
 #if defined(CLI)
       switch (auxSerialMode)
       {
       case UART_MODE_DEBUG:
         cliRxFifo.push(data);
         break;
       }
 #else
       if (xf_valid)
         uart_irq(UART_INT_MODE_RX, data);
 #endif
     }
     status = AUX_SERIAL_USART->SR;
   }
 }
 #endif
 
 extern "C"
 {
 
 #if defined(ESP_SERIAL)
   void ESP_DMA_Stream_IRQHandler(void)
   {
     DEBUG_INTERRUPT(INT_WIFI_DMA);
     if (DMA_GetITStatus(ESP_DMA_Stream_RX, ESP_DMA_RX_FLAG_TC))
     {
       DMA_ClearITPendingBit(ESP_DMA_Stream_RX, ESP_DMA_RX_FLAG_TC);
       void (*esp_dma_rx_irq)(void);
       if (crossfireSharedData.trampoline[WIFI_UART_IRQ_TRAMPOLINE])
       {
         esp_dma_rx_irq = (void (*)(void))crossfireSharedData.trampoline[WIFI_UART_IRQ_TRAMPOLINE];
         esp_dma_rx_irq();
       }
     }
   }
 #endif
 
   void INTERRUPT_EXTI_IRQHandler(void)
   {
     // DEBUG_INTERRUPT(INT_EXTI15_10);
     // CoEnterISR();
     // void (* exti_irq)(void);
     // if (crossfireSharedData.trampoline[DIO_IRQ_TRAMPOLINE]) {
     //   exti_irq = (void (*)(void))crossfireSharedData.trampoline[DIO_IRQ_TRAMPOLINE];
     //   /* call DIOCN handler of crossfire */
     //   exti_irq();
     //   isr_SetFlag(get_task_flag(XF_TASK_FLAG));
     // }
     // CoExitISR();
   }
 
   void INTERRUPT_TIM13_IRQHandler()
   {
     DEBUG_INTERRUPT(INT_TIM13);
     CoEnterISR();
     if (INTERRUPT_NOT_TIMER->SR & TIM_SR_UIF)
     {
       INTERRUPT_NOT_TIMER->SR &= ~TIM_SR_UIF;
       void (*timer_irq)(void);
       if (crossfireSharedData.trampoline[NOTIFICATION_TIMER_IRQ_TRAMPOLINE])
       {
         timer_irq = (void (*)(void))crossfireSharedData.trampoline[NOTIFICATION_TIMER_IRQ_TRAMPOLINE];
         /* call notification timer handler of crossfire */
         timer_irq();
         isr_SetFlag(get_task_flag(XF_TASK_FLAG));
       }
     }
     CoExitISR();
   }
 
 #if defined(DEBUG) && defined(AUX_SERIAL_GPIO)
 #include <stdio.h>
 #include <stdarg.h>
   void uart_tx(uint8_t byte)
   {
     while (USART_GetFlagStatus(AUX_SERIAL_USART, USART_FLAG_TXE) == RESET)
       ;
     USART_SendData(AUX_SERIAL_USART, byte);
   }
 
   void hf_printf(const char *TxBuf, ...)
   {
     uint8_t UartBuf[200];
     va_list arglist;
     volatile uint8_t *fp;
 
     va_start(arglist, TxBuf);
     vsprintf((char *)UartBuf, (const char *)TxBuf, arglist);
     va_end(arglist);
 
     fp = UartBuf;
     while (*fp)
     {
       uart_tx(*fp);
       fp++;
     }
   }
 
   void _general_exception_handler(unsigned int *hardfault_args)
   {
     bool xf_active = true;
     bool (*uart_irq)(uint8_t, uint8_t) = (bool (*)(uint8_t, uint8_t))crossfireSharedData.trampoline[DEBUG_UART_IRQ_TRAMPOLINE];
     unsigned int stacked_r0;
     unsigned int stacked_r1;
     unsigned int stacked_r2;
     unsigned int stacked_r3;
     unsigned int stacked_r12;
     unsigned int stacked_lr;
     unsigned int stacked_pc;
     unsigned int stacked_psr;
 
     stacked_r0 = ((unsigned long)hardfault_args[0]);
     stacked_r1 = ((unsigned long)hardfault_args[1]);
     stacked_r2 = ((unsigned long)hardfault_args[2]);
     stacked_r3 = ((unsigned long)hardfault_args[3]);
 
     stacked_r12 = ((unsigned long)hardfault_args[4]);
     stacked_lr = ((unsigned long)hardfault_args[5]);
     stacked_pc = ((unsigned long)hardfault_args[6]);
     stacked_psr = ((unsigned long)hardfault_args[7]);
 
     // to print all buffered messages before printing the stack
     while (1)
     {
       WDG_RESET();
       if (USART_GetFlagStatus(AUX_SERIAL_USART, USART_FLAG_TXE) != RESET)
         xf_active = uart_irq(UART_INT_MODE_TX, 0);
       if (!xf_active)
         break;
     }
 
     hf_printf("\r\n\n***OpenTX Hard Fault Handler Debug Printing***\r\n");
     hf_printf("R0\t\t= 0x%.8x\r\n", stacked_r0);
     hf_printf("R1\t\t= 0x%.8x\r\n", stacked_r1);
     hf_printf("R2\t\t= 0x%.8x\r\n", stacked_r2);
     hf_printf("R3\t\t= 0x%.8x\r\n", stacked_r3);
     hf_printf("R12\t\t= 0x%.8x\r\n", stacked_r12);
     hf_printf("LR [R14]\t\t= 0x%.8x\r\n", stacked_lr);
     hf_printf("PC [R15]\t\t= 0x%.8x\r\n", stacked_pc);
     hf_printf("PSR\t\t= 0x%.8x\r\n", stacked_psr);
     hf_printf("BFAR\t\t= 0x%.8x\r\n", (*((volatile unsigned long *)(0xE000ED38))));
     hf_printf("CFSR\t\t= 0x%.8x\r\n", (*((volatile unsigned long *)(0xE000ED28))));
     hf_printf("HFSR\t\t= 0x%.8x\r\n", (*((volatile unsigned long *)(0xE000ED2C))));
     hf_printf("AFSR\t\t= 0x%.8x\r\n", (*((volatile unsigned long *)(0xE000ED3C))));
     hf_printf("SCB_SHCSR\t= 0x%.8x\r\n", SCB->SHCSR);
 
     while (1)
       ;
   }
 #endif
 
   void HardFault_Handler(void)
   {
 #if defined(DEBUG)
     __asm("TST LR, #4");
     __asm("ITE EQ");
     __asm("MRSEQ R0, MSP");
     __asm("MRSNE R0, PSP");
     __asm("B _general_exception_handler");
 #else
     NVIC_SystemReset();
 #endif
   }
 } // extern "C" {