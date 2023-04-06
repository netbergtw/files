#include <inc/nb_ports_ctrl.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unit_test/test_util.h>
#include <unit_test/unit_test.h>

#define EEPROM_PRINT 1
#define PORT_NUMBER 32

static nb_qsfp_info_t nb_qsfp_info_arr[PORT_NUMBER + 1];

// fundamental operations
static int nb_led_test(bool single, int port, int mode) {
    int ret;
    if(!single){
        for (int rgb = 1; rgb < 8; rgb++) {
            for (int p = 1; p < PORT_NUMBER + 1; p++) {
                ret = nb_ports_ctrl_port_led_set(p, rgb);
                if (ret < 0) {
                    nb_err("[NB_FPGA][UnitTest] LED set fail : port%d: %d", p, rgb);
                    return ret;
                }
            }
        }
    }else{
        ret = nb_ports_ctrl_port_led_set(port, mode);
        if (ret < 0) {
            nb_err("[NB_FPGA][UnitTest] LED set fail : port%d: %d", port, mode);
            return ret;
        }
    }
    

    return NB_SUCCESS;
}

static int nb_all_get_present_test() {
    int ret;
    uint32_t port;
    uint32_t bit;
    uint32_t present;
    ret = nb_ports_ctrl_presnece_get(&present);
    if (ret < 0) {
        nb_err("[NB_FPGA][UnitTest] get all present error");
        return ret;
    }
    nb_debug("all_present %x\n", present);
    port = 1;
    bit = 1;
    while (port <= PORT_NUMBER) {
        nb_qsfp_info_arr[port].present = present & (bit << (port - 1));
        port++;
    }

    return NB_SUCCESS;
}

static int nb_eeprom_read_test(int port, int off, int len, uint8_t *data, bool print) {
    int ret;

    // exception hadle
    if (!len || off < 0 || off > 256 || off + len > 256) {
        nb_err(" 0 =< off <= 256\n");
        nb_err(" off + len <= 256\n");
        return -NB_FAIL;
    }

    ret = nb_ports_ctrl_port_eeprom_get(port, off, len, data);
    if (ret < 0) {
        nb_err("[NB_FPGA][UnitTest] read eeprom error | port%d off:%d len:%d", port, off, len);
        return ret;
    }

    nb_debug("[NB_FPGA][UnitTest] EERPOM: Port%d off:%d len:%d\n", port, off, len);

    // print format
    int line_len = 16;
    int start_line = off / line_len;
    int start_byte = start_line * line_len;
    char fmt_buf[100];

    if (print) {
        for (int byte = 0; byte < len; byte++) {
            if (!((byte + off) % line_len) || !byte) {
                sprintf(fmt_buf, "line%2d:%3d %3d|", start_line, start_line * line_len, ((start_line + 1) * line_len) - 1);
                start_line++;
            }
            // padding
            while (start_byte < off) {
                sprintf(fmt_buf + strlen(fmt_buf), "** ");
                start_byte++;
            }
            sprintf(fmt_buf + strlen(fmt_buf), "%02x ", *(data + byte));
            if (!((byte + 1 + off) % line_len)) nb_debug("%s", fmt_buf);
        }
        if (((off + len) % line_len)) nb_debug("%s", fmt_buf);
    }

    return NB_SUCCESS;
}

static int nb_eeprom_write_test(int port, int off, int len, uint8_t *eeprom) {
    int ret;

    ret = nb_ports_ctrl_port_eeprom_set(port, off, len, eeprom);
    if (ret < 0) {
        nb_err("[NB_FPGA][UnitTest] write eeprom error | port%d off:%d len:%d", port, off, len);
        return ret;
    }

    return NB_SUCCESS;
}

static int update_page(int port, uint8_t page) {
    int ret;

    ret = nb_eeprom_write_test(port, 127, 1, &page);
    if (ret < 0) {
        nb_err("[NB_FPGA][UnitTest] update eeprom page error | port%d page:%d", port, page);
        return ret;
    }

    return NB_SUCCESS;
}

static int lpmode_set_test(int port, bool lpmode) {
    int ret;

    ret = nb_ports_ctrl_port_lpmode_set(port, lpmode);
    if (ret < 0) {
        nb_err("[NB_FPGA][UnitTest] lpmode set error | port%d ", port);
        return ret;
    }

    return NB_SUCCESS;
}

static int reset_set_test(int port, bool reset) {
    int ret;

    ret = nb_ports_ctrl_port_reset_set(port, reset);
    if (ret < 0) {
        nb_err("[NB_FPGA][UnitTest] reset set error | port%d ", port);
        return ret;
    }

    return NB_SUCCESS;
}

/* SFF-8636, QSFP TRANSCEIVER spec */
// QSFP common

