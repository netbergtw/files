#ifndef NETBERG_AURORA_721_CPLD_H
#define NETBERG_AURORA_721_CPLD_H

/* CPLD device index value */
enum cpld_id {
    cpld1,
    cpld2
};

/* 
 *  Normally, the CPLD register range is 0x00-0xff.
 *  Therefore, we define the invalid address 0x100 as CPLD_NONE_REG
 */
#define CPLD_NONE_REG                     0x100
#define CPLD_ID_REG                       0x03


/* CPLD 1 registers */
#define CPLD_BOARD_ID_0_REG               0x00
#define CPLD_BOARD_ID_1_REG               0x01
#define CPLD_SKU_EXT_REG                  0x06
#define CPLD_MAC_INTR_REG                 0x10
#define CPLD_HWM_INTR_REG                 0x13
#define CPLD_CPLD2_INTR_REG               0x14
#define CPLD_PTP_INTR_REG                 0x1B
#define CPLD_SYSTEM_INTR_REG              0x1C
#define CPLD_MAC_MASK_REG                 0x20
#define CPLD_HWM_MASK_REG                 0x23
#define CPLD_CPLD2_MASK_REG               0x24
#define CPLD_PTP_MASK_REG                 0x2B
#define CPLD_SYSTEM_MASK_REG              0x2C
#define CPLD_MAC_EVT_REG                  0x30
#define CPLD_HWM_EVT_REG                  0x33
#define CPLD_CPLD2_EVT_REG                0x34
#define CPLD_MAC_RESET_REG                0x40
#define CPLD_SYSTEM_RESET_REG             0x41
#define CPLD_BMC_NTM_RESET_REG            0x43
#define CPLD_USB_RESET_REG                0x44
#define CPLD_I2C_MUX_RESET_REG            0x46
#define CPLD_MISC_RESET_REG               0x48
#define CPLD_BRD_PRESENT_REG              0x50
#define CPLD_PSU_STATUS_REG               0x51
#define CPLD_SYSTEM_PWR_REG               0x52
#define CPLD_MAC_SYNCE_REG                0x53
#define CPLD_MAC_AVS_REG                  0x54
#define CPLD_SYSTEM_STATUS_REG            0x55
#define CPLD_WATCHDOG_REG                 0x5A
#define CPLD_BOOT_SELECT_REG              0x5B
#define CPLD_MUX_CTRL_REG                 0x5C
#define CPLD_MISC_CTRL_1_REG              0x5D
#define CPLD_MISC_CTRL_2_REG              0x5E
#define CPLD_MAC_TEMP_REG                 0x61
#define CPLD_SYSTEM_LED_PSU_REG           0x80
#define CPLD_SYSTEM_LED_SYS_REG           0x81
#define CPLD_SYSTEM_LED_FAN_REG           0x83
#define CPLD_SYSTEM_LED_ID_REG            0x84
#define DBG_CPLD_MAC_INTR_REG             0xE0
#define DBG_CPLD_HWM_INTR_REG             0xE3
#define DBG_CPLD_CPLD2_INTR_REG           0xE4
#define DBG_CPLD_PTP_INTR_REG             0xEB

/* CPLD 2*/
#define CPLD_QSFP_ABS_0_7_REG             0x10
#define CPLD_QSFP_ABS_8_15_REG            0x11
#define CPLD_QSFP_ABS_16_23_REG           0x12
#define CPLD_QSFP_ABS_24_31_REG           0x13
#define CPLD_QSFP_INTR_0_7_REG            0x14
#define CPLD_QSFP_INTR_8_15_REG           0x15
#define CPLD_QSFP_INTR_16_23_REG          0x16
#define CPLD_QSFP_INTR_24_31_REG          0x17
#define CPLD_SFP_ABS_0_1_REG              0x18
#define CPLD_SFP_RXLOS_0_1_REG            0x19
#define CPLD_SFP_TXFLT_0_1_REG            0x1a
#define CPLD_QSFP_MASK_ABS_0_7_REG        0x20
#define CPLD_QSFP_MASK_ABS_8_15_REG       0x21
#define CPLD_QSFP_MASK_ABS_16_23_REG      0x22
#define CPLD_QSFP_MASK_ABS_24_31_REG      0x23
#define CPLD_QSFP_MASK_INTR_0_7_REG       0x24
#define CPLD_QSFP_MASK_INTR_8_15_REG      0x25
#define CPLD_QSFP_MASK_INTR_16_23_REG     0x26
#define CPLD_QSFP_MASK_INTR_24_31_REG     0x27
#define CPLD_SFP_MASK_ABS_0_1_REG         0x28
#define CPLD_SFP_MASK_RXLOS_0_1_REG       0x29
#define CPLD_SFP_MASK_TXFLT_0_1_REG       0x2A
#define CPLD_QSFP_EVT_ABS_0_7_REG         0x30
#define CPLD_QSFP_EVT_ABS_8_15_REG        0x31
#define CPLD_QSFP_EVT_ABS_16_23_REG       0x32
#define CPLD_QSFP_EVT_ABS_24_31_REG       0x33
#define CPLD_QSFP_EVT_INTR_0_7_REG        0x34
#define CPLD_QSFP_EVT_INTR_8_15_REG       0x35
#define CPLD_QSFP_EVT_INTR_16_23_REG      0x36
#define CPLD_QSFP_EVT_INTR_24_31_REG      0x37
#define CPLD_SFP_EVT_ABS_0_1_REG          0x38
#define CPLD_SFP_EVT_RXLOS_0_1_REG        0x39
#define CPLD_SFP_EVT_TXFLT_0_1_REG        0x3A
#define CPLD_QSFP_RESET_0_7_REG           0x40
#define CPLD_QSFP_RESET_8_15_REG          0x41
#define CPLD_QSFP_RESET_16_23_REG         0x42
#define CPLD_QSFP_RESET_24_31_REG         0x43
#define CPLD_QSFP_LPMODE_0_7_REG          0x44
#define CPLD_QSFP_LPMODE_8_15_REG         0x45
#define CPLD_QSFP_LPMODE_16_23_REG        0x46
#define CPLD_QSFP_LPMODE_24_31_REG        0x47
#define CPLD_SFP_TXDIS_0_1_REG            0x48
#define CPLD_SFP_TS_0_1_REG               0x49
#define CPLD_SFP_RS_0_1_REG               0x4A
#define DBG_CPLD_QSFP_ABS_0_7_REG         0xD0
#define DBG_CPLD_QSFP_ABS_8_15_REG        0xD1
#define DBG_CPLD_QSFP_ABS_16_23_REG       0xD2
#define DBG_CPLD_QSFP_ABS_24_31_REG       0xD3
#define DBG_CPLD_QSFP_INTR_0_7_REG        0xD4
#define DBG_CPLD_QSFP_INTR_8_15_REG       0xD5
#define DBG_CPLD_QSFP_INTR_16_23_REG      0xD6
#define DBG_CPLD_QSFP_INTR_24_31_REG      0xD7
#define DBG_CPLD_SFP_ABS_0_1_REG          0xD8
#define DBG_CPLD_SFP_RXLOS_0_1_REG        0xD9
#define DBG_CPLD_SFP_TXFLT_0_1_REG        0xDA

//MASK
#define MASK_ALL             (0xFF)
#define MASK_NONE            (0x00)
#define MASK_0000_0111       (0x07)
#define MASK_0011_1111       (0x3F)
#define MASK_1100_0000       (0xC0)

/* common manipulation */
#define INVALID(i, min, max)    ((i < min) || (i > max) ? 1u : 0u)

#endif
