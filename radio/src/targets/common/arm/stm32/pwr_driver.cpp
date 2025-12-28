#include "opentx.h"

void pwrInit()
{
  // ПРОСТОЙ ТЕСТ PB12: только GPIO, без OpenTX инициализации
  // Настраиваем PB12 и переключаем в бесконечном цикле
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // Настраиваем PB12 как push-pull output
  GPIOB->MODER &= ~(GPIO_MODER_MODER12);     // Сбрасываем
  GPIOB->MODER |= GPIO_MODER_MODER12_0;      // Output mode
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_12);     // Push-pull
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR12);     // No pull

  // Бесконечный цикл переключения PB12
  // HIGH 1 секунда -> LOW 1 секунда -> повтор
  while (1) {
    // HIGH
    GPIOB->BSRRL = GPIO_Pin_12;

    // Задержка ~1 секунда (при 16MHz системной частоте)
    volatile uint32_t delay = 16000000;
    while (delay--) { __ASM volatile("nop"); }

    // LOW
    GPIOB->BSRRH = GPIO_Pin_12;

    // Задержка ~1 секунда
    delay = 16000000;
    while (delay--) { __ASM volatile("nop"); }
  }
}

void pwrOn()
{
  // Пустая функция для совместимости
}

void pwrOff()
{
  // Пустая функция для совместимости
}

bool pwrPressed()
{
  // Всегда возвращаем false (не нажата)
  return false;
}

void pwrResetHandler()
{
  // Пустая функция для совместимости
}