static sff_field_map_t sff8636_field_map[] = {
    /* Base page values, including alarms and sensors */
    {IDENTIFIER, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 0, 1, true}},
    {SPEC_REV, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 1, 1, true}},
    {STATUS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 2, 1, false}},
    {SFF8636_LANE_STATUS_FLAGS,
     {QSFP_BANKNA, QSFP_PAGE0_LOWER, 3, SFF8636_LANE_STATUS_FLAG_BYTES, false}},
    {MODULE_FLAGS,
     {QSFP_BANKNA, QSFP_PAGE0_LOWER, 6, SFF8636_MODULE_FLAG_BYTES, false}},
    {SFF8636_LANE_MONITOR_FLAGS,
     {QSFP_BANKNA,
      QSFP_PAGE0_LOWER,
      9,
      SFF8636_LANE_MONITOR_FLAG_BYTES,
      false}},
    {TEMPERATURE_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 6, 1, false}},
    {VCC_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 7, 1, false}},
    {CHANNEL_RX_PWR_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 9, 2, false}},
    {CHANNEL_TX_BIAS_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 11, 2, false}},
    {CHANNEL_TX_PWR_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 13, 2, false}},
    {TEMPERATURE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 22, 2, false}},
    {VCC, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 26, 2, false}},
    {CHANNEL_RX_PWR, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 34, 8, false}},
    {CHANNEL_TX_BIAS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 42, 8, false}},
    {CHANNEL_TX_PWR, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 50, 8, false}},
    {CHANNEL_TX_DISABLE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 86, 1, false}},
    {POWER_CONTROL, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 93, 1, false}},
    {CDR_CONTROL, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 98, 1, false}},
    {ETHERNET_SECONDARY_COMPLIANCE,
     {QSFP_BANKNA, QSFP_PAGE0_LOWER, 116, 1, true}},
    {PAGE_SELECT_BYTE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 127, 1, false}},

    /* Page 0 values, including vendor info: */
    {PWR_REQUIREMENTS, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 129, 1, true}},
    {CONN_TYPE, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 130, 1, true}},
    {ETHERNET_COMPLIANCE, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 131, 1, true}},
    {LENGTH_SM_KM, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 142, 1, true}},
    {LENGTH_OM3, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 143, 1, true}},
    {LENGTH_OM2, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 144, 1, true}},
    {LENGTH_OM1, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 145, 1, true}},
    {LENGTH_CBLASSY, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 146, 1, true}},
    {VENDOR_NAME, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 148, 16, true}},
    {VENDOR_OUI, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 165, 3, true}},
    {PART_NUMBER, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 168, 16, true}},
    {REVISION_NUMBER, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 184, 2, true}},
    {ETHERNET_EXTENDED_COMPLIANCE,
     {QSFP_BANKNA, QSFP_PAGE0_UPPER, 192, 1, true}},
    {OPTIONS, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 193, 3, true}},
    {VENDOR_SERIAL_NUMBER, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 196, 16, true}},
    {MFG_DATE, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 212, 8, true}},

    /* Page 3 values, including alarm and warning threshold values: */
    {TEMPERATURE_THRESH, {QSFP_BANKNA, QSFP_PAGE3, 128, 8, true}},
    {VCC_THRESH, {QSFP_BANKNA, QSFP_PAGE3, 144, 8, true}},
    {RX_PWR_THRESH, {QSFP_BANKNA, QSFP_PAGE3, 176, 8, true}},
    {TX_BIAS_THRESH, {QSFP_BANKNA, QSFP_PAGE3, 184, 8, true}},
    {TX_PWR_THRESH, {QSFP_BANKNA, QSFP_PAGE3, 192, 8, true}},

};

