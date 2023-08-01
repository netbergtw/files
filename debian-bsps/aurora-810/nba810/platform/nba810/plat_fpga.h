/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _PLAT_FPGA_H_
#define _PLAT_FPGA_H_

#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/ioctl.h>
#include <linux/mman.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/unistd.h>

#include "../plat.h"


#define NB_SUCCESS 0

#define NB_FPGA_ACCESS_TIME_COUNT 200000   // 20*100ms = 2s
#define NB_FPGA_FW_DOWNLOAD_TIME_COUNT 60  // 60*100ms = 6s

#define NB_FPGA_ACCESS_DELAY udelay(1)
#define NB_FPGA_MEM_SIZE 0x40000
#define NB_FPGA_CMD_DELAY udelay(1)
#define NB_FPGA_GBFW_SIZE 131072
// NBA810
#define PORT_NUM 32
#define NB_FPGA_STATUS_DONE 0x0
#define NB_FPGA_STATUS_BUSY 0x1
#define NB_FPGA_STATUS_TIMEOUT 0x2

#define QSFP1_I2C_CHL 0x1  // qsfp 1 ~ 8
#define QSFP2_I2C_CHL 0x2  // qsfp 9 ~ 16
#define QSFP3_I2C_CHL 0x3  // qsfp 17 ~ 24
#define QSFP4_I2C_CHL 0x4  // qsfp 25 ~ 32

#define NB_FPGA_QSFP1_MUX_ADDR 0x70
#define NB_FPGA_QSFP2_MUX_ADDR 0x71
#define NB_FPGA_QSFP3_MUX_ADDR 0x72
#define NB_FPGA_QSFP4_MUX_ADDR 0x73

#define NB_FPGA_PCI_VENDOR_ID_XILINX 0x10ee
#define NB_FPGA_PCI_DEVICE_ID_XILINX 0x7012

#define NB_MEDIA_I2C_BASE 0x00000200
#define NB_MEDIA_I2C_FORMAT_SIZE 0x00000100

// FPGA control
#define NB_FPGA_MEDIA_CLEAR 0xff
#define NB_FPGA_LED_CLEAR 0xff
#define NB_FPGA_LED_READ  0x1
#define NB_FPGA_LED_WRITE 0x2
#define NB_FPGA_LED_MEM_ADDR_START  0x00000a00
#define NB_FPGA_LED_MEM_ADDR_RGB    0x00000a0c

#define NB_FPGA_LED_MEM_CTL_IDLE    0x0
#define NB_FPGA_LED_MEM_CTL_ENABLE  0x1
#define NB_FPGA_LED_MEM_CTL_CLEAR   0x2


// console font color
#define HEADER "\033[95m"
#define OKBLUE "\033[94m"
#define OKGREEN "\033[92m"
#define WARNING "\033[93m"
#define FAIL "\033[91m"
#define ENDC "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"

typedef struct {        /* total size :256 */
    uint8_t control;    /* addr:	0 */
    uint8_t slice;      /* addr:	1 */
    uint8_t channel;    /* addr:	2 */
    uint8_t rsd_1;      /* addr:	3 */
    uint16_t reg_addr;  /* addr:	4~5 */
    uint16_t rsd_2;     /* addr:	6~7 */
    uint16_t status;    /* addr:	8~9 */
    uint16_t rsd_3;     /* addr:	10~11 */
    uint16_t w_data;    /* addr:	12~13 */
    uint16_t rsd_4;     /* addr:	14~15 */
    uint16_t r_data;    /* addr:	16~17 */
    uint16_t rsd_5;     /* addr:	18~19 */
    uint8_t rsd_6[236]; /* addr:	20~255 */
} nb_fpga_mdio_format_t;

typedef struct {        /* total size :256 */
    uint16_t control;   /* addr:	0~1 */
    uint16_t i2c_addr;  /* addr:	2~3 */
    uint16_t reg_addr;  /* addr:	4~5 */
    uint16_t rsd_1;     /* addr:	6~7 */
    uint16_t status;    /* addr:	8~9 */
    uint16_t rsd_2;     /* addr:	10~11 */
    uint16_t w_data;    /* addr:	12~13 */
    uint16_t rsd_3;     /* addr:	14~15 */
    uint16_t r_data;    /* addr:	16~17 */
    uint16_t rsd_4;     /* addr:	18~19 */
    uint8_t rsd_5[236]; /* addr:	20~255 */
} nb_fpga_led_i2c_format_t;

typedef struct {        /* total size :256 */
    uint16_t control;   /* addr:	0~1 */
    uint16_t i2c_addr;  /* addr:	2~3 */
    uint16_t reg_addr;  /* addr:	4~5 */
    uint16_t rsd_1;     /* addr:	6~7 */
    uint16_t status;    /* addr:	8~9 */
    uint16_t rsd_2;     /* addr:	10~11 */
    uint16_t w_data;    /* addr:	12~13 */
    uint16_t rsd_3;     /* addr:	14~15 */
    uint16_t r_data;    /* addr:	16~17 */
    uint16_t rsd_4;     /* addr:	18~19 */
    uint8_t rsd_5[236]; /* addr:	20~255 */
} nb_fpga_media_i2c_format_t;

