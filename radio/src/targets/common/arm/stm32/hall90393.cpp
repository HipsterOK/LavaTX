#include "hall90393.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "opentx.h"

static bool hall90393_is_inited = false;

void hall90393_lazy_init(void)
{
    if (!hall90393_is_inited) {
        for (volatile int i = 0; i < 500000; ++i) __NOP();
        hall90393_force_spi_mode();
        hall90393_cs_init();
        hall90393_spi_init();
        hall90393_reset_spi(1);
        for (volatile int i = 0; i < 10000; ++i) __NOP();
        hall90393_is_inited = true;

        // Необязательно: hall90393_spi_loopback_test();
    }
}

void hall90393_spi_loopback_test(void)
{
    uint8_t sent = 0xA5;
    uint8_t received = 0;
    char buf[32];
    for (int i = 0; i < 10; ++i) {
        received = hall90393_spi_transfer(sent);
        snprintf(buf, sizeof(buf), "Send:%02X, Recv:%02X", sent, received);
        // ALERT("SPI LOOP", buf, 0); // для отладки, если нужно
        sent++;
    }
}

void hall90393_force_spi_mode(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = HALL_CS1_PIN | HALL_CS2_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &gpio);

    GPIO_ResetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    GPIO_ResetBits(HALL_CS2_PORT, HALL_CS2_PIN);
    for (volatile int i = 0; i < 50000; ++i) __NOP();

    GPIO_SetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);
    for (volatile int i = 0; i < 10000; ++i) __NOP();
}

void hall90393_cs_init(void) {
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    gpio.GPIO_Pin = HALL_CS1_PIN | HALL_CS2_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &gpio);

    GPIO_SetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);
}

void hall90393_spi_init(void) {
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    SPI_I2S_DeInit(SPI2);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_High;
    spi.SPI_CPHA = SPI_CPHA_2Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &spi);
    SPI_Cmd(SPI2, ENABLE);
}

void hall90393_reset_spi(uint8_t cs)
{
    hall90393_cs_select(cs);
    hall90393_spi_transfer(0xF0);   // Reset command
    hall90393_cs_release(cs);
    for (volatile int i = 0; i < 10000; ++i) __NOP();
}

void hall90393_cs_select(uint8_t cs) {
    if (cs == 1) {
        GPIO_ResetBits(HALL_CS1_PORT, HALL_CS1_PIN);
        GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);
    } else {
        GPIO_SetBits(HALL_CS1_PORT, HALL_CS1_PIN);
        GPIO_ResetBits(HALL_CS2_PORT, HALL_CS2_PIN);
    }
}

void hall90393_cs_release(uint8_t cs) {
    if (cs == 1)
        GPIO_SetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    else
        GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);
}

uint8_t hall90393_spi_transfer(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(HALL_SPI, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(HALL_SPI, data);
    while (SPI_I2S_GetFlagStatus(HALL_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(HALL_SPI);
}

void hall90393_read_xyz(uint8_t cs, int16_t* x, int16_t* y, int16_t* z) {
    // Запусти одиночное измерение
    hall90393_cs_select(cs);
    hall90393_spi_transfer(0x3F);
    hall90393_spi_transfer(0x0F);
    hall90393_cs_release(cs);

    for (volatile int i=0; i<2000; ++i) __NOP(); // ~3мкс задержка

    // Теперь считываем результат
    hall90393_cs_select(cs);
    hall90393_spi_transfer(0x4E);
    for (volatile int i=0; i<100; ++i) __NOP();

    uint8_t buf[7];
    for (int i = 0; i < 7; i++)
        buf[i] = hall90393_spi_transfer(0x00);

    hall90393_cs_release(cs);

    if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF) {
        *x = *y = *z = -1024;
        return;
    }
    if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00) {
        *x = *y = *z = -1024;
        return;
    }

    *x = (int16_t)((buf[1] << 8) | buf[2]);
    *y = (int16_t)((buf[3] << 8) | buf[4]);
    *z = (int16_t)((buf[5] << 8) | buf[6]);
}
