#include "opentx.h"

// Настройка пинов под STM32F4 и твою плату
#define LCD_SPI SPI1
#define LCD_SPI_GPIO GPIOA
#define LCD_MOSI_GPIO_PIN GPIO_Pin_7
#define LCD_MOSI_PinSource GPIO_PinSource7
#define LCD_CLK_GPIO_PIN GPIO_Pin_5
#define LCD_CLK_PinSource GPIO_PinSource5

#define LCD_NCS_GPIO GPIOA
#define LCD_NCS_GPIO_PIN GPIO_Pin_2
#define LCD_DC_GPIO GPIOA
#define LCD_DC_GPIO_PIN GPIO_Pin_1
#define LCD_RST_GPIO GPIOA
#define LCD_RST_GPIO_PIN GPIO_Pin_0

#define LCD_CS_HIGH() (LCD_NCS_GPIO->BSRRL = LCD_NCS_GPIO_PIN)
#define LCD_CS_LOW()  (LCD_NCS_GPIO->BSRRH = LCD_NCS_GPIO_PIN)
#define LCD_DC_HIGH() (LCD_DC_GPIO->BSRRL = LCD_DC_GPIO_PIN)
#define LCD_DC_LOW()  (LCD_DC_GPIO->BSRRH = LCD_DC_GPIO_PIN)
#define LCD_RST_HIGH() (LCD_RST_GPIO->BSRRL = LCD_RST_GPIO_PIN)
#define LCD_RST_LOW()  (LCD_RST_GPIO->BSRRH = LCD_RST_GPIO_PIN)

#define SPI_TIMEOUT 10000000UL

volatile bool lcd_on = false;
volatile bool lcd_busy = false;

// --- SPI write byte ---
static void spiWrite(uint8_t byte)
{
  while ((LCD_SPI->SR & SPI_SR_TXE) == 0);
  LCD_SPI->DR = byte;
  while ((LCD_SPI->SR & SPI_SR_BSY));
  (void)LCD_SPI->DR; // Dummy read
}

void lcdDmaInit(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  DMA_DeInit(LCD_DMA_Stream);

  DMA_InitTypeDef DMA_InitStructure;
  DMA_InitStructure.DMA_Channel = LCD_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(LCD_SPI->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)displayBuf;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 128; // Один page (128 байт)
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_ITConfig(LCD_DMA_Stream, DMA_IT_TC, ENABLE);
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(LCD_DMA_Stream, &DMA_InitStructure);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

// --- Commands/Data wrappers ---
static void spiWriteCommand(uint8_t cmd)
{
  LCD_DC_LOW();
  LCD_CS_LOW();
  spiWrite(cmd);
  LCD_CS_HIGH();
}

static void spiWriteData(uint8_t data)
{
  LCD_DC_HIGH();
  LCD_CS_LOW();
  spiWrite(data);
  LCD_CS_HIGH();
}

// --- Hardware init ---
void lcdHardwareInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  GPIO_PinAFConfig(LCD_SPI_GPIO, LCD_CLK_PinSource, GPIO_AF_SPI1);
  GPIO_PinAFConfig(LCD_SPI_GPIO, LCD_MOSI_PinSource, GPIO_AF_SPI1);

  GPIO_InitStructure.GPIO_Pin = LCD_CLK_GPIO_PIN | LCD_MOSI_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_NCS_GPIO_PIN | LCD_DC_GPIO_PIN | LCD_RST_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_NCS_GPIO, &GPIO_InitStructure);

  LCD_CS_HIGH();
  LCD_DC_HIGH();
  LCD_RST_HIGH();

  SPI_I2S_DeInit(LCD_SPI);
  SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 10;
  SPI_Init(LCD_SPI, &SPI_InitStructure);
  SPI_Cmd(LCD_SPI, ENABLE);

  lcdDmaInit();

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIOD->BSRRH = GPIO_Pin_9;
}

void lcdClearBuffer(void)
{
  memset(displayBuf, 0x00, sizeof(displayBuf));
}

void delay_ms(int ms)
{
  volatile int i, j;
  for (i = 0; i < ms; i++)
    for (j = 0; j < 30000; j++)
      __asm volatile("nop");
}

