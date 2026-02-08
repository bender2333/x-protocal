/* memory map */
MEMORY
{
FLASH (rx)      : ORIGIN = 0x08080400, LENGTH = 1024K
PROGVALID (rx)  : ORIGIN = 0x08080200, LENGTH = 0x00004
PROGNAME (rx)   : ORIGIN = 0x08080204, LENGTH = 0x0000C
PROGVERSION (rx): ORIGIN = 0x08080210, LENGTH = 0x00004
PROGID (rx)     : ORIGIN = 0x08080214, LENGTH = 0x00010
COMPILEDATE (rx): ORIGIN = 0x08080224, LENGTH = 0x0000C
COMPILETIME (rx): ORIGIN = 0x08080230, LENGTH = 0x00010
/* PROGVALID (rx)  : ORIGIN = 0x0817F7C0, LENGTH = 0x00004
PROGNAME (rx)   : ORIGIN = 0x0817F7C4, LENGTH = 0x0000C
PROGVERSION (rx): ORIGIN = 0x0817F7D0, LENGTH = 0x00004
PROGID (rx)     : ORIGIN = 0x0817F7D4, LENGTH = 0x00010
COMPILEDATE (rx): ORIGIN = 0x0817F7E4, LENGTH = 0x0000C
COMPILETIME (rx): ORIGIN = 0x0817F7F0, LENGTH = 0x00010 */
RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 964K
TASK_RAM (xrw)   : ORIGIN = 0x200F1000, LENGTH = 60K
}

ENTRY(Reset_Handler)

PHDRS
{
  meta PT_LOAD FLAGS(6);
  text PT_LOAD FLAGS(5);
  ro   PT_LOAD FLAGS(4);
  ram  PT_LOAD FLAGS(6);
  task PT_LOAD FLAGS(6);
}

SECTIONS
{
  __stack_size = DEFINED(__stack_size) ? __stack_size : 8K;
  __heap_size  = DEFINED(__heap_size)  ? __heap_size  : 4K;

/* ISR vectors */
  .vectors :
  {
    . = ALIGN(4);
    KEEP(*(.vectors))
    . = ALIGN(4);
    __Vectors_End = .;
    __Vectors_Size = __Vectors_End - __gVectors;
  } >FLASH :text
  /* The program code and other data goes into FLASH */
  .flag :
  {
    . = ALIGN(4);
    KEEP(*(.flag))
    . = ALIGN(4);
  } >PROGVALID :meta

  /* The program code and other data goes into FLASH */
  .progname :
  {
    . = ALIGN(4);
    KEEP(*(.progname))
    . = ALIGN(4);
  } >PROGNAME :meta

  /* The program code and other data goes into FLASH */
  .progversion :
  {
    . = ALIGN(4);
    KEEP(*(.progversion))
    . = ALIGN(4);
  } >PROGVERSION :meta

  /* The program code and other data goes into FLASH */
  .progid :
  {
    . = ALIGN(4);
    KEEP(*(.progid))
    . = ALIGN(4);
  } >PROGID :meta

  /* The program code and other data goes into FLASH */
  .compiledate :
  {
    . = ALIGN(4);
    KEEP(*(.compiledate))
    . = ALIGN(4);
  } >COMPILEDATE :meta

  /* The program code and other data goes into FLASH */
  .compiletime :
  {
    . = ALIGN(4);
    KEEP(*(.compiletime))
    . = ALIGN(4);
  } >COMPILETIME :meta

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    /* the symbol ��_etext�� will be defined at the end of code section */
    _etext = .;
  } >FLASH :text

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH :ro

   .ARM.extab :
  {
     *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH :ro

    .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    } >FLASH :ro

  .ARM.attributes : { *(.ARM.attributes) } > FLASH

  .preinit_array (READONLY) :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH :ro

  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH :ro

  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH :ro

  /* provide some necessary symbols for startup file to initialize data */
  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    /* the symbol ��_sdata�� will be defined at the data section end start */
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    /* the symbol ��_edata�� will be defined at the data section end */
    _edata = .;
  } >RAM AT> FLASH :ram

  . = ALIGN(4);
  .bss :
  {
    /* the symbol ��_sbss�� will be defined at the bss section start */
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    /* the symbol ��_ebss�� will be defined at the bss section end */
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM :ram

 . = ALIGN(8);
  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );

  .task_heap (NOLOAD) :
  {
      *(.task_heap)
  } > TASK_RAM :task

  .stack ORIGIN(RAM) + LENGTH(RAM) - __stack_size :
  {
    . = ALIGN(8);
    PROVIDE( _heap_end = . );
    . = ORIGIN(RAM) + LENGTH(RAM) - __stack_size;
    PROVIDE( _sp = . );
    . = ALIGN(8);
  } >RAM AT>RAM :ram

  /* .fill :
  {
    FILL(0xdeadbeef);
    . = ORIGIN(FLASH) + LENGTH(FLASH) - 1;
    BYTE(0x5a)
    ___rom_at = .;
  } > FLASH */
  .fill :
  {
    FILL(0xFFFFFFFF);
    . = ORIGIN(COMPILETIME) + LENGTH(COMPILETIME) - 1;
    BYTE(0x5a)
    ___rom_at = .;
  } > COMPILETIME :meta
}

 /* input sections */
GROUP(libgcc.a libc.a libm.a libnosys.a)