typedef struct {        /* total size :256 */
    uint16_t control;   /* addr:	0~1 */
    uint16_t i2c_addr;  /* addr:	2~3 */
    uint16_t reg_addr;  /* addr:	4~5 */
    uint16_t rsd_1;     /* addr:	6~7 */
    uint16_t status;    /* addr:	8~9 */
    uint16_t rsd_2;     /* addr:	10~11 */
    uint16_t w_data;    /* addr:	12~13 */
    uint16_t rsd_3;     /* addr:	14~15 */
    uint16_t r_data;    /* addr:	16~17 */
    uint16_t rsd_4;     /* addr:	18~19 */
    uint8_t rsd_5[236]; /* addr:	20~255 */
} nb_fpga_feature_card_i2c_format_t;

typedef struct {
    uint8_t info[256];
} nb_fpga_info;

typedef struct {
    uint8_t rsd[256];
} nb_fpga_reserve_256;

//========================NETBERG_FPGA Added 20220401======================================================
typedef struct {        /* total size :256 */
    uint16_t control;   /* addr:	0~1 */
    uint16_t i2c_addr;  /* addr:	2~3 */
    uint16_t reg_addr;  /* addr:	4~5 */
    uint16_t rsd_1;     /* addr:	6~7 */
    uint16_t status;    /* addr:	8~9 */
    uint16_t rsd_2;     /* addr:	10~11 */
    uint16_t w_data;    /* addr:	12~13 */
    uint16_t rsd_3;     /* addr:	14~15 */
    uint16_t r_data;    /* addr:	16~17 */
    uint16_t rsd_4;     /* addr:	18~19 */
    uint8_t rsd_5[236]; /* addr:	20~255 */
} nb_fpga_i2c_format_t;

typedef struct {        /* total size :256 */
    uint16_t control;   /* addr:	0~1 */
    uint16_t rsd_1[3];  /* addr:	2~7 */
    uint16_t status;    /* addr:	8~9 */
    uint16_t rsd_2;     /* addr:	10~11 */
    uint8_t LED[32];    /* addr:	12~43	*/
    uint8_t rsd_3[212]; /* addr:	44~255 */
} nb_fpga_led_format_t;

//======================================================================================================

typedef struct nb_fpga_data {
    nb_fpga_info fpga_info;                               /* addr:	0x00~0xff */
    nb_fpga_mdio_format_t mdio_data;                      /* addr:	0x100~0x1ff */
                                                          //=====================for netberg_fpga==================
    nb_fpga_i2c_format_t netberg_fpga_i2c_data[8];             /* addr:	0x200~0x9ff */
    nb_fpga_led_format_t netberg_fpga_led_data;                /* addr:	0xa00~0xaff */
                                                          //==================================================
    nb_fpga_reserve_256 rsd_zone_1[5];                    /* addr:	0xb00~0xfff */
    nb_fpga_feature_card_i2c_format_t feature_card_i2c_0; /* addr:	0x1000~0x10ff */
    nb_fpga_led_i2c_format_t led_i2c_data[8];             /* addr:	0x1100~0x18ff */
    nb_fpga_reserve_256 rsd_zone_2[7];                    /* addr:	0x1900~0x1fff */
    nb_fpga_feature_card_i2c_format_t feature_card_i2c_1; /* addr:	0x2000~0x20ff */
    nb_fpga_media_i2c_format_t media_i2c_data[8];         /* addr:	0x2100~0x28ff */
    nb_fpga_reserve_256 rsd_zone_3[7];                    /* addr:	0x2900~0x2fff */
    nb_fpga_reserve_256 unuse[464];                       /* addr:	0x3000~0x1ffff */
    uint8_t gearbox_fw[NB_FPGA_GBFW_SIZE];                /* addr:	0x20000~0x3ffff */
} nb_fpga_regs_t;


int nb_fpga_mem32_read(uint32_t offset, uint32_t *data);
int nb_fpga_mem32_write(uint32_t offset, uint32_t data);
int nb_fpga_i2c_check(int channel);
int nb_fpga_i2c_read(int channel, uint16_t i2c_addr, uint16_t reg_addr, uint16_t *data);
int nb_fpga_i2c_write(int channel, uint16_t i2c_addr, uint16_t reg_addr, uint16_t data);
int nb_fpga_qsfp_read(int QSFP_channel, uint16_t Reg_addr, uint8_t *R_data);
int nb_fpga_qsfp_write(int QSFP_channel, uint16_t Reg_addr, uint16_t w_data);

int nb_fpga_led_check(void);
// int netberg_fpga_led(int led_port, uint8_t data);

void nb_fpga_lock_init(nb_platdrv_t *plt_p);
void nb_fpga_lock_destroy(nb_platdrv_t *plt_p);
bool nb_fpga_lock(plat_mutex_t *mtx_p);
void nb_fpga_unlock(plat_mutex_t *mtx_p);

#endif /* _PLAT_FPGA_H_ */
