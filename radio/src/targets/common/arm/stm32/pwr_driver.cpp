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
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  // Начинаем с включенного питания для нормальной работы
  // Логика управления будет в boardOff()
  GPIO_SetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);

  // --- PWR_SWITCH (PA3) — кнопка включения ---
  GPIO_InitStructure.GPIO_Pin   = PWR_SWITCH_GPIO_PIN; // PA3
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;        // pull-up
  GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);
}

void pwrOn()
{
  // PWR_ON управляет EN пином TPS63060
  GPIO_SetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
}

void pwrOff()
{
  // Метод: переконфигурируем PB12 как input с pull-down для гарантированного LOW
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // pull-down для гарантированного LOW на EN
  GPIO_Init(PWR_ON_GPIO, &GPIO_InitStructure);

  TRACE("PWR_OFF: PB12 reconfigured as INPUT with PULL-DOWN\n");

  // Проверяем уровень на PB12
  TRACE("PWR_OFF: PB12 pin state = %d (should be 0)\n",
        GPIO_ReadInputDataBit(PWR_ON_GPIO, PWR_ON_GPIO_PIN));

  // Небольшая задержка для стабилизации
  volatile uint32_t delay = 10000;
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
