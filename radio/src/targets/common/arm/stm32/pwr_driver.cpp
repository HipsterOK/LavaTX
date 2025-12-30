#include "opentx.h"

// ПРЯМОЙ ДОСТУП К РЕГИСТРАМ GPIO (как в рабочем тестовом проекте)
#define GPIOB_BASE        (0x40020400UL)
#define GPIOB_MODER       (*((volatile uint32_t*)(GPIOB_BASE + 0x00)))
#define GPIOB_OTYPER      (*((volatile uint32_t*)(GPIOB_BASE + 0x04)))
#define GPIOB_OSPEEDR     (*((volatile uint32_t*)(GPIOB_BASE + 0x08)))
#define GPIOB_PUPDR       (*((volatile uint32_t*)(GPIOB_BASE + 0x0C)))
#define GPIOB_IDR         (*((volatile uint32_t*)(GPIOB_BASE + 0x10)))
#define GPIOB_ODR         (*((volatile uint32_t*)(GPIOB_BASE + 0x14)))

#define RCC_AHB1ENR       (*((volatile uint32_t*)0x40023830))
#define RCC_AHB1ENR_GPIOBEN (1 << 1)

void pwrInit()
{
  // ВКЛЮЧАЕМ GPIO CLOCKS
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

  // PB12 уже настроен в SystemInit() как output LOW
  // Здесь мы только настраиваем кнопку питания

  // --- PWR_SWITCH (PA3) — кнопка включения ---
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin   = PWR_SWITCH_GPIO_PIN; // PA3
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;        // pull-up
  GPIO_Init(PWR_SWITCH_GPIO, &GPIO_InitStructure);

  TRACE("PWR_INIT: PB12 left in LOW state (power off)\n");
}

void pwrOn()
{
  // ВКЛЮЧАЕМ GPIOB CLOCK (как в тестовом проекте)
  RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  (void)RCC_AHB1ENR; // барьер для синхронизации

  // ПРЯМОЙ ДОСТУП К РЕГИСТРАМ (как в рабочем тестовом проекте STM32CubeIDE)
  // Устанавливаем PB12 в HIGH через ODR регистр
  GPIOB_ODR |= (1 << 12);   // PB12 HIGH (bit 12)

  TRACE("PWR_ON: PB12 set to HIGH via direct register access\n");

  // Задержка для TPS63060 startup
  volatile uint32_t delay = 50000; // 50ms для гарантии
  while (delay--) { __ASM volatile("nop"); }
}

void pwrOff()
{
  // ВКЛЮЧАЕМ GPIOB CLOCK (как в тестовом проекте)
  RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  (void)RCC_AHB1ENR; // барьер для синхронизации

  // ПРЯМОЙ ДОСТУП К РЕГИСТРАМ (как в рабочем тестовом проекте STM32CubeIDE)
  // Устанавливаем PB12 в LOW через ODR регистр
  GPIOB_ODR &= ~(1 << 12);  // PB12 LOW (bit 12)

  TRACE("PWR_OFF: PB12 set to LOW via direct register access\n");

  // Проверяем уровень на PB12 через IDR
  uint32_t pb12_state = (GPIOB_IDR & (1 << 12)) ? 1 : 0;
  TRACE("PWR_OFF: PB12 pin state = %d (should be 0)\n", pb12_state);

  // Задержка для TPS63060 shutdown
  volatile uint32_t delay = 50000; // 50ms для гарантии
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

  // Питание теперь включается в boardInit() плавно
  // Здесь оставляем PB12 в состоянии после SystemInit() (LOW)
  TRACE("PWR_RESET: PB12 kept in LOW state (power control in boardInit)\n");
}
