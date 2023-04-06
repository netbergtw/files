#ifndef _UNIT_UTIL_H
#define _UNIT_UTIL_H

#include <stdint.h>


#define NB_SUCCESS 0
#define NB_FAIL 1

#define SFF8636_MODULE_FLAG_BYTES 3
#define CMIS_MODULE_FLAG_BYTES 6
#define SFF8636_LANE_STATUS_FLAG_BYTES 3
#define SFF8636_LANE_MONITOR_FLAG_BYTES 13
#define CMIS_LANE_FLAG_BYTES 19

#define BYTES_PER_APP 4

enum {
    /* Size of page read from QSFP via I2C */
    MAX_QSFP_PAGE_SIZE = 128,
    /* limit of memory address space in QSFP */
    MAX_QSFP_PAGE_SIZE_255 = 255,
    /* Maximum number of channels per module */
    CHANNEL_COUNT = 16,
    /* Maximum cable length reported */
    MAX_CABLE_LEN = 255,
};

// Commonly used QSFP variants
typedef enum {
    UNKNOWN = 0,
    QSFP = 0x0C,
    QSFP_PLUS = 0x0D,
    QSFP_28 = 0x11,
    QSFP_DD = 0x18,
    OSFP = 0x19,
    QSFP_CMIS = 0x1E,
} Module_identifier;

// memory map formats (IDENTIFIER_OFFSET field is used to determine this)
typedef enum {
    MMFORMAT_UNKNOWN,
    MMFORMAT_SFF8636,
    MMFORMAT_CMIS3P0,
    MMFORMAT_CMIS4P0,
    MMFORMAT_CMIS5P0,
} MemMap_Format;

typedef enum {
    QSFP_BANKNA = -2,
    QSFP_BANKCH = -1,  // the bank number is based on the channel
    QSFP_BANK0 = 0,
    QSFP_BANK1,
    QSFP_BANK2,
    QSFP_BANK3
} Qsfp_bank;

typedef enum {
    QSFP_PAGE0_LOWER = -1,
    QSFP_PAGE0_UPPER,
    QSFP_PAGE1,
    QSFP_PAGE2,
    QSFP_PAGE3,
    QSFP_PAGE16 = 16,
    QSFP_PAGE17 = 17
} Qsfp_page;

/*
 * Identifies the type of the QSFP-DD connected
 */
typedef enum nb_pltfm_qsfpdd_type_t {
    BF_PLTFM_QSFPDD_CU_0_5_M = 0,
    BF_PLTFM_QSFPDD_CU_1_M = 1,
    BF_PLTFM_QSFPDD_CU_1_5_M = 2,
    BF_PLTFM_QSFPDD_CU_2_M = 3,
    BF_PLTFM_QSFPDD_CU_2_5_M = 4,
    BF_PLTFM_QSFPDD_CU_LOOP = 5,
    BF_PLTFM_QSFPDD_OPT = 6,

    // Keep this last
    BF_PLTFM_QSFPDD_UNKNOWN,
} nb_pltfm_qsfpdd_type_t;

/*
 * Identifies the type of the QSFP connected
 */
typedef enum nb_pltfm_qsfp_type_t {

    BF_PLTFM_QSFP_CU_0_5_M = 0,
    BF_PLTFM_QSFP_CU_1_M = 1,
    BF_PLTFM_QSFP_CU_2_M = 2,
    BF_PLTFM_QSFP_CU_3_M = 3,
    BF_PLTFM_QSFP_CU_LOOP = 4,
    BF_PLTFM_QSFP_OPT = 5,
    BF_PLTFM_QSFP_UNKNOWN = 6
} nb_pltfm_qsfp_type_t;

/* following compliance codes are derived from SFF-8436 document */
typedef enum {
    COMPLIANCE_NONE = 0,
    ACTIVE_CABLE = 1 << 0,
    LR4_40GBASE = 1 << 1,
    SR4_40GBASE = 1 << 2,
    CR4_40GBASE = 1 << 3,
    SR_10GBASE = 1 << 4,
    LR_10GBASE = 1 << 5,
    LRM_10GBASE = 1 << 6,
    COMPLIANCE_RSVD = 1 << 7,
} Ethernet_compliance;

