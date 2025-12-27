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

  // Включаем индикатор питания при инициализации
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
  // PWR_ON управляет EN пином TPS63060
  GPIO_ResetBits(PWR_ON_GPIO, PWR_ON_GPIO_PIN);
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

  // Для TBS TANGO: включаем только индикатор питания
  // Питание управляется аппаратно через кнопку Power_SW (PA3)
  pwrOn();
}