// --- Reset display ---
void lcdReset(void)
{
  LCD_RST_HIGH();
  delay_ms(200);
  LCD_RST_LOW();
  delay_ms(200);
  LCD_RST_HIGH();
  delay_ms(200);
}

// --- Init display ---
void lcdDisplayInit(void)
{
  spiWriteCommand(0xAE); // Display OFF
  spiWriteCommand(0xD5);
  spiWriteCommand(0x80); // Clock
  spiWriteCommand(0xA8);
  spiWriteCommand(0x3F); // Multiplex
  spiWriteCommand(0xD3);
  spiWriteCommand(0x00);
  spiWriteCommand(0x40);
  spiWriteCommand(0x8D);
  spiWriteCommand(0x14);
  spiWriteCommand(0x20);
  spiWriteCommand(0x02);
  spiWriteCommand(0xA1);
  spiWriteCommand(0xC8);
  spiWriteCommand(0xDA);
  spiWriteCommand(0x12);
  spiWriteCommand(0x81);
  spiWriteCommand(0x7F);
  spiWriteCommand(0xD9);
  spiWriteCommand(0xF1);
  spiWriteCommand(0xDB);
  spiWriteCommand(0x40);
  spiWriteCommand(0xA4);
  spiWriteCommand(0xA6);
  spiWriteCommand(0xAF); // Display ON
}

#define SSD1306_X_OFFSET 0

void lcdRefresh(bool wait)
{
  for (uint8_t page = 0; page < 8; page++)
  {
    LCD_CS_LOW();
    LCD_DC_LOW();

    spiWrite(0xB0 | page);
    spiWrite(0x00 | (SSD1306_X_OFFSET & 0x0F));
    spiWrite(0x10 | ((SSD1306_X_OFFSET >> 4) & 0x0F));

    while (LCD_SPI->SR & SPI_SR_BSY);

    LCD_DC_HIGH();

    DMA_Cmd(LCD_DMA_Stream, DISABLE);
    DMA_ClearFlag(LCD_DMA_Stream, LCD_DMA_FLAGS);

    LCD_DMA_Stream->M0AR = (uint32_t)&displayBuf[page * 128];
    LCD_DMA_Stream->NDTR = 128;
    lcd_busy = true;

    SPI_I2S_DMACmd(LCD_SPI, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(LCD_DMA_Stream, ENABLE);

    while (lcd_busy);
  }
}

// --- Initialize LCD ---
void lcdInit(void)
{
  DEBUG_TIMER_START(debugTimerPerMain1);

  // Очистим буфер перед инициализацией
  memset(displayBuf, 0x00, sizeof(displayBuf));

  lcdHardwareInit();
  lcdReset();
  lcdDisplayInit();

  // Очистим экран через DMA
  lcdRefresh(true);

  delay_ms(200);
}

// --- Turn LCD on ---
void lcdOn(void)
{
  if (!lcd_on)
  {
    LCD_CS_LOW();
    spiWriteCommand(0xAF);
    LCD_CS_HIGH();
    lcd_on = true;
  }
}

// --- Turn LCD off ---
void lcdOff(void)
{
  if (lcd_on)
  {
    LCD_CS_LOW();
    spiWriteCommand(0xAE);
    LCD_CS_HIGH();
    lcdRefresh(true);
    lcd_on = false;
  }
}

void spiWriteCommandWithArg(uint8_t cmd, uint8_t arg)
{
  LCD_DC_LOW();
  LCD_CS_LOW();
  spiWrite(cmd);
  LCD_DC_HIGH();
  spiWrite(arg);
  LCD_CS_HIGH();
}

void lcdAdjustContrast(uint8_t val)
{
  LCD_CS_LOW();
  LCD_DC_LOW();
  spiWriteCommandWithArg(0x81, val);
  LCD_CS_HIGH();
}

void lcdRefreshWait(void)
{
  while (lcd_busy);
}

void lcdSetRefVolt(uint8_t val)
{
  (void)val;
}

extern "C" void DMA2_Stream5_IRQHandler(void)
{
  if (DMA_GetFlagStatus(DMA2_Stream5, DMA_FLAG_TCIF5) != RESET)
  {
    DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF5);

    while (SPI1->SR & SPI_SR_BSY);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
    DMA_Cmd(DMA2_Stream5, DISABLE);

    LCD_CS_HIGH();
    lcd_busy = false;
  }
}