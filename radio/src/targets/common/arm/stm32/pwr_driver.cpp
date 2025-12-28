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

  // ТЕСТ GPIO: проверяем можем ли мы вообще управлять GPIO
  // PB12 всегда HIGH по данным логического анализатора
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // Тестируем PB13 (который должен работать, там MLX)
  GPIOB->MODER &= ~(GPIO_MODER_MODER13);    // PB13 output
  GPIOB->MODER |= GPIO_MODER_MODER13_0;
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_13);    // Push-pull
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR13);    // No pull
  GPIOB->BSRRH = GPIO_Pin_13;               // LOW

  TRACE("GPIO_TEST: PB13 set to LOW - checking if GPIO works\n");
  TRACE("GPIO_TEST: PB13 ODR = %d, IDR = %d\n",
        (GPIOB->ODR & GPIO_Pin_13) ? 1 : 0,
        (GPIOB->IDR & GPIO_Pin_13) ? 1 : 0);

  // Тестируем PB12 - попробуем разные конфигурации
  GPIOB->MODER &= ~(GPIO_MODER_MODER12);    // Output
  GPIOB->MODER |= GPIO_MODER_MODER12_0;
  GPIOB->OTYPER |= GPIO_OTYPER_OT_12;       // Open-drain
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR12);    // No pull сначала
  GPIOB->BSRRH = GPIO_Pin_12;               // LOW

  TRACE("GPIO_TEST: PB12 set to LOW (open-drain, no pull)\n");
  TRACE("GPIO_TEST: PB12 ODR = %d, IDR = %d\n",
        (GPIOB->ODR & GPIO_Pin_12) ? 1 : 0,
        (GPIOB->IDR & GPIO_Pin_12) ? 1 : 0);

  // Задержка
  volatile uint32_t delay = 500000; // 0.5 сек
  while (delay--) { __ASM volatile("nop"); }

  // Попробуем с pull-down
  GPIOB->PUPDR |= GPIO_PUPDR_PUPDR12_1;     // Pull-down
  TRACE("GPIO_TEST: PB12 with pull-down added\n");
  TRACE("GPIO_TEST: PB12 ODR = %d, IDR = %d\n",
        (GPIOB->ODR & GPIO_Pin_12) ? 1 : 0,
        (GPIOB->IDR & GPIO_Pin_12) ? 1 : 0);

  // Еще задержка
  delay = 500000; // 0.5 сек
  while (delay--) { __ASM volatile("nop"); }

  TRACE("GPIO_TEST: PB12 always HIGH according to logic analyzer - hardware issue\n");

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
