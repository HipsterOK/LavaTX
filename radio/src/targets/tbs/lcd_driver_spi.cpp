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
#define LCD_CS_LOW() (LCD_NCS_GPIO->BSRRH = LCD_NCS_GPIO_PIN)
#define LCD_DC_HIGH() (LCD_DC_GPIO->BSRRL = LCD_DC_GPIO_PIN)
#define LCD_DC_LOW() (LCD_DC_GPIO->BSRRH = LCD_DC_GPIO_PIN)
#define LCD_RST_HIGH() (LCD_RST_GPIO->BSRRL = LCD_RST_GPIO_PIN)
#define LCD_RST_LOW() (LCD_RST_GPIO->BSRRH = LCD_RST_GPIO_PIN)

#define SPI_TIMEOUT 10000000UL

volatile bool lcd_on = false;

// --- SPI write byte ---
static void spiWrite(uint8_t byte)
{
while ((LCD_SPI->SR & SPI_SR_TXE) == 0);
LCD_SPI->DR = byte;
while ((LCD_SPI->SR & SPI_SR_BSY));
(void)LCD_SPI->DR; // Dummy read
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
  LCD_CS_LOW();

  spiWriteCommand(0xAE); // Display OFF
  spiWriteCommand(0xD5);
  spiWriteCommand(0x80); // Clock
  spiWriteCommand(0xA8);
  spiWriteCommand(0x3F); // Multiplex
  spiWriteCommand(0xD3);
  spiWriteCommand(0x00); // Display offset = 0
  spiWriteCommand(0x40); // Start line = 0

  spiWriteCommand(0x8D);
  spiWriteCommand(0x14); // Charge pump
  spiWriteCommand(0x20);
  spiWriteCommand(0x02); // page addressing mode

  spiWriteCommand(0xA1); // Segment remap
  spiWriteCommand(0xC8); // COM scan direction

  spiWriteCommand(0xDA);
  spiWriteCommand(0x12);
  spiWriteCommand(0x81);
  spiWriteCommand(0x7F);
  spiWriteCommand(0xD9);
  spiWriteCommand(0xF1);
  spiWriteCommand(0xDB);
  spiWriteCommand(0x40);

  spiWriteCommand(0xA4); // Resume RAM content
  spiWriteCommand(0xA6); // Normal display
  spiWriteCommand(0xAF); // Display ON

  LCD_CS_HIGH();
}

#define SSD1306_X_OFFSET 0 // Подстройка: сдвиг вправо на 2 пикселя (если нужно)

void lcdRefresh(bool wait)
{
  for (uint8_t page = 0; page < 8; page++)
  {
    LCD_CS_LOW();

    LCD_DC_LOW();
    spiWrite(0xB0 | page);
    spiWrite(0x00 | (SSD1306_X_OFFSET & 0x0F));
    spiWrite(0x10 | ((SSD1306_X_OFFSET >> 4) & 0x0F));
    delay_ms(1);

    LCD_DC_HIGH();

    for (uint8_t col = 0; col < 128; col++)
    {
      spiWrite(displayBuf[page * 128 + col]);
    }

    LCD_CS_HIGH();
  }
}

void lcdRefreshWait(void)
{
    // Simple wait for LCD without DMA
}

// --- Initialize LCD ---
void lcdInit(void)
{

  DEBUG_TIMER_START(debugTimerPerMain1);
  lcdHardwareInit();
  lcdReset();
  lcdDisplayInit();
  // Выводим белый экран вручную
  for (uint8_t page = 0; page < 8; page++)
  {
    LCD_CS_LOW();
    LCD_DC_LOW();
    spiWriteCommand(0xB0 | page);
    spiWriteCommand(0x00);
    spiWriteCommand(0x10);
    LCD_DC_HIGH();
    for (uint8_t col = 0; col < 128; col++)
    {
      spiWrite(0xFF); // белый
    }
    LCD_CS_HIGH();
  }

  delay_ms(500);
}

// --- Turn LCD on ---
void lcdOn(void)
{
  if (!lcd_on)
  {
    LCD_CS_LOW();
    spiWriteCommand(0xAF); // Display ON
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
    spiWriteCommand(0xAE); // Display OFF
    LCD_CS_HIGH();
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
  spiWriteCommandWithArg(0x81, val); // Контрастность
  LCD_CS_HIGH();
}

void lcdSetRefVolt(uint8_t val)
{
  (void)val; // Если не используешь, можно просто заглушить
}