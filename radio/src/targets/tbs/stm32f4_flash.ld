ENTRY(Reset_Handler)

_estack = 0x20020000;      /* Верхняя граница SRAM (128KB) */
_heap_end = _estack;       /* Конец кучи */

_Min_Heap_Size = 0;
_Main_Stack_Size = 0x1000; /* 4 KB */

_main_stack_start = _estack - _Main_Stack_Size;

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  RAM   (xrw): ORIGIN = 0x20000000, LENGTH = 128K
  CCM   (xrw): ORIGIN = 0x10000000, LENGTH = 64K
}

SECTIONS
{
  /* Вектор прерываний и код в FLASH */
   .text :
  {
    FILL(0xFF)
    CREATE_OBJECT_SYMBOLS
    KEEP(*(.bootloader))
    /* зарезервировать под загрузчик первые 0xC000 байт */
    /* . = ORIGIN(FLASH) + 0xC000;  = 0x0800C000 */
    . = ALIGN(4);
    _stext = .;
    KEEP(*(.isr_vector))
    KEEP(*(.fwversiondata))
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH

  .ARM.exidx :
{
  __exidx_start = .;
  *(.ARM.exidx*)
  __exidx_end = .;
  . = ALIGN(4);
} >FLASH

  /* Инициализация массивов с данными */
_sidata = .;

  /* Инициализируемые данные в SRAM */
  .data : AT (_sidata)
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM

  /* Неинициализированные данные в SRAM */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM

  _end = ALIGN(4);

  /* Куча и стек в SRAM */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    . = . + _Min_Heap_Size;
    . = . + _Main_Stack_Size;
    . = ALIGN(4);
  } >RAM

  /* CCM память (если используется) */
  .ccmram (NOLOAD) :
  {
    . = ALIGN(4);
    *(.ccmram)
    *(.ccmram*)
    . = ALIGN(4);
  } >CCM

  /DISCARD/ :
  {
    libc.a (*)
    libm.a (*)
    libgcc.a (*)
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }

  PROVIDE(_heap_end = ORIGIN(RAM) + LENGTH(RAM));
}