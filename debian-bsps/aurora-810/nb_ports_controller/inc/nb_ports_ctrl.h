#ifndef _NB_PORTS_CTRL_H
#define _NB_PORTS_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>

#define NB_SUCCESS 0
#define NB_FAIL 1

#define nb_err(fmt, args...) syslog(LOG_ERR, "[%s:%d]" fmt, __func__, __LINE__, ##args)
#define nb_debug(fmt, args...) syslog(LOG_DEBUG, "[%s:%d]" fmt, __func__, __LINE__, ##args)
#define nb_info(fmt, args...) syslog(LOG_INFO, "[%s:%d]" fmt, __func__, __LINE__, ##args)

int nb_ports_cache_init();
void nb_ports_controller_destroy(bool rpc);
int nb_ports_ctrl_init();
int nb_ports_ctrl_deinit();
int nb_ports_ctrl_presnece_get(uint32_t *u32presence);
int nb_ports_ctrl_port_presnece_get(uint8_t port, bool *bpresence);
int nb_ports_ctrl_lpmode_get(uint32_t *u32lpmode);
int nb_ports_ctrl_port_lpmode_get(uint8_t port, bool *blpmode);
int nb_ports_ctrl_port_lpmode_set(uint8_t port, bool bassert);
int nb_ports_ctrl_reset_get(uint32_t *u32reset);
int nb_ports_ctrl_port_reset_set(uint8_t port, bool bassert);
int nb_ports_ctrl_port_led_get(uint8_t port, uint32_t *leddata);
int nb_ports_ctrl_port_led_set(uint8_t port, uint8_t u8mode);
int nb_ports_ctrl_port_eeprom_get(uint8_t port, uint8_t u8offset, uint8_t u8length, uint8_t *u8data);
int nb_ports_ctrl_port_eeprom_set(uint8_t port, uint8_t u8offset, uint8_t u8length, uint8_t *u8data);
#ifdef __cplusplus
}
#endif
#endif /*end of NB_PORTS_CTRL_H*/
