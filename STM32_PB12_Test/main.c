#include <stdint.h>

// STM32F4 GPIO registers
#define RCC_AHB1ENR     (*(volatile uint32_t *)0x40023830)
#define GPIOB_MODER     (*(volatile uint32_t *)0x40020400)
#define GPIOB_OTYPER    (*(volatile uint32_t *)0x40020404)
#define GPIOB_OSPEEDR   (*(volatile uint32_t *)0x40020408)
#define GPIOB_PUPDR     (*(volatile uint32_t *)0x4002040C)
#define GPIOB_ODR       (*(volatile uint32_t *)0x40020414)
#define GPIOB_BSRRL     (*(volatile uint32_t *)0x40020418)
#define GPIOB_BSRRH     (*(volatile uint32_t *)0x4002041A)
#define GPIOB_AFRH      (*(volatile uint32_t *)0x40020424)

// SYSCFG for JTAG disable
#define RCC_APB2ENR     (*(volatile uint32_t *)0x40023844)
#define SYSCFG_MEMRMP   (*(volatile uint32_t *)0x40013800)

#define GPIO_PIN_12    (1 << 12)

void delay(uint32_t count) {
    volatile uint32_t i;
    for (i = 0; i < count; i++) {
        __asm("nop");
    }
}

void init_gpio(void) {
    // Enable GPIOB clock
    RCC_AHB1ENR |= (1 << 1);  // GPIOBEN

    // Enable SYSCFG clock for JTAG disable
    RCC_APB2ENR |= (1 << 14); // SYSCFGEN

    // Disable JTAG, enable SWD only
    // MEMRMP[2:1] = 10 (JTAG disabled, SWD enabled)
    SYSCFG_MEMRMP &= ~(0x3 << 1);
    SYSCFG_MEMRMP |= (0x2 << 1);

    // Configure PB12 as output
    GPIOB_MODER &= ~(0x3 << (12 * 2));     // Clear mode
    GPIOB_MODER |= (0x1 << (12 * 2));      // Output mode

    GPIOB_OTYPER &= ~(0x1 << 12);          // Push-pull
    GPIOB_OSPEEDR &= ~(0x3 << (12 * 2));   // Low speed
    GPIOB_PUPDR &= ~(0x3 << (12 * 2));     // No pull
    GPIOB_AFRH &= ~(0xF << ((12-8) * 4));  // AF0 (GPIO)
}

int main(void) {
    init_gpio();

    // Test sequence: toggle PB12 every 1 second
    while (1) {
        // HIGH for 1 second
        GPIOB_BSRRL = GPIO_PIN_12;
        delay(168000000 / 1000);  // ~1 second at 168MHz

        // LOW for 1 second
        GPIOB_BSRRH = GPIO_PIN_12;
        delay(168000000 / 1000);  // ~1 second at 168MHz
    }

    return 0;
}
