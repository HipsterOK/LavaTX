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

#if defined(RADIO_TANGO)
// Программное управление яркостью подсветки кнопок
static uint8_t currentBacklightLevel = 0;
static uint8_t backlightPwmCounter = 0;

void backlightUpdate()
{
  backlightPwmCounter++;
  if (backlightPwmCounter >= 10) { // 100Hz PWM
    backlightPwmCounter = 0;
    if (currentBacklightLevel > 0 && backlightPwmCounter < currentBacklightLevel) {
      GPIO_ResetBits(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PIN); // ВКЛ (инвертированная логика)
    } else {
      GPIO_SetBits(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PIN);   // ВЫКЛ
    }
  }
}
#endif
#if defined(RADIO_TANGO)
void backlightInit(void)
{
  // Для RADIO_TANGO подсветка управляется через GPIO с программным PWM
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);

  currentBacklightLevel = 0;
  backlightPwmCounter = 0;
}

void backlightDisable(void)
{
  currentBacklightLevel = 0;
  GPIO_SetBits(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PIN); // Инвертированная логика для подсветки кнопок
}

void backlightEnable(uint8_t level)
{
  // Программное управление яркостью через PWM
  // level от 0 до 100, преобразуем в шкалу 0-10 для PWM
  currentBacklightLevel = level / 10; // 0-10 диапазон
  if (currentBacklightLevel > 10) currentBacklightLevel = 10;
}

uint8_t isBacklightEnabled(void)
{
  return currentBacklightLevel > 0;
}
#elif defined(RADIO_MAMBO)
void backlightInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
  GPIO_PinAFConfig(BACKLIGHT_GPIO, BACKLIGHT_GPIO_PinSource, BACKLIGHT_GPIO_AF);
  BACKLIGHT_TIMER->ARR = 100;
  BACKLIGHT_TIMER->PSC = BACKLIGHT_TIMER_FREQ / 50000 - 1; // 20us * 100 = 2ms => 500Hz
  BACKLIGHT_TIMER->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2; // PWM
  BACKLIGHT_TIMER->CCER = TIM_CCER_CC4E | TIM_CCER_CC2E;
  BACKLIGHT_COUNTER_REGISTER = 0;
  BACKLIGHT_TIMER->EGR = 0;
  BACKLIGHT_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
}

void backlightEnable(uint8_t level)
{
  BACKLIGHT_COUNTER_REGISTER = 100 - level;
}

void backlightDisable()
{
  BACKLIGHT_COUNTER_REGISTER = 0;
}

uint8_t isBacklightEnabled()
{
  return BACKLIGHT_COUNTER_REGISTER != 0;
}
#endif
