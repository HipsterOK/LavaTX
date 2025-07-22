#ifndef HALL90393_H
#define HALL90393_H

#include <stdint.h>

// Пины управления CS
#define HALL_CS1_PORT   GPIOD
#define HALL_CS1_PIN    GPIO_Pin_3
#define HALL_CS2_PORT   GPIOD
#define HALL_CS2_PIN    GPIO_Pin_6

// SPI
#define HALL_SPI        SPI2

void hall90393_spi_init(void);
void hall90393_cs_init(void);
void hall90393_cs_select(uint8_t cs);     // 1 или 2
void hall90393_cs_release(uint8_t cs);    // 1 или 2
void hall90393_reset_spi(uint8_t cs);
uint8_t hall90393_spi_transfer(uint8_t data);
void hall90393_read_xyz(uint8_t cs, int16_t* x, int16_t* y, int16_t* z);
void hall90393_lazy_init();
void hall90393_spi_loopback_test();
void hall90393_force_spi_mode(void);

#endif // HALL90393_H
