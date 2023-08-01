#ifndef _LEDS_IO_H_
#define _LEDS_IO_H_

#include <linux/ioctl.h>
#include "base_nb.h"

#define LIO_ERR(fmt, args...)  NB_LOG_ERR("[LIO]", fmt, ##args)
#define LIO_INFO(fmt, args...) NB_LOG_INFO("[LIO]", fmt, ##args)
#define LIO_DBG(fmt, args...)  NB_LOG_DBG("[LIO]", fmt, ##args)


typedef struct {
    int port_num;
    uint32_t data;
} lio_args_port_led_t;


#define LIO_DRV_VERSION         "1.0.0"

#define LEDS_IOCTL_CLS_NAME     "lio"
#define LEDS_IOCTL_DEV_NAME     "leds_io"
#define LEDS_IOCTL_DEV_NUM      (1)

// ----- For LED Color Control -----
// NBA810 8-Bit: x R B G x R B G
#define LEDS_PORT_RGB_OFF       (0x0)
#define LEDS_PORT_RGB_GREEN     (0x11)
#define LEDS_PORT_RGB_BLUE      (0x22)
#define LEDS_PORT_RGB_TIFFANY   (0x33)
#define LEDS_PORT_RGB_RED       (0x44)
#define LEDS_PORT_RGB_YELLOW    (0x55)
#define LEDS_PORT_RGB_PURPLE    (0x66)
#define LEDS_PORT_RGB_WHITE     (0x77)

// ----- For IOCTL -----
#define LIO_MAGIC_NUM           '\x92'
#define LIO_IOCTL_PORT_GET      _IOW(LIO_MAGIC_NUM, 1, lio_args_port_led_t)
#define LIO_IOCTL_PORT_SET      _IOR(LIO_MAGIC_NUM, 2, lio_args_port_led_t)


#endif /* _LEDS_IO_H_ */