/* CMIS (Common Management Interface Spec), used in QSFP-DD, OSFP, COBO, etc */
static sff_field_map_t cmis_field_map[] = {
    /* Base page values, including alarms and sensors */
    {IDENTIFIER, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 0, 1, true}},
    {SPEC_REV, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 1, 1, true}},
    {STATUS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 2, 1, true}},
    {MODULE_STATE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 3, 1, false}},
    {MODULE_FLAGS,
     {QSFP_BANKNA, QSFP_PAGE0_LOWER, 8, CMIS_MODULE_FLAG_BYTES, false}},
    {TEMPERATURE_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 9, 1, false}},
    {VCC_ALARMS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 9, 1, false}},
    {TEMPERATURE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 14, 2, false}},
    {VCC, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 16, 2, false}},
    {POWER_CONTROL, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 26, 1, false}}, /* note -
        LowPwr bit (6) first appeared in CMIS 4.0 */
    {FIRMWARE_VER_ACTIVE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 39, 2, false}},
    {MODULE_MEDIA_TYPE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 85, 1, true}},
    {APSEL1_ALL, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 86, BYTES_PER_APP, true}},
    {APSEL1_HOST_ID, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 86, 1, true}},
    {APSEL1_MEDIA_ID, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 87, 1, true}},
    {APSEL1_LANE_COUNTS, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 88, 1, true}},
    {APSEL1_HOST_LANE_OPTION_MASK,
     {QSFP_BANKNA, QSFP_PAGE0_LOWER, 89, 1, true}},
    {PAGE_SELECT_BYTE, {QSFP_BANKNA, QSFP_PAGE0_LOWER, 127, 1, false}},

    /* Upper Page 0 values, including vendor info: */

    // Ethernet compliance is advertised in the Application Codes
    // {ETHERNET_COMPLIANCE},
    // {ETHERNET_EXTENDED_COMPLIANCE},
    // {LENGTH_OM1},  not supported in CMIS modules
    {VENDOR_NAME, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 129, 16, true}},
    {VENDOR_OUI, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 145, 3, true}},
    {PART_NUMBER, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 148, 16, true}},
    {REVISION_NUMBER, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 164, 2, true}},
    {VENDOR_SERIAL_NUMBER, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 166, 16, true}},
    {MFG_DATE, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 182, 8, true}},
    {PWR_CLASS, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 200, 1, true}},
    {PWR_REQUIREMENTS, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 201, 1, true}},
    {LENGTH_CBLASSY, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 202, 1, true}},
    {CONN_TYPE, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 203, 1, true}},
    {MEDIA_INT_TECH, {QSFP_BANKNA, QSFP_PAGE0_UPPER, 212, 1, true}},

    /* page 1 values, active module features */
    {FIRMWARE_VER_INACTIVE, {QSFP_BANKNA, QSFP_PAGE1, 128, 2, false}},
    {HARDWARE_VER, {QSFP_BANKNA, QSFP_PAGE1, 130, 2, true}},
    {LENGTH_SM_KM, {QSFP_BANKNA, QSFP_PAGE1, 132, 1, true}},
    {LENGTH_OM5, {QSFP_BANKNA, QSFP_PAGE1, 133, 1, true}},
    {LENGTH_OM4, {QSFP_BANKNA, QSFP_PAGE1, 134, 1, true}},
    {LENGTH_OM3, {QSFP_BANKNA, QSFP_PAGE1, 135, 1, true}},
    {LENGTH_OM2, {QSFP_BANKNA, QSFP_PAGE1, 136, 1, true}},
    {FLAG_SUPPORT, {QSFP_BANKNA, QSFP_PAGE1, 157, 2, true}},
    {APSEL1_MEDIA_LANE_OPTION_MASK, {QSFP_BANKNA, QSFP_PAGE1, 176, 1, true}},
    {APSEL9_ALL, {QSFP_BANKNA, QSFP_PAGE1, 223, BYTES_PER_APP, true}},
    {APSEL9_HOST_ID, {QSFP_BANKNA, QSFP_PAGE1, 223, 1, true}},
    {APSEL9_MEDIA_ID, {QSFP_BANKNA, QSFP_PAGE1, 224, 1, true}},
    {APSEL9_LANE_COUNTS, {QSFP_BANKNA, QSFP_PAGE1, 225, 1, true}},
    {APSEL9_HOST_LANE_OPTION_MASK, {QSFP_BANKNA, QSFP_PAGE1, 226, 1, true}},

    /* Page 2 values, including alarm and warning threshold values: */
    {TEMPERATURE_THRESH, {QSFP_BANKNA, QSFP_PAGE2, 128, 8, true}},
    {VCC_THRESH, {QSFP_BANKNA, QSFP_PAGE2, 136, 8, true}},
    {RX_PWR_THRESH, {QSFP_BANKNA, QSFP_PAGE2, 192, 8, true}},
    {TX_BIAS_THRESH, {QSFP_BANKNA, QSFP_PAGE2, 184, 8, true}},
    {TX_PWR_THRESH, {QSFP_BANKNA, QSFP_PAGE2, 176, 8, true}},

    /* Page 16 (banked), data path control */
    {DATAPATH_DEINIT, {QSFP_BANKCH, QSFP_PAGE16, 128, 1, false}},
    {CHANNEL_TX_DISABLE, {QSFP_BANKCH, QSFP_PAGE16, 130, 1, false}},
    {APPLY_DATAPATHINIT_SS0, {QSFP_BANKCH, QSFP_PAGE16, 143, 1, false}},
    {APPLY_IMMEDIATE_SS0, {QSFP_BANKCH, QSFP_PAGE16, 144, 1, false}},  // Netberg
    {DATAPATH_CFG_ALL_STAGED_SET0, {QSFP_BANKCH, QSFP_PAGE16, 145, 33, false}},
    {APPLICATION_SELECT_LN1, {QSFP_BANKCH, QSFP_PAGE16, 145, 1, false}},
    {RX_EQ, {QSFP_BANKCH, QSFP_PAGE16, 162, 1, false}},  // Netberg
    {DATAPATH_CFG_ALL_STAGED_SET1, {QSFP_BANKCH, QSFP_PAGE16, 180, 33, false}},

    /* Page 17 (banked), data path status */
    {DATAPATH_STATE_ALL, {QSFP_BANKCH, QSFP_PAGE17, 128, 4, false}},
    {DATAPATH_STATE_LN1AND2, {QSFP_BANKCH, QSFP_PAGE17, 128, 1, false}},
    {CMIS_LANE_FLAGS,
     {QSFP_BANKCH, QSFP_PAGE17, 134, CMIS_LANE_FLAG_BYTES, false}},
    {CHANNEL_TX_PWR_ALARMS, {QSFP_BANKCH, QSFP_PAGE17, 139, 4, false}},
    {CHANNEL_TX_BIAS_ALARMS, {QSFP_BANKCH, QSFP_PAGE17, 143, 4, false}},
    {CHANNEL_RX_PWR_ALARMS, {QSFP_BANKCH, QSFP_PAGE17, 149, 4, false}},
    {CHANNEL_TX_PWR, {QSFP_BANKCH, QSFP_PAGE17, 154, 16, false}},
    {CHANNEL_TX_BIAS, {QSFP_BANKCH, QSFP_PAGE17, 170, 16, false}},
    {CHANNEL_RX_PWR, {QSFP_BANKCH, QSFP_PAGE17, 186, 16, false}},
    {DATAPATH_CFG_STATUS, {QSFP_BANKCH, QSFP_PAGE17, 202, 1, false}},
    {DATAPATH_CFG_ALL_ACTIVE_SET, {QSFP_BANKCH, QSFP_PAGE17, 206, 29, false}},
    {LN1_ACTIVE_SET, {QSFP_BANKCH, QSFP_PAGE17, 206, 1, false}},
};

/* cable length multiplier values, from SFF specs */
static const sff_field_mult_t qsfp_multiplier[] = {
    {LENGTH_SM_KM, 1000},
    {LENGTH_OM5, 2},
    {LENGTH_OM4, 2},
    {LENGTH_OM3, 2},
    {LENGTH_OM2, 1},
    {LENGTH_OM1, 1},
    {LENGTH_CBLASSY, 1},
};

bool nb_qsfp_is_cmis(int port) {
    if (port > PORT_NUMBER) {
        return false;
    }

    if (nb_qsfp_info_arr[port].memmap_format >= MMFORMAT_CMIS3P0) {
        return true;
    }
    return false;
}

bool nb_qsfp_is_sff8636(int port) {
    if (port > PORT_NUMBER) {
        return false;
    }

    if (nb_qsfp_info_arr[port].memmap_format == MMFORMAT_SFF8636) {
        return true;
    }
    return false;
}

/* return sff_field_info_t for a given field in sff8636_field_map[] */
static sff_field_info_t *get_sff_field_addr(const Sff_field field) {
    int i, cnt = sizeof(sff8636_field_map) / sizeof(sff_field_map_t);

    for (i = 0; i < cnt; i++) {
        if (sff8636_field_map[i].field == field) {
            return (&(sff8636_field_map[i].info));
        }
    }
    return NULL;
}

/* return sff_field_info_t for a given field in cmis_field_map[] */
static sff_field_info_t *get_cmis_field_addr(const Sff_field field) {
    int i, cnt = sizeof(cmis_field_map) / sizeof(sff_field_map_t);

    for (i = 0; i < cnt; i++) {
        if (cmis_field_map[i].field == field) {
            return (&(cmis_field_map[i].info));
        }
    }
    return NULL;
}

