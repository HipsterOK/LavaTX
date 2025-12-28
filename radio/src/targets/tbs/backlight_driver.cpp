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
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BUTTONLIGHT_GPIO, &GPIO_InitStructure);

  GPIO_ResetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);
}

void backlightEnable(uint8_t level)
{
  if (level == 0) {
    // Выключить подсветку
    GPIO_ResetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);
  } else {
    // Включить подсветку
    GPIO_SetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);
  }
}

void backlightDisable()
{
  GPIO_ResetBits(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);
}

uint8_t isBacklightEnabled()
{
  return GPIO_ReadOutputDataBit(BUTTONLIGHT_GPIO, BUTTONLIGHT_GPIO_PIN);
}

#endif // BACKLIGHT_GPIO || BUTTONLIGHT_GPIO
