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
#if defined(RADIO_LAVA_ONE) || defined(RADIO_TANGO)

void ledInit(void)
{
  // Для RADIO_LAVA_ONE/RADIO_TANGO подсветка управляется через GPIO
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = BACKLIGHT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(BACKLIGHT_GPIO, &GPIO_InitStructure);
}

// backlight functions moved to backlight_driver.cpp

void ledOff()
{
  // Для RADIO_LAVA_ONE/RADIO_TANGO просто отключаем подсветку
  backlightDisable();
}

void ledRed()
{
  // Для RADIO_LAVA_ONE/RADIO_TANGO включаем подсветку для красного
  backlightEnable(100);
}

void ledGreen()
{
  // Для RADIO_LAVA_ONE/RADIO_TANGO включаем подсветку для зеленого
  backlightEnable(100);
}

void ledBlue()
{
  // Для RADIO_LAVA_ONE/RADIO_TANGO включаем подсветку для синего
  backlightEnable(100);
}
#endif
