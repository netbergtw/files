/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _PLAT_H_
#define _PLAT_H_

#include <linux/errno.h>
#include <linux/mutex.h>
#include "../includes/base_nb.h"

#define PLAT_ERR(fmt, args...)  NB_LOG_ERR("[PLAT]", fmt, ##args)
#define PLAT_INFO(fmt, args...) NB_LOG_INFO("[PLAT]", fmt, ##args)
#define PLAT_DBG(fmt, args...)  NB_LOG_DBG("[PLAT]", fmt, ##args)

#define PLAT_DRV_VERSION             "1.0.0"

#define PLAT_PORTS_TOTAL_MAX         (128)
#define PLAT_IOEXP_DATA_WIDTH_MAX    (8)
#define PLAT_PORT_LED_PERPORT_NUM    (2)
#define PLAT_CLIENT_REQ_ALL          (-1)


typedef struct plat_mutex {
    int id;
    char name[16];
    struct mutex dev_mutex;

    bool (*lock_dev) (struct plat_mutex *mtx_p);
    void (*unlock_dev) (struct plat_mutex *mtx_p);
    
} plat_mutex_t;


typedef struct {
    int num;
    int type;

    uint8_t *cache;
    uint32_t cache_led;

    uint32_t led_ctl_addr;
    uint32_t led_rgb_addr;
    uint8_t led_rgb_offs;
    uint8_t led_grn_bit[PLAT_PORT_LED_PERPORT_NUM];
    uint8_t led_blu_bit[PLAT_PORT_LED_PERPORT_NUM];
    uint8_t led_red_bit[PLAT_PORT_LED_PERPORT_NUM];
    
    plat_mutex_t *mutex_p;
} port_t;


typedef struct {
    int data_width;
    uint64_t cache;

    uint16_t in_bus[PLAT_IOEXP_DATA_WIDTH_MAX];
    uint16_t in_addr[PLAT_IOEXP_DATA_WIDTH_MAX];
    uint16_t in_reg[PLAT_IOEXP_DATA_WIDTH_MAX];

    uint16_t out_bus[PLAT_IOEXP_DATA_WIDTH_MAX];
    uint16_t out_addr[PLAT_IOEXP_DATA_WIDTH_MAX];
    uint16_t out_reg[PLAT_IOEXP_DATA_WIDTH_MAX];

    plat_mutex_t *mutex_p;
} ioexp_t;


typedef struct nb_platdrv {
    bool ready;
    int log_lv;
    int port_total;
    char plat_name[16];

    port_t *port_list[PLAT_PORTS_TOTAL_MAX];
    uint64_t port_present;

    ioexp_t *ioexp_present_p;
    ioexp_t *ioexp_lpmod_p;
    ioexp_t *ioexp_reset_p;
    ioexp_t *ioexp_rxlos_p;

    struct pci_dev *pcidev;
    plat_mutex_t *fpga_mutex;
    
    int (*plat_port_present_get) (struct nb_platdrv *plat_p, uint64_t *buf, int port_num);
    int (*plat_port_lpmod_get) (struct nb_platdrv *plat_p, uint64_t *buf, int port_num);
    int (*plat_port_lpmod_set) (struct nb_platdrv *plat_p, uint64_t *buf, int port_num);
    int (*plat_port_reset_get) (struct nb_platdrv *plat_p, uint64_t *buf, int port_num);
    int (*plat_port_reset_set) (struct nb_platdrv *plat_p, uint64_t *buf, int port_num);
    int (*plat_port_rxlos_get) (struct nb_platdrv *plat_p, uint64_t *buf, int port_num);
    int (*plat_port_i2c_get) (struct nb_platdrv *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg);
    int (*plat_port_i2c_set) (struct nb_platdrv *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg);
    int (*plat_port_xcvr_get) (struct nb_platdrv *plat_p, uint16_t *buf, int port_num, uint16_t reg);
    int (*plat_port_xcvr_set) (struct nb_platdrv *plat_p, uint16_t *buf, int port_num, uint16_t reg);
    int (*plat_led_port_get) (struct nb_platdrv *plat_p, uint32_t *buf, int port_num);
    int (*plat_led_port_set) (struct nb_platdrv *plat_p, uint32_t *buf, int port_num);
    int (*plat_init) (struct nb_platdrv *plt_p);
    void (*plat_free) (struct nb_platdrv *plt_p);
    
} nb_platdrv_t;


int get_platform_drv(nb_platdrv_t *drv);

extern int loglv;
extern int hotswap;

#endif /* _PLAT_H_ */


