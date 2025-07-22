#include "hall90393.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include <opentx.h>


static bool hall90393_is_inited = false;

void hall90393_lazy_init()
{
    if (!hall90393_is_inited) {
        for (volatile int i = 0; i < 500000; ++i) __NOP();
        hall90393_force_spi_mode();
        hall90393_cs_init();
        hall90393_spi_init(); // ВСЁ! Никаких задержек тут!
        hall90393_reset_spi(1);
        // hall90393_reset_spi(2);
        // Можно чуть подождать (NOP), если нужно <1мс
        for (volatile int i = 0; i < 10000; ++i) __NOP();
        hall90393_is_inited = true;

        hall90393_spi_loopback_test();
    }
    int16_t x1, y1, z1;
hall90393_read_xyz(1, &x1, &y1, &z1);

// Где-нибудь в draw функции/отрисовке экрана:
char buf[32];
lcdDrawText(10, 10, buf, 0); // (x, y, строка, стиль)
}

void hall90393_spi_loopback_test()
{
    uint8_t sent = 0xA5; // Любой нестандартный байт
    uint8_t received = 0;

    // Просто сделаем 10 попыток, чтобы увидеть результат
    for (int i = 0; i < 10; ++i) {
        received = hall90393_spi_transfer(sent);

        // ALERT("SPI LOOPBACK", buf, 0);

        // Измени sent, чтобы убедиться, что MOSI живой
        sent++;
    }
}

void hall90393_force_spi_mode(void)
{
    // Включаем тактирование порта CS
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // CS как GPIO-выход
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = HALL_CS1_PIN | HALL_CS2_PIN; // Если два CS — оба
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &gpio);

    // Держим CS LOW (активный!) хотя бы 3-5мс
    GPIO_ResetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    GPIO_ResetBits(HALL_CS2_PORT, HALL_CS2_PIN);
    for (volatile int i = 0; i < 50000; ++i) __NOP(); // ~3-5мс

    // Отпускаем CS в HIGH
    GPIO_SetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);
    for (volatile int i = 0; i < 10000; ++i) __NOP(); // ~1мс пауза
}

// =======================
// ИНИЦИАЛИЗАЦИЯ
// =======================
void hall90393_cs_init(void) {
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    gpio.GPIO_Pin = HALL_CS1_PIN | HALL_CS2_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &gpio);

    // Отключаем оба CS по умолчанию (лог.1)
    GPIO_SetBits(HALL_CS1_PORT, HALL_CS1_PIN);
    GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);
}

void hall90393_spi_init(void) {
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    // Включаем тактирование
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    // Настраиваем PB13 (SCK), PB14 (MISO), PB15 (MOSI)
    gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);

    // Альтернативная функция для SPI2
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    // Настройка SPI2 (Mode 0, 8 MHz максимум, 8 bit)
    SPI_I2S_DeInit(SPI2);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_High;
    spi.SPI_CPHA = SPI_CPHA_2Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // (8 МГц при 128 МГц)
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
    // Обязательная микрозадержка после reset (внутри функции)
    for (volatile int i = 0; i < 10000; ++i) __NOP(); // ~1-2мс
}


// =======================
// CS управление
// =======================
void hall90393_cs_select(uint8_t cs) {
    if (cs == 1) {
        GPIO_ResetBits(HALL_CS1_PORT, HALL_CS1_PIN); // CS1 активный
        GPIO_SetBits(HALL_CS2_PORT, HALL_CS2_PIN);   // CS2 неактивный!
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

// =======================
// SPI transfer
// =======================
uint8_t hall90393_spi_transfer(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(HALL_SPI, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(HALL_SPI, data);
    while (SPI_I2S_GetFlagStatus(HALL_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(HALL_SPI);
}

// =======================
// Пример чтения XYZ (MLX90393)
// =======================
// Реализация базового "Read Measurement" для MLX90393 (command: 0x4E, потом читаем 7 байт)

void hall90393_read_xyz(uint8_t cs, int16_t* x, int16_t* y, int16_t* z) {
    hall90393_cs_select(cs);
    hall90393_spi_transfer(0x4E);

    for (volatile int i=0; i<100; ++i) __NOP();

    uint8_t buf[7];
    for (int i = 0; i < 7; i++)
        buf[i] = hall90393_spi_transfer(0x00);

    hall90393_cs_release(cs);

    if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF) {
        // ALERT("HALL ERR", "NO SPI RESPONSE!", 0);
        *x = *y = *z = -1024;
        return;
    }
    if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00) {
        // ALERT("HALL ERR", "CS FAILURE!", 0);
        *x = *y = *z = -1024;
        return;
    }

    *x = (int16_t)((buf[1] << 8) | buf[2]);
    *y = (int16_t)((buf[3] << 8) | buf[4]);
    *z = (int16_t)((buf[5] << 8) | buf[6]);

    if (*x == 0 && *y == 0 && *z == 0) {
        // ALERT("HALL ERR", "ZERO XYZ!", 0);
    }
}
