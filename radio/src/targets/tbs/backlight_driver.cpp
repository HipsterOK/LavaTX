#include "opentx.h"

#if defined(BACKLIGHT_GPIO) || defined(BUTTONLIGHT_GPIO)

void backlightInit()
{
  RCC_AHB1PeriphClockCmd(BUTTONLIGHT_RCC_AHB1Periph, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin   = BUTTONLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(BUTTONLIGHT_GPIO, &GPIO_InitStructure);

  GPIO_SetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN); // Начинаем с HIGH (выключено)
}

void backlightEnable(uint8_t level)
{
  // Для TBS подсветка управляется инвертированной логикой
  // level > 0 = включить (LOW), level = 0 = выключить (HIGH)
  if (level > 0) {
    GPIO_ResetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN); // ВКЛ (LOW)
  } else {
    GPIO_SetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);   // ВЫКЛ (HIGH)
  }
}

void backlightDisable()
{
  GPIO_SetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN); // ВЫКЛ (HIGH)
}

uint8_t isBacklightEnabled()
{
  return GPIO_ReadOutputDataBit(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);
}

#endif // BACKLIGHT_GPIO || BUTTONLIGHT_GPIO
