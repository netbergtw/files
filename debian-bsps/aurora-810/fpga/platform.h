#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/types.h>

#include "inc/nb_fpga.h"
#include "utils.h"

// NETBERG_FPGA
#define NETBERG_FPGA_PCI_VENDOR_ID_XILINX 0x10ee
#define NETBERG_FPGA_PCI_DEVICE_ID_XILINX 0x7012
#define MODULE_EEPROM_SIZE 256
extern u_int32_t port_i2c_chl_mask[];
// FPGA
#define SN_FPGA_VERSION_MEM_OFFSET 0x0
// IO expender register addr
#define IO_EXPANDER_INPUT_PORT0_ADDR_0 0x0
#define IO_EXPANDER_INPUT_PORT1_ADDR_0 0x1
#define IO_EXPANDER_OUTPUT_PORT0_ADDR_0 0x2
#define IO_EXPANDER_OUTPUT_PORT1_ADDR_0 0x3
#define IO_EXPANDER_PO_INVERSION_PORT0_ADDR_0 0x4
#define IO_EXPANDER_PO_INVERSION_PORT1_ADDR_0 0x5
#define IO_EXPANDER_CFG_PORT0_ADDR_0 0x6
#define IO_EXPANDER_CFG_PORT1_ADDR_0 0x7

#define IO_OUPUT 0x00
#define IO_INPUT 0xFF

// LPmode
#define LP_MODE_I2C_CHL 0x05
#define LP_MODE_I2C_ADDR_NUM 2
#define LP_MODE_IO_NUM 4
#define LP_MODE_I2C_ADDR_1 0x20
#define LP_MODE_I2C_ADDR_2 0x21
// Reset
#define RESET_I2C_CHL 0x06
#define RESET_I2C_ADDR_NUM 2
#define RESET_IO_NUM 2
#define RESET_I2C_ADDR_1 0x26
#define RESET_I2C_ADDR_2 0x27
// QSFP  Present
#define PRESENT_I2C_CHL 0x07  // I2C addr
#define PRESENT_I2C_ADDR_NUM 2
#define PRESENT_IO_NUM 4
#define PRESENT_I2C_ADDR_1 0x22  // QSFP-DD Present 0-15
#define PRESENT_I2C_ADDR_2 0x23  // QSFP-DD Present 16-31

// Rx Loss
#define RX_LOSS_I2C_CHL 0x08
#define RX_LOSS_I2C_ADDR_NUM 2
#define RX_LOSS_IO_NUM 2
#define RX_LOSS_I2C_ADDR_1 0x24
#define RX_LOSS_I2C_ADDR_2 0x25

// LED
#define LED_RGB_NUM 7
#define LED_CTL_I2C_ADDR 0x00000a00
#define LED_RGB_I2C_ADDR 0x00000a0c

// Temperature
#define QSFP_TEMP_MSB_EEPROM_ADDR 0x16
#define QSFP_TEMP_LSB_EEPROM_ADDR 0x17

// port
typedef struct port_t {
    int num;
} port_t;

// addr for media bytes

typedef struct addr_t {
    uint16_t i2c_addr;
    uint16_t reg_addr;
} addr_t;

extern sn_fpga_regs_t *sn_fpga_mmap_addr;

// plafrom
int platform_probe(uint32_t pci_addr);
int platform_init(void);
int platform_exit(void);

// i2c util
int sn_i2c_io_init(void);

// FPGA
int sn_fpga_version_read(uint32_t *version);

// Present
int sn_port_read_present(port_t port, bool *presnet);
int sn_port_read_all_present(uint32_t *all_present);
int get_port_present_addr(int port, addr_t *addr);

// LED
int sn_port_read_led(port_t port, uint8_t *data);
int sn_port_write_led(port_t port, uint32_t data);
int get_port_led_value(uint32_t port_group_addr, uint32_t *data);
int write_port_led_value(uint32_t port_group_addr, uint32_t data);
uint32_t get_led_addr(int port_num);

// EEPROM
int sn_port_read_eeprom(port_t port, uint8_t *buff, uint16_t offset, uint16_t len);
int sn_port_write_eeprom(port_t port, uint8_t *buff, uint16_t offset, uint16_t len);

// LPMode
int sn_port_read_lpmode(port_t port, bool *lpmode);
int sn_port_write_lpmode(port_t port, bool lpmode);
int read_lpmode(int port_num, uint16_t *data);
int write_lpmode(int port_num, uint16_t data);

// Reset
int sn_port_write_reset(port_t port, bool reset);
int read_reset(int port_num, uint16_t *data);
int write_reset(int port_num, uint16_t data);

// Temperature
int sn_port_temp_read(port_t port, int8_t *temp);

#endif
