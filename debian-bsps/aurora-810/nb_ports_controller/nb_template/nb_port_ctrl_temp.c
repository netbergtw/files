#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nb_ports_ctrl.h"

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_init                                         */
/* Description  : Initialize port ctrl IO interface                          */
/* Input        : None                                                       */
/* Output       : None                                                       */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_init() {
  printf("[NB_PORTS_CTRL]TBD on %s:%d\n", __func__, __LINE__);
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_presnece_get                                 */
/* Description  : Get all ports presence status                              */
/* Input        : None                                                       */
/* Output       : u32presence                                                */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_presnece_get(uint32_t *u32presence) {
  printf("[NB_PORTS_CTRL]TBD on %s:%d\n", __func__, __LINE__);
  *u32presence = 0;
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_presnece_get                            */
/* Description  : Get specific ports presence status                         */
/* Input        : port                                                       */
/* Output       : bpresence                                                  */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_presnece_get(uint8_t port, bool *bpresence) {
  printf("[NB_PORTS_CTRL]TBD on %s:%d\n", __func__, __LINE__);
  *bpresence = false;
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_lpmode_get                                   */
/* Description  : Get all ports lpmode status                                */
/* Input        : None                                                       */
/* Output       : u32lpmode                                                  */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_lpmode_get(uint32_t *u32lpmode) {
  printf("[NB_PORTS_CTRL]TBD on %s:%d\n", __func__, __LINE__);
  *u32lpmode = 0;
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_lpmode_set                              */
/* Description  : set specific port lpmode                                   */
/* Input        : u8port, bassert                                            */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_lpmode_set(uint8_t port, bool bassert) {
  printf("[NB_PORTS_CTRL]TBD on %s :%d\n", __func__, __LINE__);
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_reset_set                               */
/* Description  : assert or deassert specific port reset                     */
/* Input        : u8port, bassert                                            */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_reset_set(uint8_t port, bool bassert) {
  printf("[NB_PORTS_CTRL]TBD on %s :%d\n", __func__, __LINE__);
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_led_set                                 */
/* Description  : set specific port led mode                                 */
/* Input        : u8port, u8mode                                             */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_led_set(uint8_t port, uint8_t u8mode) {
  printf("[NB_PORTS_CTRL]TBD on %s :%d\n", __func__, __LINE__);
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_eeprom_read                             */
/* Description  : read port's eeprom information                             */
/* Input        : u8port, u8offset                                           */
/* Output       : u8length, u8data                                           */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_eeprom_get(uint8_t port, uint8_t u8offset,
                                  uint8_t *u8length, uint8_t *u8data) {
  printf("[NB_PORTS_CTRL]TBD on %s :%d\n", __func__, __LINE__);
  return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_eeprom_set                             */
/* Description  : write port's eeprom information                            */
/* Input        : u8port, u8offset, u8length, u8data                         */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_eeprom_set(uint8_t port, uint8_t u8offset,
                                  uint8_t u8length, uint8_t *u8data) {
  printf("[NB_PORTS_CTRL]TBD on %s :%d\n", __func__, __LINE__);
  return NB_SUCCESS;
}