/* following complianbce codes are derived from SFF-8024 document */
typedef enum {
    EXT_COMPLIANCE_NONE = 0,
    AOC_100G_BER_5 = 0x01, /* 100G AOC or 25G AUI C2M AOC 5 * 10^^-5 BER */
    SR4_100GBASE = 0x02,   /* or SR-25GBASE */
    LR4_100GBASE = 0x03,   /* or LR-25GBASE */
    ER4_100GBASE = 0x04,   /* or ER-25GBASE */
    SR10_100GBASE = 0x05,
    CWDM4_100G = 0x06,
    PSM4_100G_SMF = 0x07,
    ACC_100G_BER_5 = 0x08, /* 100G ACC or 25G AUI C2M ACC 5 * 10^^-5 BER */
    // EXT_COMPLIANCE_OBSOLETE = 0x09,
    // EXT_COMPLIANCE_RSVD1 = 0x0A,
    CR4_100GBASE = 0x0B, /* or CR-25GBASE CA-L */
    CR_25GBASE_CA_S = 0x0C,
    CR_25GBASE_CA_N = 0x0D,
    EXT_COMPLIANCE_RSVD2 = 0x0E,
    EXT_COMPLIANCE_RSVD3 = 0x0F,
    ER4_40GBASE = 0x10,
    SR_10GBASE_4 = 0x11,
    PSM4_40G_SMF = 0x12,
    G959_P1I1_2D1 = 0x13, /* 10709 Mbd, 2 km, 1310nm SM */
    G959_P1S1_2D2 = 0x14, /* 10709 Mbd, 40 km, 1550nm SM */
    G959_P1L1_2D2 = 0x15, /* 10709 Mbd, 80 km, 1550nm SM */
    T_10BASE_SFI = 0x16,  /* 10BASE-T with SFI electrical interface */
    CLR4_100G = 0x17,
    AOC_100G_BER_12 = 0x18, /* 100G AOC or 25G AUI C2M AOC 10^^-12 BER */
    ACC_100G_BER_12 = 0x19, /* 100G ACC or 25G AUI C2M ACC 10^^-12 BER */
    DWDM2_100GE = 0x1A,     /* DMWM module using 1550nm, 80 km */
} Ethernet_extended_compliance;

// Media encoding type : Byte 85
typedef enum {
    MEDIA_TYPE_UNDEF = 0X00,
    MEDIA_TYPE_MMF = 0x01,
    MEDIA_TYPE_SMF = 0x02,
    MEDIA_TYPE_PASSIVE_CU = 0x03,
    MEDIA_TYPE_ACTIVE_CBL = 0x04,
} Media_type_enc_for_CMIS;

// one supported application in CMIS or SFF-8636. In the case of 8636, each
// field is entered as if that 8636 entry was advertised as a CMIS Application
typedef struct nb_pm_qsfp_apps_t {
    uint8_t host_if_id;   // host I/F ID from CMIS or 8636 equiv
    uint8_t media_if_id;  // media I/F ID from CMIS or 8636 equiv
    uint8_t host_lane_cnt;
    uint8_t media_lane_cnt;
    uint8_t host_lane_assign_mask;  // each set bit location corresponds to
    // supported starting lane for this application
    uint8_t media_lane_assign_mask;  // each set bit location corresponds to
                                     // supported starting lane for this application
} nb_pm_qsfp_apps_t;

typedef struct nb_qsfp_info_t {
    bool present;
    bool reset;
    bool soft_removed;
    bool flat_mem;
    bool passive_cu;
    uint8_t num_ch;
    MemMap_Format memmap_format;

    /* cached QSFP values */
    bool cache_dirty;
    bool suppress_repeated_rd_fail_msgs;
    uint8_t idprom[MAX_QSFP_PAGE_SIZE];
    uint8_t page0[MAX_QSFP_PAGE_SIZE];
    uint8_t page1[MAX_QSFP_PAGE_SIZE];
    uint8_t page2[MAX_QSFP_PAGE_SIZE];
    uint8_t page3[MAX_QSFP_PAGE_SIZE];
    //   uint8_t module_flags[MODULE_FLAG_BYTES];
    //   uint32_t module_flag_cnt[MODULE_FLAG_BYTES][8];
    //   uint8_t lane_flags[LANE_FLAG_BYTES];
    //   uint32_t lane_flag_cnt[LANE_FLAG_BYTES][8];

    uint8_t media_type;
    nb_pm_qsfp_apps_t *app_list;
    int num_apps;
} nb_qsfp_info_t;

