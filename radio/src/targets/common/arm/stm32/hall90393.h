#pragma once
#include <stdint.h>
#include <stdbool.h>

// --- Пины CS ---
#define HALL_CS1_PORT  GPIOD
#define HALL_CS2_PORT  GPIOD
#define HALL_CS1_PIN   GPIO_Pin_3
#define HALL_CS2_PIN   GPIO_Pin_6
#define HALL_SPI       SPI2

#ifdef __cplusplus
extern "C" {
#endif

void hall90393_lazy_init(void);
void hall90393_cs_init(void);
void hall90393_spi_init(void);
void hall90393_force_spi_mode(void);
void hall90393_reset_spi(uint8_t cs);
uint8_t hall90393_spi_transfer(uint8_t data);

void hall90393_cs_select(uint8_t cs);
void hall90393_cs_release(uint8_t cs);

void hall90393_read_xyz(uint8_t cs, int16_t* x, int16_t* y, int16_t* z);
void hall90393_spi_loopback_test(void);

#ifdef __cplusplus
}
#endif