/* return the contents of the  sff_field_info_t for a given field */
static int get_sff_field_info(int port,
                              Sff_field field,
                              sff_field_info_t **info) {
    if (field >= SFF_FIELD_MAX) {
        return -1;
    }
    if (field == IDENTIFIER) {
        // we may not know if we're CMIS or SFF-8636. Luckily, this field is
        // common to both
        *info = get_sff_field_addr(field);
    } else if (nb_qsfp_is_cmis(port)) {
        *info = get_cmis_field_addr(field);
    } else {
        *info = get_sff_field_addr(field);
    }
    if (!*info) {
        return -1;
    }
    return 0;
}

static bool nb_qsfp_is_flat_mem(int port) {
    if (port > PORT_NUMBER) {
        return false;
    }
    return nb_qsfp_info_arr[port].flat_mem;
}

static void nb_qsfp_idprom_clr(int port) {
    if (port > PORT_NUMBER) {
        return;
    }
    memset(&(nb_qsfp_info_arr[port].idprom[0]), 0, MAX_QSFP_PAGE_SIZE);
    memset(&(nb_qsfp_info_arr[port].page0[0]), 0, MAX_QSFP_PAGE_SIZE);
    memset(&(nb_qsfp_info_arr[port].page1[0]), 0, MAX_QSFP_PAGE_SIZE);
    memset(&(nb_qsfp_info_arr[port].page2[0]), 0, MAX_QSFP_PAGE_SIZE);
    memset(&(nb_qsfp_info_arr[port].page3[0]), 0, MAX_QSFP_PAGE_SIZE);

    nb_qsfp_info_arr[port].cache_dirty = true;
}

static int nb_qsfp_init(void) {
    int port;

    for (port = 0; port <= PORT_NUMBER; port++) {
        nb_qsfp_idprom_clr(port);
        nb_qsfp_info_arr[port].present = false;
        nb_qsfp_info_arr[port].reset = false;
        nb_qsfp_info_arr[port].flat_mem = true;
        nb_qsfp_info_arr[port].passive_cu = true;
        nb_qsfp_info_arr[port].num_ch = 0;
        nb_qsfp_info_arr[port].memmap_format = MMFORMAT_UNKNOWN;
        nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs = false;
        // nb_qsfp_info_arr[port].rxlos_debounce_cnt = QSFP_RXLOS_DEBOUNCE_DFLT;
        // nb_qsfp_info_arr[port].checksum = true;
    }
    return 0;
}

static bool is_qsfp_addr_in_cache(int port, sff_field_info_t *info) {
    // assumes calling routine is handling port availability checks
    if (nb_qsfp_info_arr[port].cache_dirty) {
        return false;
    }

    return info->in_cache;
}

/* do not call this directly, go through nb_qsfp_field_read_onebank */
static bool nb_qsfp_read_cache(int port,
                               sff_field_info_t *info,
                               uint8_t *data) {
    // is this field in the cache? (currently, only non-banked pages are cached)
    if ((!is_qsfp_addr_in_cache(port, info)) || (info->bank != QSFP_BANKNA)) {
        return false;  // not in cache
    }

    // get pointer to cache location containing data
    uint8_t *cache_loc;
    if (info->page == QSFP_PAGE0_LOWER) {
        cache_loc = &nb_qsfp_info_arr[port].idprom[0 + info->offset];
    } else {
        if (info->page == QSFP_PAGE0_UPPER) {
            cache_loc =
                &nb_qsfp_info_arr[port].page0[info->offset - MAX_QSFP_PAGE_SIZE];
        } else if (!nb_qsfp_is_flat_mem(port)) {
            switch (info->page) {
                case QSFP_PAGE1:
                    cache_loc =
                        &nb_qsfp_info_arr[port].page1[info->offset - MAX_QSFP_PAGE_SIZE];
                    break;
                case QSFP_PAGE2:
                    cache_loc =
                        &nb_qsfp_info_arr[port].page2[info->offset - MAX_QSFP_PAGE_SIZE];
                    break;
                case QSFP_PAGE3:
                    cache_loc =
                        &nb_qsfp_info_arr[port].page3[info->offset - MAX_QSFP_PAGE_SIZE];
                    break;
                default:
                    nb_err("Invalid Page value %d\n", info->page);
                    return false;
            }
        } else {
            nb_err("Invalid Page value %d\n", info->page);
            return false;
        }
    }

    // copy cached data into the memory location provided by  calling routine
    memcpy(data, cache_loc, info->length);
    return true;
}

static int nb_qsfp_field_read_onebank(int port,
                                      Sff_field field,
                                      int chmask,
                                      int addl_offset,
                                      int max_length,
                                      uint8_t *data) {
    sff_field_info_t *info = NULL;
    int offset;

    // retrieve the field address info
    if (get_sff_field_info(port, field, &info) < 0) {
        nb_err("QSFP    %2d : field %d not decodable", port, field);
        return -1;
    }

    if (!info) {
        nb_err("QSFP    %2d : field %d decoded to NULL", port, field);
        return -1;
    }

    // make sure the buffer is big enough
    if (info->length > max_length) {
        nb_err(
            "QSFP    %2d : field %d length exceeds available buffer", port, field);
        return -1;
    }

    // add in any additional offset for arrays
    offset = info->offset + addl_offset;

    if ((offset < 0) || ((offset + info->length) > MAX_QSFP_PAGE_SIZE_255)) {
        nb_err("qsfp %d offset (%d) or length (%d) for field %d is invalid\n",
               port,
               offset,
               info->length,
               field);
        return -1;
    }

    // check cache first
    if (!nb_qsfp_read_cache(port, info, data)) {
        // data is not in cache
        if (!nb_qsfp_is_flat_mem(port) && info->page != QSFP_PAGE0_LOWER) {
            update_page(port, info->page);
        } else {
            // update_page(port, 0);
        }

        // read that location
        if (nb_eeprom_read_test(port, offset, info->length, data, EEPROM_PRINT) < 0) {
            if (!nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs) {
                nb_err("qsfp %d read failed\n", port);
            }
            return -1;
        }
    }
    return 0;
}

