#ifndef _PORTS_IO_H_
#define _PORTS_IO_H_

#include <linux/ioctl.h>
#include "base_nb.h"

#define PIO_ERR(fmt, args...)  NB_LOG_ERR("[PIO]", fmt, ##args)
#define PIO_INFO(fmt, args...) NB_LOG_INFO("[PIO]", fmt, ##args)
#define PIO_DBG(fmt, args...)  NB_LOG_DBG("[PIO]", fmt, ##args)


typedef struct {
    dev_t major;
    int sfp_min;
    int sfp_max;
    int qsfp_min;
    int qsfp_max;
    int port_total;
} pio_data_t ;


#define PIO_DRV_VERSION          "1.0.0"

#define PORTS_IOCTL_CLS_NAME     "pio"
#define PORTS_IOCTL_DEV_NAME     "ports_io"
#define PORTS_IOCTL_DEV_NUM      (1)
#define PORTS_IOCTL_REQ_GET_ALL  (-1)  // TBD: common include header files   

#define PORTS_XCVR_I2C_ADDR      (0x50)
#define PORTS_XCVR_REG_BANK_SEL  (126)
#define PORTS_XCVR_REG_PAG_SEL   (127)

// ----- For IOCTL -----
#define PIO_RW_I2C_LENGTH_MAX    (32)  // Follow i2c.h I2C_SMBUS_BLOCK_MAX = 32
#define PIO_RW_IOEXP_LENGTH_MAX  (2)
#define PIO_RW_IOEXP_REQ_GET_ALL (-1)
#define PIO_RW_XCVR_CTL_BYPASS   (0xFEFE)

typedef struct {
    int port_num;
    uint16_t addr;
    uint16_t reg;
    uint16_t data_len;
    uint16_t data[PIO_RW_I2C_LENGTH_MAX];
} pio_args_i2c_t;

typedef struct {
    int port_num;
    uint16_t bank_num;  // Can bypass setup bank_num by setup value: PIO_RW_XCVR_CTL_BYPASS
    uint16_t page_num;  // Can bypass setup page_num by setup value: PIO_RW_XCVR_CTL_BYPASS
    uint16_t reg;
    uint16_t data_len;
    uint16_t data[PIO_RW_I2C_LENGTH_MAX];
} pio_args_xcvr_t;

typedef struct {
    int port_num;
    uint64_t data;
} pio_args_ioexp_t;


#define PIO_MAGIC_NUM           '\x91'
#define PIO_IOCTL_I2C_READ      _IOR(PIO_MAGIC_NUM, 0, pio_args_i2c_t)
#define PIO_IOCTL_I2C_WRITE     _IOW(PIO_MAGIC_NUM, 1, pio_args_i2c_t)
#define PIO_IOCTL_XCVR_READ     _IOR(PIO_MAGIC_NUM, 2, pio_args_xcvr_t)
#define PIO_IOCTL_XCVR_WRITE    _IOW(PIO_MAGIC_NUM, 3, pio_args_xcvr_t)
#define PIO_IOCTL_PRESENT_READ  _IOR(PIO_MAGIC_NUM, 4, pio_args_ioexp_t)
#define PIO_IOCTL_LPMOD_READ    _IOR(PIO_MAGIC_NUM, 5, pio_args_ioexp_t)
#define PIO_IOCTL_LPMOD_WRITE   _IOW(PIO_MAGIC_NUM, 6, pio_args_ioexp_t)
#define PIO_IOCTL_REST_READ     _IOR(PIO_MAGIC_NUM, 7, pio_args_ioexp_t)
#define PIO_IOCTL_REST_WRITE    _IOW(PIO_MAGIC_NUM, 8, pio_args_ioexp_t)
#define PIO_IOCTL_RXLOS_READ    _IOR(PIO_MAGIC_NUM, 9, pio_args_ioexp_t)


#endif /* _PORTS_IO_H_ */
