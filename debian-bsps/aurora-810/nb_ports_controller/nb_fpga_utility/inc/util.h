#ifndef _UTIL_H
#define _UTIL_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#define NB_SUCCESS 0
#define NB_FAIL 1

#define EXEC_TIME 1
#define PORT_NUM 32
#define CMD_LEN 50

#define fpga_ut_err(fmt, args...) syslog(LOG_ERR, "[%s:%d]" fmt, __func__, __LINE__, ##args)
#define fpga_ut_debug(fmt, args...) syslog(LOG_DEBUG, "[%s:%d]" fmt, __func__, __LINE__, ##args)
#define fpga_ut_info(fmt, args...) syslog(LOG_INFO, "[%s:%d]" fmt, __func__, __LINE__, ##args)

typedef union {
    uint32_t u32lpmode;
    uint8_t lpmode_chl[4];
} cache_lpmode_t;

typedef union {
    uint32_t u32reset;
    uint8_t reset_chl[4];
} cache_reset_t;

typedef union {
    uint32_t u32leddata;
    uint8_t leddata[4];
} cache_leddata_t;

typedef struct {
    cache_lpmode_t cache_lpmode;
    cache_reset_t cache_reset;
    cache_leddata_t cache_leddata[8];
} ports_cache_info_t;

typedef struct addr_t {
    uint16_t i2c_addr;
    uint16_t reg_addr;
} addr_t;

// IO expender register addr
#define IO_EXPANDER_INPUT_PORT0_ADDR_0 0x0
#define IO_EXPANDER_INPUT_PORT1_ADDR_0 0x1
#define IO_EXPANDER_OUTPUT_PORT0_ADDR_0 0x2
#define IO_EXPANDER_OUTPUT_PORT1_ADDR_0 0x3

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

// LED
#define LED_RGB_NUM 7
#define LED_CTL_I2C_ADDR 0x00000a00
#define LED_RGB_I2C_ADDR 0x00000a0c

int nb_lock_init();
int nb_lock_del();
int nb_lock();
int nb_unlock();

int nb_ports_ctrl_run_cmd(char *cmd, uint32_t *data);
int nb_ports_ctrl_output_parser(char *output, bool *result, uint32_t *data);
int nb_ports_ctrl_result_parser(char *result);
int nb_ports_ctrl_get_read_present_addr(uint8_t port, addr_t *addr);
int nb_ports_ctrl_get_read_lpmode_addr(uint8_t port, addr_t *addr);
int nb_ports_ctrl_get_write_lpmode_addr(uint8_t port, addr_t *addr);
int nb_ports_ctrl_get_read_reset_addr(uint8_t port, addr_t *addr);
int nb_ports_ctrl_get_write_reset_addr(uint8_t port, addr_t *addr);
#endif