static int nb_qsfp_module_read(unsigned int port, int page, int offset, int len, uint8_t *buf) {
    int ret = 0;

    if (port > PORT_NUMBER) {
        return -1;
    }

    if (page == QSFP_PAGE0_LOWER) {
        page = 0;
    }

    if (!nb_qsfp_is_flat_mem(port) && (offset >= MAX_QSFP_PAGE_SIZE)) {
        uint8_t pg = (uint8_t)page;
        ret = update_page(port, pg);
    }
    ret |= nb_eeprom_read_test(port, offset, len, buf, EEPROM_PRINT);

    return ret;
}

// returns the spec revision, CMIS modules only
static int nb_cmis_spec_rev_get(int port, uint8_t *major, uint8_t *minor) {
    uint8_t rev;
    if (nb_qsfp_field_read_onebank(port, SPEC_REV, 0, 0, 1, &rev) < 0) {
        return -1;
    }
    if (major != NULL) {
        *major = rev >> 4;
    }
    if (minor != NULL) {
        *minor = rev & 0xF;
    }
    return 0;
}

static int set_qsfp_idprom(int port) {
    uint8_t id;
    uint8_t major_rev;
    uint8_t status;

    if (port > PORT_NUMBER) {
        return -1;
    }
    if (!nb_qsfp_info_arr[port].present) {
        nb_err("QSFP %d IDProm set failed as QSFP is not present\n", port);
        return -1;
    }

    // determine the module type first
    if (nb_qsfp_field_read_onebank(port, IDENTIFIER, 0, 0, 1, &id) < 0) {
        if (!nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs) {
            nb_err("Unable to obtain Identifier field in QSFP %d\n", port);
        }
        return -1;
    }

    if ((id == QSFP) || (id == QSFP_PLUS) || (id == QSFP_28)) {
        nb_qsfp_info_arr[port].memmap_format = MMFORMAT_SFF8636;
        nb_qsfp_info_arr[port].num_ch = 4;
    } else {  // CMIS
        // now determine if the CMIS memory map is rev 3.0 or 4.0+
        if (nb_cmis_spec_rev_get(port, &major_rev, NULL) < 0) {
            nb_err("Unable to obtain revision field in module %d\n", port);
            return -1;
        }
        if (major_rev < 4) {  // CMIS 3.X
            nb_qsfp_info_arr[port].memmap_format = MMFORMAT_CMIS3P0;
        } else if (major_rev < 5) {  // CMIS 4.0 or newer. There was a break in
                                     // backwards compatibility
            nb_qsfp_info_arr[port].memmap_format = MMFORMAT_CMIS4P0;
        } else {
            nb_qsfp_info_arr[port].memmap_format = MMFORMAT_CMIS5P0;
        }
        // identify the number of channels in the module, based on the module type
        if (id == QSFP_CMIS) {
            nb_qsfp_info_arr[port].num_ch = 4;
        } else if ((id == QSFP_DD) || (id == OSFP)) {
            nb_qsfp_info_arr[port].num_ch = 8;
        }
    }

    // now that we know the memory map format, we can determine if it is flat or
    // paged memory
    if (nb_qsfp_field_read_onebank(port, STATUS, 0, 0, 1, &status) < 0) {
        nb_err("Unable to obtain status field in module %d\n", port);
        return -1;
    }
    if (nb_qsfp_info_arr[port].memmap_format == MMFORMAT_SFF8636) {
        nb_qsfp_info_arr[port].flat_mem = (status >> 2) & 1;
    } else {  // CMIS and UNKNOWN
        nb_qsfp_info_arr[port].flat_mem = (status >> 7) & 1;
    }
    return 0;
}

static int cmis_get_media_type(int port, uint8_t *media_type) {
    if (port > PORT_NUMBER) {
        return -1;
    }

    if (nb_qsfp_field_read_onebank(port, MODULE_MEDIA_TYPE, 0, 0, 1, media_type) <
        0) {
        return -1;
    }
    return 0;
}

/* return the qsfp multiplier for a given field in struct qsfp_multiplier[] */
/* this function does not handle the CMIS upper nibble multiplier for
 * cable assys */
static uint32_t get_qsfp_multiplier(Sff_field field) {
    int i, cnt = sizeof(qsfp_multiplier) / sizeof(sff_field_mult_t);

    for (i = 0; i < cnt; i++) {
        if (qsfp_multiplier[i].field == field) {
            return (qsfp_multiplier[i].value);
        }
    }
    return 0; /* invalid field value */
}

static bool nb_qsfp_is_eth_ext_compliance_copper(uint8_t ext_comp) {
    // fitler only copper cases; all other cases are optical
    switch (ext_comp) {
        case 0xB:
        case 0xC:
        case 0xD:
        case 0x16:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x40:
            return true;
        default:
            break;
    }
    return false;
}

static float get_qsfp_cable_length(int port, Sff_field fieldName) {
    uint8_t data = 0;
    float multiplier, length;

    if (nb_qsfp_field_read_onebank(port, fieldName, 0, 0, 1, &data) < 0) {
        return 0.0;
    }

    multiplier = get_qsfp_multiplier(fieldName);
    if ((nb_qsfp_is_cmis(port)) && (fieldName == LENGTH_CBLASSY)) {
        // multiplier is built into upper 2 bits of memory map data
        multiplier = pow(10, (data >> 6) - 1);
        data &= 0x1F;
    } else {
        multiplier = get_qsfp_multiplier(fieldName);
    }

    length = data * multiplier;
    if (data == MAX_CABLE_LEN) {
        length = -(MAX_CABLE_LEN - 1) * multiplier;
    }
    return length;
}

static int nb_qsfp_get_eth_compliance(int port, Ethernet_compliance *code) {
    uint8_t eth_compliance;

    if (port > PORT_NUMBER) {
        return -1;
    }
    if (nb_qsfp_field_read_onebank(
            port, ETHERNET_COMPLIANCE, 0, 0, 1, &eth_compliance) < 0) {
        *code = COMPLIANCE_NONE;
        return -1;
    }
    *code = eth_compliance;
    return 0;
}

