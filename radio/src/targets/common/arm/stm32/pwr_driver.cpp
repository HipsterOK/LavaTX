#include "opentx.h"

void pwrInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // --- включаем тактирование GPIOA и GPIOB (PA3 — кнопка, PB12 — питание) ---
  // GPIOB уже включен в SystemInit(), но включаем для надежности
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

  // --- PWR_ON (PB12) — управление EN пином TPS63060 ---
  GPIO_InitStructure.GPIO_Pin   = PWR_ON_GPIO_PIN;   // PB12
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; // Open-drain для надежного LOW
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN; // Pull-down для LOW
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // Начинаем с ВЫКЛЮЧЕННОГО питания для теста TPS63060DSCR
  GPIO_ResetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  TRACE("PWR_INIT: PB12 initialized as OPEN-DRAIN OUTPUT, set to LOW (power OFF)\n");

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
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // Push-pull
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // Устанавливаем HIGH для включения питания
  GPIO_SetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  TRACE("PWR_ON: PB12 reconfigured as PUSH-PULL OUTPUT, set to HIGH\n");
}

void pwrOff()
{
  // Для TPS63060DSCR пробуем open-drain output для гарантированного LOW
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; // Open-drain
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // Pull-down для LOW
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // Устанавливаем LOW через open-drain
  GPIO_ResetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  TRACE("PWR_OFF: PB12 configured as OPEN-DRAIN OUTPUT, set to LOW\n");

  // Проверяем уровень на PB12
  TRACE("PWR_OFF: PB12 pin state = %d (should be 0)\n",
        GPIO_ReadOutputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));

  // Увеличиваем задержку для TPS63060DSCR
  volatile uint32_t delay = 50000; // ~5ms для shutdown
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
