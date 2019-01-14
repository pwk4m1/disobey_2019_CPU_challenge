
/*
 * This file defines different interrupts for
 * handler to use
 */

#if !defined(__interrupts_h__)
#define __interrupts_h__

/* these values are stored in AL register */
#define __int_screen_write	0x01
#define __int_screen_clear	0x02

#define __int_reset_cpu		0x03
#define __int_poweroff		0x04

#define __int_get_uuid		0x05
#define __int_get_serial	0x06
#define __int_delay		0x07

#define __int_rx		0x08
#define __int_tx		0x09

#define __int_spi_read		0x0A
#define __int_spi_write		0x0B

#define __int_i2c_read		0x0C
#define __int_i2c_write		0x0D

#endif

