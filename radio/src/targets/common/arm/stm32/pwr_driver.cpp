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
  // Включаем GPIOB clock через прямой доступ к RCC
  RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  // Проверяем что clock включился (RCC_AHB1ENR bit 1)
  volatile uint32_t rcc_check = RCC_AHB1ENR & RCC_AHB1ENR_GPIOBEN;
  (void)rcc_check; // Для отладки

  // Сбрасываем все настройки PB12
  GPIOB_MODER &= ~(3 << 24);    // MODER12 = 00 (input)
  GPIOB_OTYPER &= ~(1 << 12);   // OT12 = 0 (push-pull)
  GPIOB_OSPEEDR &= ~(3 << 24);  // OSPEEDR12 = 00 (low speed)
  GPIOB_PUPDR &= ~(3 << 24);    // PUPDR12 = 00 (no pull)
  GPIOB_AFR1 &= ~(0xF << 16);   // AFR12 = 0000 (GPIO function)

  // Небольшая задержка для стабилизации
  volatile uint32_t i;
  for (i = 0; i < 10000; i++) { __ASM volatile("nop"); }

  // Настраиваем PB12 как output push-pull
  GPIOB_MODER |= (1 << 24);     // MODER12 = 01 (output)

  // Проверяем настройки
  volatile uint32_t moder_check = GPIOB_MODER;
  (void)moder_check; // Для отладки

  // Бесконечный цикл тестирования PB12
  // HIGH 0.5 секунды -> LOW 0.5 секунды
  while (1) {
    // HIGH - установить бит 12 в ODR
    GPIOB_ODR |= (1 << 12);

    // Проверяем что установилось
    volatile uint32_t high_check = GPIOB_ODR & (1 << 12);
    (void)high_check; // Для отладки

    // Задержка ~0.5 секунды
    for (i = 0; i < 8000000; i++) { __ASM volatile("nop"); }

    // LOW - сбросить бит 12 в ODR
    GPIOB_ODR &= ~(1 << 12);

    // Проверяем что сбросилось
    volatile uint32_t low_check = GPIOB_ODR & (1 << 12);
    (void)low_check; // Для отладки

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