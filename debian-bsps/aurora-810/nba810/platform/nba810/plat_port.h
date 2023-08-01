/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _PLAT_PORT_H_
#define _PLAT_PORT_H_

#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include "plat_fpga.h"
#include "../plat.h"

#define PORT_TYPE_SFP              (10001)
#define PORT_TYPE_QSFP             (10002)

#define IOEXP_TYPE_PRESENT         (20001)
#define IOEXP_TYPE_LPMOD           (20002)
#define IOEXP_TYPE_RESET           (20003)
#define IOEXP_TYPE_RXLOS           (20004)
#define IOEXP_TYPE_INTL            (20004)

#define PLAT_PORT_TASK_ENABLE      (1)
#define PLAT_PORT_TASK_PERIOD_MS   (300)
#define PLAT_PORT_FSM_RETRY_COUNT  (3)

#define FSM_PORT_STATE_DISABLE     (100)
#define FSM_PORT_STATE_RUNNING     (101)
#define FSM_PORT_STATE_CHECKING    (102)

typedef struct {
    int state;
    int state_retry;   
    uint64_t port_present;
    nb_platdrv_t *plat_p;
} port_fsm_t;

port_t* get_port_obj(nb_platdrv_t *plat_p, int port_num);
int plat_port_first_num(void);
int plat_port_type_get(int port_num);
int plat_port_present_get_all(nb_platdrv_t *plat_p, uint64_t *data);
int plat_port_lpmod_get_all(nb_platdrv_t *plat_p, uint64_t *data);
int plat_port_lpmod_set_all(nb_platdrv_t *plat_p, uint64_t *data);
int plat_port_reset_get_all(nb_platdrv_t *plat_p, uint64_t *data);
int plat_port_reset_set_all(nb_platdrv_t *plat_p, uint64_t *data);
int plat_port_rxlos_get_all(nb_platdrv_t *plat_p, uint64_t *data);
int plat_port_i2c_read(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg);
int plat_port_i2c_write(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg);
int plat_port_xcvr_read(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t reg);
int plat_port_xcvr_write(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t reg);
int plat_port_led_rgb_get(nb_platdrv_t *plat_p, uint32_t *buf, int port_num);
int plat_port_led_rgb_set(nb_platdrv_t *plat_p, uint32_t *buf, int port_num);

int plat_port_init(nb_platdrv_t *plt_p);
void plat_port_free(nb_platdrv_t *plt_p);

extern nb_fpga_regs_t *nb_fpga_mmap_addr;


#endif /* _PLAT_PORT_H_ */
