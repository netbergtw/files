/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _PLAT_CONF_H_
#define _PLAT_CONF_H_

#define PLAT_HW_NAME   "NBA810"

// Port settings
//
// - SFP  : SFP+/SFP28
// - QSFP : QSFP+/QSFP28/QSFP56/QSFP28-DD/QSFP56-DD
#define PLAT_PORT_SFP_MIN    (0)
#define PLAT_PORT_SFP_MAX    (0)
#define PLAT_PORT_QSFP_MIN   (1)
#define PLAT_PORT_QSFP_MAX   (32)

#define PLAT_PORT_NUM_START  (1)
#define PLAT_PORT_NUM_TOTAL  (32)


// IO Expander settings
//
// => Default settings for PCA9555
// => PCA9555 layout:
//    - Bytes 0~1 : Input
//    - Bytes 2~3 : Output
//    - Bytes 4~5 : Polarity Inversion 
//    - Bytes 6~7 : Configuration (directions)
#define PLAT_IOEXP_CONF_NUM  (40)
//                                                |--------------------------set LPMode-----------------------|---------------------------set Reset-----------------------|----------------set Present------------|-------------set Rx loss---------------|
uint16_t ioexp_config_bus[PLAT_IOEXP_CONF_NUM]  = {0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08};
uint16_t ioexp_config_addr[PLAT_IOEXP_CONF_NUM] = {0x20,0x20,0x21,0x21,0x20,0x20,0x21,0x21,0x20,0x20,0x21,0x21,0x26,0x26,0x27,0x27,0x26,0x26,0x27,0x27,0x26,0x26,0x27,0x27,0x22,0x22,0x23,0x23,0x22,0x22,0x23,0x23,0x24,0x24,0x25,0x25,0x24,0x24,0x25,0x25};
uint16_t ioexp_config_reg[PLAT_IOEXP_CONF_NUM]  = {0x06,0x07,0x06,0x07,0x02,0x03,0x02,0x03,0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,0x02,0x03,0x02,0x03,0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,0x04,0x05,0x04,0x05};
uint16_t ioexp_config_data[PLAT_IOEXP_CONF_NUM] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};


// IO Expander IO data width (Bytes)
#define PLAT_IOEXP_DATA_WIDTH (4)

// IO Expander: Present mapping (RO)
//                                                      |1  8 |9  16|17 24|25 32|
uint16_t ioexp_in_present_bus[PLAT_IOEXP_DATA_WIDTH]  = {0x07, 0x07, 0x07, 0x07};
uint16_t ioexp_in_present_addr[PLAT_IOEXP_DATA_WIDTH] = {0x22, 0x22, 0x23, 0x23};
uint16_t ioexp_in_present_reg[PLAT_IOEXP_DATA_WIDTH]  = {0x00, 0x01, 0x00, 0x01};


// IO Expander: LPMod mapping (RW)
uint16_t ioexp_in_lpmod_bus[PLAT_IOEXP_DATA_WIDTH]  = {0x05, 0x05, 0x05, 0x05};
uint16_t ioexp_in_lpmod_addr[PLAT_IOEXP_DATA_WIDTH] = {0x20, 0x20, 0x21, 0x21};
uint16_t ioexp_in_lpmod_reg[PLAT_IOEXP_DATA_WIDTH]  = {0x00, 0x01, 0x00, 0x01};

uint16_t ioexp_out_lpmod_bus[PLAT_IOEXP_DATA_WIDTH]  = {0x05, 0x05, 0x05, 0x05};
uint16_t ioexp_out_lpmod_addr[PLAT_IOEXP_DATA_WIDTH] = {0x20, 0x20, 0x21, 0x21};
uint16_t ioexp_out_lpmod_reg[PLAT_IOEXP_DATA_WIDTH]  = {0x02, 0x03, 0x02, 0x03};


// IO Expander: RESET mapping (RW)
uint16_t ioexp_in_reset_bus[PLAT_IOEXP_DATA_WIDTH]  = {0x06, 0x06, 0x06, 0x06};
uint16_t ioexp_in_reset_addr[PLAT_IOEXP_DATA_WIDTH] = {0x26, 0x26, 0x27, 0x27};
uint16_t ioexp_in_reset_reg[PLAT_IOEXP_DATA_WIDTH]  = {0x00, 0x01, 0x00, 0x01};

uint16_t ioexp_out_reset_bus[PLAT_IOEXP_DATA_WIDTH]  = {0x06, 0x06, 0x06, 0x06};
uint16_t ioexp_out_reset_addr[PLAT_IOEXP_DATA_WIDTH] = {0x26, 0x26, 0x27, 0x27};
uint16_t ioexp_out_reset_reg[PLAT_IOEXP_DATA_WIDTH]  = {0x02, 0x03, 0x02, 0x03};


// IO Expander: RX Los mapping (RO)
uint16_t ioexp_in_rxlos_bus[PLAT_IOEXP_DATA_WIDTH]  = {0x08, 0x08, 0x08, 0x08};
uint16_t ioexp_in_rxlos_addr[PLAT_IOEXP_DATA_WIDTH] = {0x24, 0x24, 0x25, 0x25};
uint16_t ioexp_in_rxlos_reg[PLAT_IOEXP_DATA_WIDTH]  = {0x00, 0x01, 0x00, 0x01};


// FPGA: LED RGB reg mapping
#define PLAT_FPGA_LED_CTL_BASE_ADDR        (0x00000a00)

//                                     Port Number: |   1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27    28    29    30    31    32|
uint8_t led_rgb_offset_addr[PLAT_PORT_NUM_TOTAL] = {0x0c, 0x0c, 0x0c, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x14, 0x14, 0x14, 0x14, 0x18, 0x18, 0x18, 0x18, 0x1c, 0x1c, 0x1c, 0x1c, 0x20, 0x20, 0x20, 0x20, 0x24, 0x24, 0x24, 0x24, 0x28, 0x28, 0x28, 0x28};
uint8_t led_rgb_offset_bit[PLAT_PORT_NUM_TOTAL]  = {0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18, 0x00, 0x08, 0x10, 0x18};

uint8_t  led_rgb_offset_bit_green[PLAT_PORT_LED_PERPORT_NUM] = {0, 3};
uint8_t  led_rgb_offset_bit_blue[PLAT_PORT_LED_PERPORT_NUM]  = {1, 4};
uint8_t  led_rgb_offset_bit_red[PLAT_PORT_LED_PERPORT_NUM]   = {2, 5};

#endif /* _PLAT_CONF_H_ */
