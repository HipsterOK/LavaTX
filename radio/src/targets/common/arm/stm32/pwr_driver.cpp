#include "opentx.h"

void pwrInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // --- включаем тактирование GPIOA и GPIOB (PA3 — кнопка, PB12 — питание) ---
  // GPIOB уже включен в SystemInit(), но включаем для надежности
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

  // --- PWR_ON (PB12) — управление EN пином TPS63060DSCR ---
  GPIO_InitStructure.GPIO_Pin   = PWR_ON_GPIO_PIN;   // PB12
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;     // Input floating (high-Z)
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; // No pull-up/down
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // PB12 в high-Z состоянии - TPS63060DSCR EN pin использует внутренний pull-down
  TRACE("PWR_INIT: PB12 set to INPUT FLOATING (high-Z) - TPS63060DSCR EN uses internal pull-down\n");

  // --- PWR_SWITCH (PA3) — кнопка включения ---
  GPIO_InitStructure.GPIO_Pin   = PWR_SWITCH_GPIO_PIN; // PA3
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;        // pull-up
  GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);
}

void pwrOn()
{
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

  TRACE("PWR_ON: PB12 configured as PUSH-PULL OUTPUT with PULL-UP, set to HIGH\n");

  // Проверяем уровень
  TRACE("PWR_ON: PB12 output level = %d (should be 1)\n",
        GPIO_ReadOutputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));
}

void pwrOff()
{
  // Для TPS63060DSCR: устанавливаем PB12 как input floating (high-Z)
  // EN pin TPS63060DSCR имеет внутренний pull-down ~100kOhm до GND
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;        // Input floating
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // High-Z, используем pull-down TPS63060
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  TRACE("PWR_OFF: PB12 set to INPUT FLOATING (high-Z) - TPS63060DSCR EN uses internal pull-down\n");

  // Проверяем уровень на PB12 (должен быть LOW от pull-down TPS63060)
  TRACE("PWR_OFF: PB12 pin level = %d (should be 0 from TPS63060 internal pull-down)\n",
        GPIO_ReadInputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));

  // Задержка для shutdown TPS63060DSCR
  volatile uint32_t delay = 100000; // ~10ms для надежного shutdown
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