static int nb_qsfp_get_eth_ext_compliance(int port,
                                          Ethernet_extended_compliance *code) {
    uint8_t eth_ext_comp;

    if (port > PORT_NUMBER) {
        return -1;
    }
    if (nb_qsfp_field_read_onebank(
            port, ETHERNET_EXTENDED_COMPLIANCE, 0, 0, 1, &eth_ext_comp) < 0) {
        *code = EXT_COMPLIANCE_NONE;
        return -1;
    }
    *code = eth_ext_comp;
    return 0;
}

static int nb_qsfp_type_get(int port, nb_pltfm_qsfp_type_t *qsfp_type) {
    bool type_copper = true;  // default to copper

    if (port > PORT_NUMBER) {
        return -1;
    }

    Ethernet_compliance eth_comp;
    if (nb_qsfp_get_eth_compliance(port, &eth_comp) != 0) {
        // Default to Copper Loop back in case of error

        *qsfp_type = BF_PLTFM_QSFP_CU_LOOP;
        return -1;
    }

    /* there are some odd module/cable types that have inconsistent information
     * in it. We need to categorise them seperately
     */
    /*bypass special case handle
    if (nb_qsfp_special_case_handled(port, qsfp_type, &type_copper)) {
      return 0;
    }
    */

    /* SFF standard is not clear about necessity to look at both compliance
     * code and the extended compliance code. From the cable types known so far,
     * if bit-7 of compliance code is set, then, we look at extended compliance
     * code as well. Otherwise, we characterize the QSFP by regular compliance
     * code only.
     */
    if (eth_comp & 0x77) {  // all except 40GBASE-CR4 and ext compliance bits
        type_copper = false;
    } else if (eth_comp & 0x80) {
        Ethernet_extended_compliance ext_comp;
        // See SFF-8024 spec rev 4.6.1
        if (nb_qsfp_get_eth_ext_compliance(port, &ext_comp) == 0) {
            type_copper = nb_qsfp_is_eth_ext_compliance_copper(ext_comp);
        } else {
            type_copper = true;
        }
    }
    if (!type_copper) {  // we are done if found optical module
        *qsfp_type = BF_PLTFM_QSFP_OPT;
        return 0;
    }

    uint8_t cable_len;
    if (nb_qsfp_field_read_onebank(port, LENGTH_CBLASSY, 0, 0, 1, &cable_len) <
        0) {
        return -1;
    }
    switch (cable_len) {
        case 0:
            *qsfp_type = BF_PLTFM_QSFP_CU_LOOP;
            return 0;
        case 1:
            *qsfp_type = BF_PLTFM_QSFP_CU_1_M;
            return 0;
        case 2:
            *qsfp_type = BF_PLTFM_QSFP_CU_2_M;
            return 0;
        default:
            *qsfp_type = BF_PLTFM_QSFP_CU_3_M;
            return 0;
    }
    return 0;
}

static int nb_cmis_type_get(int port, nb_pltfm_qsfpdd_type_t *qsfp_type) {
    if (port > PORT_NUMBER) {
        return -1;
    }
    uint8_t media_type;
    if (nb_qsfp_field_read_onebank(
            port, MODULE_MEDIA_TYPE, 0, 0, 1, &media_type) < 0) {
        nb_debug("MODULE_MEDIA_TYPE error\n");
        *qsfp_type = BF_PLTFM_QSFPDD_UNKNOWN;
        return 0;
    }
    if ((media_type == MEDIA_TYPE_SMF) || (media_type == MEDIA_TYPE_MMF) ||
        (media_type == MEDIA_TYPE_ACTIVE_CBL)) {
        *qsfp_type = BF_PLTFM_QSFPDD_OPT;
        return 0;
    }

    // if we get here, it's a copper cable. figure out its characteristics
    float cable_len = get_qsfp_cable_length(port, LENGTH_CBLASSY);
    if (cable_len == 0.0) {
        *qsfp_type = BF_PLTFM_QSFPDD_CU_LOOP;
        return 0;
    } else if (cable_len <= 0.5f) {
        *qsfp_type = BF_PLTFM_QSFPDD_CU_0_5_M;
        return 0;
    } else if (cable_len <= 1.0f) {
        *qsfp_type = BF_PLTFM_QSFPDD_CU_1_M;
        return 0;
    } else if (cable_len <= 1.5f) {
        *qsfp_type = BF_PLTFM_QSFPDD_CU_1_5_M;
        return 0;
    } else if (cable_len == 2.0f) {
        *qsfp_type = BF_PLTFM_QSFPDD_CU_2_M;
        return 0;
    } else if (cable_len == 2.5f) {
        *qsfp_type = BF_PLTFM_QSFPDD_CU_2_5_M;
        return 0;
    } else {  // For all other lengths default to max supported.
        nb_debug("QSFPDD length %f unsupported for Qsfp %d. Defaulting to 2.5m`n\n",
                 cable_len,
                 port);
        *qsfp_type = BF_PLTFM_QSFPDD_CU_2_5_M;
    }

    return 0;
}

static bool nb_qsfp_is_passive_cu(int port) {
    if (port > PORT_NUMBER) {
        return false;
    }
    return nb_qsfp_info_arr[port].passive_cu;
}