typedef enum {
    /* Shared CMIS, QSFP and SFP fields: */
    IDENTIFIER, /* Type of Transceiver */
    SPEC_REV,
    STATUS, /* Support flags for upper pages */
    SFF8636_LANE_STATUS_FLAGS,
    SFF8636_LANE_MONITOR_FLAGS,
    MODULE_FLAGS,
    TEMPERATURE_ALARMS,
    VCC_ALARMS, /* Voltage */
    CHANNEL_RX_PWR_ALARMS,
    CHANNEL_TX_BIAS_ALARMS,
    CHANNEL_TX_PWR_ALARMS,
    TEMPERATURE,
    VCC, /* Voltage */
    CHANNEL_RX_PWR,
    CHANNEL_TX_BIAS,
    CHANNEL_TX_PWR,
    CHANNEL_TX_DISABLE,
    POWER_CONTROL,
    CDR_CONTROL, /* sff-8636 only */
    ETHERNET_COMPLIANCE,
    PWR_CLASS,
    PWR_REQUIREMENTS,
    PAGE_SELECT_BYTE,
    CONN_TYPE,
    LENGTH_SM_KM, /* Single mode, in km */
    LENGTH_SM,    /* Single mode in 100m (not in QSFP) */
    LENGTH_OM5,
    LENGTH_OM4,
    LENGTH_OM3,
    LENGTH_OM2,
    LENGTH_OM1,
    LENGTH_CBLASSY,
    VENDOR_NAME,                   /* QSFP Vendor Name (ASCII) */
    VENDOR_OUI,                    /* QSFP Vendor IEEE company ID */
    PART_NUMBER,                   /* Part NUmber provided by QSFP vendor (ASCII) */
    REVISION_NUMBER,               /* Revision number */
    ETHERNET_EXTENDED_COMPLIANCE,  /* ethernet extended compliance code */
    ETHERNET_SECONDARY_COMPLIANCE, /* ethernet secondary ext compliance code */
    VENDOR_SERIAL_NUMBER,          /* Vendor Serial Number (ASCII) */
    MFG_DATE,                      /* Manufacturing date code */
    TEMPERATURE_THRESH,
    VCC_THRESH,
    RX_PWR_THRESH,
    TX_BIAS_THRESH,
    TX_PWR_THRESH,
    OPTIONS,

    /* CMIS-specific fields */
    /* If these change, also update ApplicationAdversiting below */
    MODULE_STATE,
    FIRMWARE_VER_ACTIVE,
    MODULE_MEDIA_TYPE,
    APSEL1_ALL,
    APSEL1_HOST_ID,
    APSEL1_MEDIA_ID,
    APSEL1_LANE_COUNTS,
    APSEL1_HOST_LANE_OPTION_MASK,
    FIRMWARE_VER_INACTIVE,
    HARDWARE_VER,
    APSEL1_MEDIA_LANE_OPTION_MASK,  // this is one contiguous list
    APSEL9_ALL,
    APSEL9_HOST_ID,
    APSEL9_MEDIA_ID,
    APSEL9_LANE_COUNTS,
    APSEL9_HOST_LANE_OPTION_MASK,
    DATAPATH_DEINIT,
    APPLY_DATAPATHINIT_SS0,
    APPLY_IMMEDIATE_SS0,  // Netberg
    RX_EQ,                // Netberg
    DATAPATH_CFG_ALL_STAGED_SET0,
    DATAPATH_CFG_ALL_STAGED_SET1,
    DATAPATH_CFG_ALL_ACTIVE_SET,
    APPLICATION_SELECT_LN1,
    DATAPATH_STATE_ALL,
    DATAPATH_STATE_LN1AND2,
    CMIS_LANE_FLAGS,
    MEDIA_INT_TECH,
    DATAPATH_CFG_STATUS,
    FLAG_SUPPORT,
    LN1_ACTIVE_SET,

    /* SFP-specific Fields */
    /* 0xA0 Address Fields */
    EXT_IDENTIFIER,        /* Extended type of transceiver */
    CONNECTOR_TYPE,        /* Code for Connector Type */
    TRANSCEIVER_CODE,      /* Code for Electronic or optical capability */
    ENCODING_CODE,         /* High speed Serial encoding algo code */
    SIGNALLING_RATE,       /* nominal signalling rate */
    RATE_IDENTIFIER,       /* type of rate select functionality */
    TRANCEIVER_CAPABILITY, /* Code for Electronic or optical capability */
    WAVELENGTH,            /* laser wavelength */
    CHECK_CODE_BASEID,     /* Check code for the above fields */
    /* Extended options */
    ENABLED_OPTIONS,         /* Indicates the optional transceiver signals enabled */
    UPPER_BIT_RATE_MARGIN,   /* upper bit rate margin */
    LOWER_BIT_RATE_MARGIN,   /* lower but rate margin */
    ENHANCED_OPTIONS,        /* Enhanced options implemented */
    SFF_COMPLIANCE,          /* revision number of SFF compliance */
    CHECK_CODE_EXTENDED_OPT, /* check code for the extended options */
    VENDOR_EEPROM,

    /* 0xA2 address Fields */
    /* Diagnostics */
    ALARM_THRESHOLD_VALUES,  /* diagnostic flag alarm and warning thresh values */
    EXTERNAL_CALIBRATION,    /* diagnostic calibration constants */
    CHECK_CODE_DMI,          /* Check code for base Diagnostic Fields */
    DIAGNOSTICS,             /* Diagnostic Monitor Data */
    STATUS_CONTROL,          /* Optional Status and Control bits */
    ALARM_WARN_FLAGS,        /* Diagnostic alarm and warning flag */
    EXTENDED_STATUS_CONTROL, /* Extended status and control bytes */
    /* General Purpose */
    VENDOR_MEM_ADDRESS, /* Vendor Specific memory address */
    USER_EEPROM,        /* User Writable NVM */
    VENDOR_CONTROL,     /* Vendor Specific Control */

    SFF_FIELD_MAX /* keep this the last */
} Sff_field;

