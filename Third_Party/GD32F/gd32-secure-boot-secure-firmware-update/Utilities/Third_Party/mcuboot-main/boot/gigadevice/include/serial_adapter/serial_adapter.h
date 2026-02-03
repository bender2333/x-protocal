#ifndef __SERIAL_ADAPTER_H
#define __SERIAL_ADAPTER_H

/**
 * Serial write implementation used by MCUboot boot serial structure
 * in boot_serial.h
 */
void console_write(const char *str, int cnt);

/**
 * Serial read implementation used by MCUboot boot serial structure
 * in boot_serial.h
 */
int console_read(char *str, int str_cnt, int *newline);

/**
 * Initialize GPIOs used by console serial read/write
 */
void boot_console_init(void);

/**
 * Check if serial recovery detection pin is active
 */
//bool boot_serial_detect_pin(void);

uint16_t serial_htons(uint16_t x);
uint16_t serial_ntohs(uint16_t x);

void serial_delay_us(uint32_t cnt);
void serial_system_reset(void);
uint16_t crc16_ccitt(uint16_t crc_init, char *data, int len);

#endif /* __SERIAL_ADAPTER_H */