static int cmis_get_application_data(int port,
                                     int apsel_code,
                                     uint8_t *host_id,
                                     uint8_t *media_id,
                                     uint8_t *host_lane_count,
                                     uint8_t *media_lane_count,
                                     uint8_t *host_lane_option_mask,
                                     uint8_t *media_lane_option_mask) {
    uint8_t app_data[BYTES_PER_APP];
    if (port > PORT_NUMBER) {
        return -1;
    }
    if (apsel_code > 15) {
        return -1;
    }

    if (apsel_code <= 8) {
        if (nb_qsfp_field_read_onebank(port,
                                       APSEL1_ALL,
                                       0,
                                       ((apsel_code - 1) * BYTES_PER_APP),
                                       BYTES_PER_APP,
                                       app_data) < 0) {
            return -1;
        }
    } else {
        if (nb_qsfp_field_read_onebank(port,
                                       APSEL9_ALL,
                                       0,
                                       ((apsel_code - 9) * BYTES_PER_APP),
                                       BYTES_PER_APP,
                                       app_data) < 0) {
            return -1;
        }
    }

    *host_id = app_data[0];
    *media_id = app_data[1];
    *host_lane_count = app_data[2] >> 4;
    *media_lane_count = app_data[2] & 0xF;
    *host_lane_option_mask = app_data[3];

    if (!nb_qsfp_is_flat_mem(port)) {
        if (nb_qsfp_field_read_onebank(port,
                                       APSEL1_MEDIA_LANE_OPTION_MASK,
                                       0,
                                       (apsel_code - 1),
                                       1,
                                       media_lane_option_mask) < 0) {
            return -1;
        }
    }
    return 0;
}

static int cmis_calc_application_count(int port, int *app_count) {
    if (port > PORT_NUMBER) {
        return -1;
    }
    uint8_t host_id;

    *app_count = 0;
    do {
        nb_qsfp_field_read_onebank(
            port, APSEL1_HOST_ID, 0, (*app_count * BYTES_PER_APP), 1, &host_id);
        (*app_count)++;
        //  nb_debug("QSFP    %2d : App00 %d : host_id 0x%x", port, cur_app,
        //  host_id);
    } while ((host_id != 0xFF) && (*app_count < 8));

    if ((host_id != 0xFF) && (!nb_qsfp_is_flat_mem(port))) {
        // not yet at end of list, list continues on Pg 1
        do {
            nb_qsfp_field_read_onebank(port,
                                       APSEL9_HOST_ID,
                                       0,
                                       ((*app_count - 9) * BYTES_PER_APP),
                                       1,
                                       &host_id);
            (*app_count)++;
            //    nb_debug("QSFP %2d : App01 %d : host_id 0x%x", port, cur_app,
            //    host_id);
        } while ((host_id != 0xFF) && (*app_count < 15));
    }
    (*app_count)--;
    return 0;
}

// populate nb_qsfp_info_arr with the supported Application details
static int qsfp_cmis_populate_app_list(int port) {
    int cur_app_num = 0;
    while (cur_app_num < nb_qsfp_info_arr[port].num_apps) {
        if (cmis_get_application_data(
                port,
                cur_app_num + 1,
                &nb_qsfp_info_arr[port].app_list[cur_app_num].host_if_id,
                &nb_qsfp_info_arr[port].app_list[cur_app_num].media_if_id,
                &nb_qsfp_info_arr[port].app_list[cur_app_num].host_lane_cnt,
                &nb_qsfp_info_arr[port].app_list[cur_app_num].media_lane_cnt,
                &nb_qsfp_info_arr[port].app_list[cur_app_num].host_lane_assign_mask,
                &nb_qsfp_info_arr[port]
                     .app_list[cur_app_num]
                     .media_lane_assign_mask) != 0) {
            return -1;
        }
        cur_app_num++;
    }
    return 0;
}