typedef struct {
    Qsfp_bank bank;
    Qsfp_page page;
    int offset;
    int length;
    bool in_cache;
} sff_field_info_t;

typedef struct {
    Sff_field field;
    uint32_t value;
} sff_field_mult_t;

typedef struct {
    Sff_field field;
    sff_field_info_t info;
} sff_field_map_t;

bool nb_qsfp_is_cmis(int port);
bool nb_qsfp_is_sff8636(int port);
/* return sff_field_info_t for a given field in sff8636_field_map[] */
static sff_field_info_t *get_sff_field_addr(const Sff_field field);
/* return sff_field_info_t for a given field in cmis_field_map[] */
static sff_field_info_t *get_cmis_field_addr(const Sff_field field);
/* return the contents of the  sff_field_info_t for a given field */
static int get_sff_field_info(int port,
                              Sff_field field,
                              sff_field_info_t **info);
static bool nb_qsfp_is_flat_mem(int port);
static void nb_qsfp_idprom_clr(int port);
static int nb_qsfp_init(void);
static bool is_qsfp_addr_in_cache(int port, sff_field_info_t *info);
/* do not call this directly, go through nb_qsfp_field_read_onebank */
static bool nb_qsfp_read_cache(int port,
                               sff_field_info_t *info,
                               uint8_t *data);
static int nb_qsfp_field_read_onebank(int port,
                                      Sff_field field,
                                      int chmask,
                                      int addl_offset,
                                      int max_length,
                                      uint8_t *data);
static int nb_cmis_spec_rev_get(int port, uint8_t *major, uint8_t *minor);
static int set_qsfp_idprom(int port);
static int cmis_get_media_type(int port, uint8_t *media_type);
static uint32_t get_qsfp_multiplier(Sff_field field);
static bool nb_qsfp_is_eth_ext_compliance_copper(uint8_t ext_comp);
static float get_qsfp_cable_length(int port, Sff_field fieldName);
static int nb_qsfp_get_eth_compliance(int port, Ethernet_compliance *code);
static int nb_qsfp_get_eth_ext_compliance(int port,
                                          Ethernet_extended_compliance *code);
static int nb_qsfp_type_get(int port, nb_pltfm_qsfp_type_t *qsfp_type);
static int nb_cmis_type_get(int port, nb_pltfm_qsfpdd_type_t *qsfp_type);
static bool nb_qsfp_is_passive_cu(int port);
static int cmis_get_application_data(int port,
                                     int apsel_code,
                                     uint8_t *host_id,
                                     uint8_t *media_id,
                                     uint8_t *host_lane_count,
                                     uint8_t *media_lane_count,
                                     uint8_t *host_lane_option_mask,
                                     uint8_t *media_lane_option_mask);
static int cmis_calc_application_count(int port, int *app_count);
// populate nb_qsfp_info_arr with the supported Application details
static int qsfp_cmis_populate_app_list(int port);
static int nb_qsfp_update_cache(int port);
static int nb_qsfp_detect_transceiver(int port);
#endif
