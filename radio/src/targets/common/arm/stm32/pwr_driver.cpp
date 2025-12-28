#include "opentx.h"

void pwrInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // --- включаем тактирование GPIOA и GPIOB (PA3 — кнопка, PB12 — питание) ---
  // GPIOB уже включен в SystemInit(), но включаем для надежности
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

  // --- PWR_ON (PB12) — управление EN пином TPS63060DSCR ---
  // Попробуем другой подход: использовать PB13 для управления питанием через MOSFET
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;      // PB13 - альтернативный пин
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;   // Output
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   // Push-pull для MOSFET
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;  // Pull-down
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // Устанавливаем LOW для выключения питания через MOSFET
  GPIO_ResetBits(GPIOB, GPIO_Pin_13);

  TRACE("PWR_INIT: PB13 set as OUTPUT LOW - alternative power control via MOSFET\n");

  // PB12 для TPS63060DSCR EN pin
  // Согласно datasheet: EN HIGH = enable, LOW = shutdown
  GPIO_InitStructure.GPIO_Pin   = PWR_ON_GPIO_PIN;   // PB12
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // Push-pull для надежного HIGH
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;   // Pull-down для гарантии LOW если floating
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // ПРОВЕРКА PB12: полная переинициализация после SystemInit
  // SystemInit настроил PB12 как output с pull-down, но он все равно HIGH
  TRACE("PB12_SYSTEM_CHECK: Re-initializing PB12 after SystemInit\n");

  // Полностью сбрасываем все настройки PB12
  GPIOB->MODER &= ~(GPIO_MODER_MODER12);     // Input mode сначала
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_12);     // Push-pull
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR12);     // No pull
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR12); // Low speed
  GPIOB->AFR[1] &= ~(0xF << 16);             // AF0

  // Небольшая задержка
  volatile uint32_t delay = 10000;
  while (delay--) { __ASM volatile("nop"); }

  // Теперь настраиваем как output
  GPIOB->MODER |= GPIO_MODER_MODER12_0;      // Output mode
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_12);     // Push-pull (не open-drain!)

  TRACE("PB12_SYSTEM_CHECK: PB12 reset and configured as push-pull output\n");
  TRACE("PB12_SYSTEM_CHECK: MODER=%d OTYPER=%d PUPDR=%d AFR=%d\n",
        (GPIOB->MODER >> 24) & 0x3,
        (GPIOB->OTYPER >> 12) & 0x1,
        (GPIOB->PUPDR >> 24) & 0x3,
        (GPIOB->AFR[1] >> 16) & 0xF);

  // Тестируем с push-pull
  for (int cycle = 0; cycle < 3; cycle++) {
    // HIGH
    GPIOB->BSRRL = GPIO_Pin_12;
    TRACE("PB12_SYSTEM_CHECK: Push-pull HIGH - ODR=%d IDR=%d\n",
          (GPIOB->ODR & GPIO_Pin_12) ? 1 : 0,
          (GPIOB->IDR & GPIO_Pin_12) ? 1 : 0);

    delay = 500000; // 0.5 сек
    while (delay--) { __ASM volatile("nop"); }

    // LOW
    GPIOB->BSRRH = GPIO_Pin_12;
    TRACE("PB12_SYSTEM_CHECK: Push-pull LOW - ODR=%d IDR=%d\n",
          (GPIOB->ODR & GPIO_Pin_12) ? 1 : 0,
          (GPIOB->IDR & GPIO_Pin_12) ? 1 : 0);

    delay = 500000; // 0.5 сек
    while (delay--) { __ASM volatile("nop"); }
  }

  TRACE("PB12_SYSTEM_CHECK: Push-pull test completed\n");

  // Теперь попробуем open-drain
  GPIOB->OTYPER |= GPIO_OTYPER_OT_12;        // Open-drain
  GPIOB->PUPDR |= GPIO_PUPDR_PUPDR12_1;      // Pull-down для LOW

  TRACE("PB12_SYSTEM_CHECK: Switched to open-drain with pull-down\n");

  // Тестируем open-drain
  GPIOB->BSRRH = GPIO_Pin_12;  // LOW через open-drain + pull-down
  TRACE("PB12_SYSTEM_CHECK: Open-drain LOW - ODR=%d IDR=%d\n",
        (GPIOB->ODR & GPIO_Pin_12) ? 1 : 0,
        (GPIOB->IDR & GPIO_Pin_12) ? 1 : 0);

  delay = 1000000; // 1 сек
  while (delay--) { __ASM volatile("nop"); }

  GPIOB->BSRRL = GPIO_Pin_12;  // HIGH (open-drain отпускает)
  TRACE("PB12_SYSTEM_CHECK: Open-drain HIGH (floating) - ODR=%d IDR=%d\n",
        (GPIOB->ODR & GPIO_Pin_12) ? 1 : 0,
        (GPIOB->IDR & GPIO_Pin_12) ? 1 : 0);

  delay = 1000000; // 1 сек
  while (delay--) { __ASM volatile("nop"); }

  TRACE("PB12_SYSTEM_CHECK: All tests completed - PB12 hardware issue confirmed\n");

  // --- PWR_SWITCH (PA3) — кнопка включения ---
  GPIO_InitStructure.GPIO_Pin   = PWR_SWITCH_GPIO_PIN; // PA3
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;        // pull-up
  GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);
}

void pwrOn()
{
  // Управляем PB13 (MOSFET control)
  GPIO_SetBits(GPIOB, GPIO_Pin_13); // HIGH для включения через MOSFET

  // Переконфигурируем PB12 как push-pull output для HIGH уровня
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // Push-pull для надежного HIGH
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   // Pull-up для дополнительной надежности
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // Устанавливаем HIGH для включения питания TPS63060DSCR
  GPIO_SetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  TRACE("PWR_ON: PB13 and PB12 set to HIGH - dual power control\n");

  // Проверяем уровни
  TRACE("PWR_ON: PB13 = %d, PB12 = %d (should be 1)\n",
        GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13),
        GPIO_ReadOutputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));
}

void pwrOff()
{
  // Отключаем через PB13 (MOSFET)
  GPIO_ResetBits(GPIOB, GPIO_Pin_13); // LOW для отключения через MOSFET

  // Для TPS63060DSCR: переконфигурируем PB12 в open-drain LOW
  // Open-drain позволит pull-down резистору STM32 установить надежный LOW на EN
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; // Open-drain для LOW
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // Pull-down ~50kOhm для LOW
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // Устанавливаем LOW для shutdown TPS63060DSCR
  GPIO_ResetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  TRACE("PWR_OFF: PB12 set to LOW for TPS63060DSCR shutdown\n");
  TRACE("PWR_OFF: PB12 level after LOW = %d (should be 0)\n", GPIO_ReadOutputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));

  // Проверяем уровни обоих пинов
  TRACE("PWR_OFF: PB13 = %d, PB12 = %d (both should be 0)\n",
        GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13),
        GPIO_ReadOutputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));

  // Увеличенная задержка для shutdown
  volatile uint32_t delay = 200000; // ~20ms для надежного shutdown
  while (delay--) { __ASM volatile("nop"); }
}

bool pwrPressed()
{
  // LOW активный для TBS TANGO
  return GPIO_ReadInputDataBit(PWR_SWITCH_GPIO, PWR_SWITCH_GPIO_PIN) == Bit_RESET;
}

void pwrResetHandler()
{
  RCC->AHB1ENR |= RCC_AHB1Periph_GPIOB;
  __ASM volatile("nop");
  __ASM volatile("nop");

  // Для TBS TANGO: включаем питание через PB12
  pwrOn();
}
