#include "opentx.h"

// Прямой доступ к регистрам для надежности
#define GPIOB_BASE        (0x40020400UL)
#define GPIOB_MODER       (*((volatile uint32_t*)(GPIOB_BASE + 0x00)))
#define GPIOB_OTYPER      (*((volatile uint32_t*)(GPIOB_BASE + 0x04)))
#define GPIOB_OSPEEDR     (*((volatile uint32_t*)(GPIOB_BASE + 0x08)))
#define GPIOB_PUPDR       (*((volatile uint32_t*)(GPIOB_BASE + 0x0C)))
#define GPIOB_IDR         (*((volatile uint32_t*)(GPIOB_BASE + 0x10)))
#define GPIOB_ODR         (*((volatile uint32_t*)(GPIOB_BASE + 0x14)))
#define GPIOB_BSRRL       (*((volatile uint32_t*)(GPIOB_BASE + 0x18)))
#define GPIOB_BSRRH       (*((volatile uint32_t*)(GPIOB_BASE + 0x1A)))
#define GPIOB_LCKR        (*((volatile uint32_t*)(GPIOB_BASE + 0x1C)))
#define GPIOB_AFR0        (*((volatile uint32_t*)(GPIOB_BASE + 0x20)))
#define GPIOB_AFR1        (*((volatile uint32_t*)(GPIOB_BASE + 0x24)))

#define RCC_AHB1ENR       (*((volatile uint32_t*)0x40023830))
#define RCC_AHB1ENR_GPIOBEN (1 << 1)

void pwrInit()
{
  // ТЕСТИРУЕМ PB13 вместо PB12 (PB13 работает с MLX датчиком)
  // Если PB13 переключается - GPIO работает, проблема только с PB12
  // Если PB13 не переключается - проблема со всем GPIO STM32

  // Включаем GPIOB clock
  RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  // Настраиваем PB13 как output (он должен работать)
  GPIOB_MODER &= ~(3 << 26);    // MODER13 = 00 (input)
  GPIOB_OTYPER &= ~(1 << 13);   // OT13 = 0 (push-pull)
  GPIOB_PUPDR &= ~(3 << 26);    // PUPDR13 = 00 (no pull)
  GPIOB_AFR1 &= ~(0xF << 20);   // AFR13 = 0000 (GPIO function)

  // Небольшая задержка
  volatile uint32_t i;
  for (i = 0; i < 10000; i++) { __ASM volatile("nop"); }

  // Настраиваем PB13 как output
  GPIOB_MODER |= (1 << 26);     // MODER13 = 01 (output)

  // Бесконечный цикл тестирования PB13
  // HIGH 0.5 секунды -> LOW 0.5 секунды
  while (1) {
    // HIGH
    GPIOB_ODR |= (1 << 13);

    // Задержка ~0.5 секунды
    for (i = 0; i < 8000000; i++) { __ASM volatile("nop"); }

    // LOW
    GPIOB_ODR &= ~(1 << 13);

    // Задержка ~0.5 секунды
    for (i = 0; i < 8000000; i++) { __ASM volatile("nop"); }
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