static int nb_qsfp_update_cache(int port) {
    if (port > PORT_NUMBER) {
        return -1;
    }
    nb_qsfp_idprom_clr(port);

    if (!nb_qsfp_info_arr[port].present) {
        return 0;  // not present, exit now that the cache is cleared
    }

    // first, figure out memory map format.
    if (set_qsfp_idprom(port)) {
        if (!nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs) {
            nb_err("Error setting idprom for qsfp %d\n", port);
        }
        return -1;
    }

    nb_debug(
        "QSFP    %2d : %s\n", port, nb_qsfp_is_cmis(port) ? "CMIS" : "SFF-8636");

    nb_debug(
        "QSFP    %2d : %s\n", port, nb_qsfp_is_flat_mem(port) ? "Flat" : "Paged");

    nb_debug(
        "QSFP    %2d : Lane count = %d\n", port, nb_qsfp_info_arr[port].num_ch);

    // now that we know the memory map format, we can selectively read the
    // locations that are static into our cache. The static locations of CMIS and
    // SFF-8636 are not even close to the same. The _field_map variables indicate
    // if each field is stored in the cache or not

    // page 00h (lower), bytes 0-2
    if (nb_qsfp_module_read(port,
                            0,
                            0,
                            3,
                            &nb_qsfp_info_arr[port].idprom[0]) < 0) {
        nb_debug("Error reading Qsfp %d lower memory\n", port);
        return -1;
    }

    // BOTH module types, CMIS and SFF-8636, flat and paged
    // page 00h (upper), bytes 128-255 - read first because we'll need this
    if (nb_qsfp_module_read(port,
                            0,
                            128,
                            128,
                            &nb_qsfp_info_arr[port].page0[0]) < 0) {
        nb_debug("Error reading Qsfp %d uppper memory\n", port);
        return -1;
    }

    // bypass checksum check

    if (nb_qsfp_is_cmis(port)) {
        nb_debug("is cmis\n");
        // page 00h (lower), bytes 85-117
        if (nb_qsfp_module_read(port,
                                0,
                                85,
                                33,
                                &nb_qsfp_info_arr[port].idprom[85]) < 0) {
            nb_debug("Error reading Qsfp %d lower memory\n", port);
            return -1;
        }
        nb_debug("read lower memory done\n");
        if (!nb_qsfp_is_flat_mem(port)) {
            // page 01h, bytes 130-255
            if (nb_qsfp_module_read(port,
                                    1,
                                    130,
                                    126,
                                    &nb_qsfp_info_arr[port].page1[2]) < 0) {
                nb_debug("Error reading Qsfp %d page 1\n", port);
                return -1;
            }

            // page 02h, bytes 128-255
            if (nb_qsfp_module_read(port,
                                    2,
                                    128,
                                    128,
                                    &nb_qsfp_info_arr[port].page2[0]) < 0) {
                nb_debug("Error reading Qsfp %d page 2\n", port);
                return -1;
            }
        }

        if (cmis_get_media_type(port, &nb_qsfp_info_arr[port].media_type) != 0) {
            nb_debug(
                "Error : in getting the CMIS media type type of the QSFP at "
                "%s:%d\n",
                __func__,
                __LINE__);
        }

        nb_pltfm_qsfpdd_type_t cmis_type;
        if (nb_cmis_type_get(port, &cmis_type) != 0) {
            nb_err("Error: in getting the CMIS type for port %d at %s:%d",
                   port,
                   __func__,
                   __LINE__);
        }

        if (cmis_type == BF_PLTFM_QSFPDD_OPT) {
            nb_qsfp_info_arr[port].passive_cu = false;
        } else {
            nb_qsfp_info_arr[port].passive_cu = true;
        }

        if (cmis_calc_application_count(port, &nb_qsfp_info_arr[port].num_apps) !=
            0) {
            nb_err(
                "Error : in getting the number of CMIS Applications at "
                "%s:%d\n",
                __func__,
                __LINE__);
        }
        nb_debug("QSFP    %2d : Application count = %d\n",
                 port,
                 nb_qsfp_info_arr[port].num_apps);

        nb_qsfp_info_arr[port].app_list = (nb_pm_qsfp_apps_t *)malloc(
            nb_qsfp_info_arr[port].num_apps * sizeof(nb_pm_qsfp_apps_t));
        if (!nb_qsfp_info_arr[port].app_list) {
            nb_err(
                "Error in malloc for app_list at "
                "%s:%d\n",
                __func__,
                __LINE__);
            return -1;
        }

        if (qsfp_cmis_populate_app_list(port) != 0) {
            nb_err(
                "Error : populating the Application list at "
                "%s:%d\n",
                __func__,
                __LINE__);
        }
    } else {  // SFF-8636
        // page 00h (lower), bytes 107-116
        if (nb_qsfp_module_read(port,
                                0,
                                107,
                                10,
                                &nb_qsfp_info_arr[port].idprom[107]) < 0) {
            nb_debug("Error reading Qsfp %d lower memory\n", port);
            return -1;
        }

        nb_pltfm_qsfp_type_t qsfp_type;
        if (nb_qsfp_type_get(port, &qsfp_type) != 0) {
            nb_err("Error: in getting the QSFP type for port %d at %s:%d",
                   port,
                   __func__,
                   __LINE__);
        }
        if (qsfp_type == BF_PLTFM_QSFP_OPT) {
            nb_qsfp_info_arr[port].passive_cu = false;
        } else {
            nb_qsfp_info_arr[port].passive_cu = true;
        }

        if (!nb_qsfp_is_flat_mem(port)) {
            // page 03h, bytes 128-229
            if (nb_qsfp_module_read(port,
                                    3,
                                    128,
                                    102,
                                    &nb_qsfp_info_arr[port].page3[0]) < 0) {
                nb_debug("Error reading Qsfp %d page 3\n", port);
                return -1;
            }
        }
        /*bypass sff8636 pupulate app list
        if (qsfp_sff8636_populate_app_list(port) != 0) {
            nb_err(
                "Error : populating the Ethernet compliance list at "
                "%s:%d\n",
                __func__,
                __LINE__);
        }
        */
    }

    nb_debug("QSFP    %2d : %s\n",
             port,
             nb_qsfp_is_passive_cu(port) ? "Passive copper" : "Active/Optical");

    nb_qsfp_info_arr[port].cache_dirty = false;
    nb_debug("QSFP    %2d : Update cache complete\n", port);
    return 0;
}

static int nb_qsfp_detect_transceiver(int port) {
    if (port < 0 || port > PORT_NUMBER) {
        return -1;
    }
    bool st = nb_qsfp_info_arr[port].present;

    if (nb_qsfp_info_arr[port].cache_dirty) {
        if (!nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs) {
            nb_debug("QSFP    %2d : %s\n", port, st ? "PRESENT" : "REMOVED");
        }
        if (st) {
            if (nb_qsfp_update_cache(port)) {
                /* NOTE, we do not clear IDPROM data here so that we can read
                 * whatever data it returned.
                 */
                nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs = true;
                return -1;
            }
            nb_qsfp_info_arr[port].suppress_repeated_rd_fail_msgs = false;
            // nb_qsfp_set_pwr_ctrl(port);  don't power up the module here, do it in
            // the module FSM
        }
    }
    return 0;
}

int BSP_behavior_test(int test_cycle) {
    int ret;
    int port;
    if (!test_cycle) {
        nb_err("test_cycle count must > 0");
        return -NB_FAIL;
    }

    for (int ctr = 1; ctr <= test_cycle; ctr++) {
        nb_info("[Unit Test] Test:%d", ctr);
        nb_led_test(false, 0, 0);
        // intializations
        nb_qsfp_init();
        // 1 assert all reset and lpmode
        for (port = 1; port <= PORT_NUMBER; port++) {
            ret = lpmode_set_test(port, true);
            if (ret < 0) goto exc_fail;
            ret = reset_set_test(port, true);
            if (ret < 0) goto exc_fail;
        }
        // 2 get all present
        ret = nb_all_get_present_test();
        if (ret < 0) goto exc_fail;
        for (port = 1; port <= PORT_NUMBER; port++) {
            ret = nb_all_get_present_test();
            if (ret < 0) goto exc_fail;
            if (nb_qsfp_info_arr[port].present) {
                ret = nb_led_test(true, port, 3);
                if (ret < 0) goto exc_fail;
                // 3 dasser presented port's reset signel
                ret = reset_set_test(port, false);
                if (ret < 0) goto exc_fail;
                sleep(1);
                // 4 updata caches
                nb_qsfp_detect_transceiver(port);
                if (ret < 0) goto exc_fail;
                ret = nb_led_test(true, port, 0);
                if (ret < 0) goto exc_fail;
            }else{
                ret = nb_led_test(true, port, 0);   
            }
        }
    }

    if (ret < 0) goto exc_fail;
    return NB_SUCCESS;
exc_fail:
    return -NB_FAIL;
}
