#include <sys/types.h>

#define NB_FPGA_RES_ADDR "/sys/bus/pci/devices/0000:66:00.0/resource0"	

#define NB_FPGA_ACCESS_TIME_COUNT 2000 //20*100ms = 2s
#define NB_FPGA_FW_DOWNLOAD_TIME_COUNT 60 //60*100ms = 6s 


#define NB_FPGA_ACCESS_DELAY usleep(1) 
//#define NB_FPGA_MEM_SIZE (12*1024) // FPGA BAR0 is 12K
//#define NB_FPGA_MEM_SIZE 0x120000    // FPGA BAR0 is 1152K
#define NB_FPGA_MEM_SIZE 0x40000
#define NB_FPGA_UTIL_VERSION_MAJOR 2
#define NB_FPGA_UTIL_VERSION_MINOR 05
#define NB_FPGA_CMD_DELAY usleep(10)

#define NB_FPGA_GBFW_SIZE 131072


// FPGA operation mode
#define NB_FPGA_OP_NONE 						0x0
#define NB_FPGA_OP_WRITE_MDIO 			0x101
#define NB_FPGA_OP_READ_MDIO				0x102

#define NB_FPGA_OP_WRITE_LED 				0x201
#define NB_FPGA_OP_READ_LED					0x202

#define NB_FPGA_OP_WRITE_MEDIA_BYTE			0x301
#define NB_FPGA_OP_READ_MEDIA_BYTE			0x302
//no use #define NB_FPGA_OP_WRITE_MEDIA_WORD 		0x303
#define NB_FPGA_OP_SET_I2C_MUX					0x304
#define NB_FPGA_OP_SET_QSFP_MUX         0x305

#define NB_FPGA_OP_WRITE_MEM32			0x401
#define NB_FPGA_OP_READ_MEM32				0x402


#define NB_FPGA_OP_WRITE_GEARBOX_FW	0x501
#define NB_FPGA_OP_READ_GEARBOX_FW	0x502
#define NB_FPGA_OP_PROG_GEARBOX_FW	0x503

#define NBA810_OP_I2C_WRITE  	0x601
#define NBA810_OP_I2C_READ  		0x602
#define NBA810_OP_LED_WRITE  	0x603
#define NBA810_OP_QSFP_READ  		0x604
#define NBA810_OP_QSFP_WRITE  	0x605
#define NBA810_OP_QSFP_initial	0x606
#define NBA810_OP_QSFP_Set_LPMode	0x607
#define NBA810_OP_QSFP_Set_Reset   	0x608
#define NBA810_OP_QSFP_Read_Present	0x609
#define NBA810_OP_QSFP_Read_RxLoss  	0x610
#define NBA810_OP_QSFP_Read_LPMode    0x611


// FPGA status
#define NB_FPGA_STATUS_DONE					0x0 
#define NB_FPGA_STATUS_BUSY					0x1 
#define NB_FPGA_STATUS_TIMEOUT			0x2

// FPGA control
#define NB_FPGA_MDIO_IDLE 				0x0
#define NB_FPGA_MDIO_WRITE 				0x1 
#define NB_FPGA_MDIO_READ 				0x2
#define NB_FPGA_MDIO_FW_DOWNLOAD	0x3
#define NB_FPGA_MDIO_CLEAR 				0xff

#define NB_FPGA_MEDIA_IDLE 					0x0
#define NB_FPGA_MEDIA_READ_BYTE 		0x1 
#define NB_FPGA_MEDIA_WRITE_BYTE 		0x2
//no use #define NB_FPGA_MEDIA_WRITE_WORD 		0x3
#define NB_FPGA_MEDIA_WRITE_CONTROL 0x4
#define NB_FPGA_MEDIA_CLEAR 				0xff

#define NB_FPGA_LED_IDLE 				0x0
#define NB_FPGA_LED_READ 				0x1 
#define NB_FPGA_LED_WRITE 			0x2
#define NB_FPGA_LED_CLEAR 			0xff

#define NB_FPGA_QSFP1_ADDR 				0x72
#define NB_FPGA_QSFP2_ADDR				0x73


// console font color
#define  HEADER     "\033[95m"
#define  OKBLUE     "\033[94m"
#define  OKGREEN    "\033[92m"
#define  WARNING    "\033[93m"
#define  FAIL       "\033[91m"
#define  ENDC       "\033[0m"
#define  BOLD       "\033[1m"
#define  UNDERLINE  "\033[4m"



//#define EN_DEBUG_MSG
#if defined (EN_DEBUG_MSG)
	#define nb_fpga_debug(fmt, args...)  printf(ENDC   "INFO:  [%s] " fmt "\n" ENDC,__FUNCTION__,##args)
#else
	#define nb_fpga_debug(fmt, args...)        while(0)
#endif

#define nb_fpga_print(fmt, args...) printf(OKGREEN "MSG:   [%s] " fmt  ENDC "\n",__FUNCTION__,##args); 
#define nb_fpga_err(fmt, args...)   printf(FAIL   "ERROR: [%s] " fmt  ENDC "\n",__FUNCTION__,##args); 
// FPGA address 
#define NB_FPGA_MDIO_START_ADDR  0x0100
#define NB_FPGA_LED_START_ADDR 	 0x1100
#define NB_FPGA_MEDIA_START_ADDR 0x2100
#define NB_FPGA_FORMAT_SZIE      0x100
#define NBA810_LED_START_ADDR 0x00000A00

typedef struct {					/* total size :256 */
    u_int8_t control;		/* addr:	0 */
    u_int8_t slice;			/* addr:	1 */
    u_int8_t slot;			/* addr:	2 */
    u_int8_t rsd_1;			/* addr:	3 */
    u_int16_t reg_addr;		/* addr:	4~5 */
		u_int16_t rsd_2;			/* addr:	6~7 */
    u_int16_t status;			/* addr:	8~9 */
    u_int16_t rsd_3;			/* addr:	10~11 */
    u_int16_t w_data;			/* addr:	12~13 */
    u_int16_t rsd_4;			/* addr:	14~15 */
    u_int16_t r_data;			/* addr:	16~17 */
    u_int16_t rsd_5;			/* addr:	18~19 */
    u_int8_t rsd_6[236];	/* addr:	20~255 */
} nb_fpga_mdio_format_t;

typedef struct {					/* total size :256 */
    u_int16_t control;		/* addr:	0~1 */
    u_int16_t i2c_addr;		/* addr:	2~3 */
    u_int16_t reg_addr;		/* addr:	4~5 */
		u_int16_t rsd_1;			/* addr:	6~7 */
    u_int16_t status;			/* addr:	8~9 */
    u_int16_t rsd_2;			/* addr:	10~11 */
    u_int16_t w_data;			/* addr:	12~13 */
    u_int16_t rsd_3;			/* addr:	14~15 */
    u_int16_t r_data;			/* addr:	16~17 */
    u_int16_t rsd_4;			/* addr:	18~19 */
    u_int8_t rsd_5[236];	/* addr:	20~255 */
} nb_fpga_led_i2c_format_t;

typedef struct {					/* total size :256 */
    u_int16_t control;		/* addr:	0~1 */
    u_int16_t i2c_addr;		/* addr:	2~3 */
    u_int16_t reg_addr;		/* addr:	4~5 */
		u_int16_t rsd_1;			/* addr:	6~7 */
    u_int16_t status;			/* addr:	8~9 */
    u_int16_t rsd_2;			/* addr:	10~11 */
    u_int16_t w_data;			/* addr:	12~13 */
    u_int16_t rsd_3;			/* addr:	14~15 */
    u_int16_t r_data;			/* addr:	16~17 */
    u_int16_t rsd_4;			/* addr:	18~19 */
    u_int8_t rsd_5[236];	/* addr:	20~255 */
} nb_fpga_media_i2c_format_t;

typedef struct {					/* total size :256 */
    u_int16_t control;		/* addr:	0~1 */
    u_int16_t i2c_addr;		/* addr:	2~3 */
    u_int16_t reg_addr;		/* addr:	4~5 */
	u_int16_t rsd_1;			/* addr:	6~7 */
    u_int16_t status;			/* addr:	8~9 */
    u_int16_t rsd_2;			/* addr:	10~11 */
    u_int16_t w_data;			/* addr:	12~13 */
    u_int16_t rsd_3;			/* addr:	14~15 */
    u_int16_t r_data;			/* addr:	16~17 */
    u_int16_t rsd_4;			/* addr:	18~19 */
    u_int8_t rsd_5[236];	/* addr:	20~255 */    
} nb_fpga_feature_card_i2c_format_t;

typedef struct {
    u_int16_t info[128];	
} nb_fpga_info;

typedef struct {
    u_int8_t rsd[256];	
} nb_fpga_reserve_256;

//========================NBA810 Jim Add 20220401======================================================
typedef struct {				/* total size :256 */
    u_int16_t control;			/* addr:	0~1 */
    u_int16_t i2c_addr;			/* addr:	2~3 */
    u_int16_t reg_addr;			/* addr:	4~5 */
    u_int16_t rsd_1;		/* addr:	6~7 */
    u_int16_t status;			/* addr:	8~9 */
    u_int16_t rsd_2;			/* addr:	10~11 */
    u_int16_t w_data;			/* addr:	12~13 */
    u_int16_t rsd_3;			/* addr:	14~15 */
    u_int16_t r_data;			/* addr:	16~17 */
    u_int16_t rsd_4;			/* addr:	18~19 */
    u_int8_t rsd_5[236];		/* addr:	20~255 */
} nba810_i2c_format_t;

typedef struct {				/* total size :256 */
    u_int16_t control;			/* addr:	0~1 */
    u_int16_t rsd_1[3];			/* addr:	2~7 */
    u_int16_t status;			/* addr:	8~9 */
    u_int16_t rsd_2;			/* addr:	10~11 */
    u_int8_t  LED[32];			/* addr:	12~43	*/
    u_int8_t rsd_3[212];	    /* addr:	44~255 */
} nba810_led_format_t;

//======================================================================================================



typedef struct nb_fpga_data {
	nb_fpga_info fpga_info;									/* addr:	0x00~0xff */	
	nb_fpga_mdio_format_t mdio_data;						/* addr:	0x100~0x1ff */	
//=====================Jim add 20220401==================================================================
	nba810_i2c_format_t nba810_i2c_data[8];				/* addr:	0x200~0x9ff */	
	nba810_led_format_t nba810_led_dara;					/* addr:	0xa00~0xaff */
//=========================================================================================================
	nb_fpga_reserve_256 rsd_zone_1[5];						/* addr:	0xb00~0xfff */	
	nb_fpga_feature_card_i2c_format_t feature_card_i2c_0;	/* addr:	0x1000~0x10ff */	
	nb_fpga_led_i2c_format_t led_i2c_data[8];				/* addr:	0x1100~0x18ff */		
	nb_fpga_reserve_256 rsd_zone_2[7];						/* addr:	0x1900~0x1fff */
	nb_fpga_feature_card_i2c_format_t feature_card_i2c_1;	/* addr:	0x2000~0x20ff */
	nb_fpga_media_i2c_format_t media_i2c_data[8];			/* addr:	0x2100~0x28ff */
	nb_fpga_reserve_256 rsd_zone_3[7];						/* addr:	0x2900~0x2fff */  
	nb_fpga_reserve_256 unuse[464];                       	/* addr:	0x3000~0x1ffff */
	u_int8_t gearbox_fw[NB_FPGA_GBFW_SIZE];					/* addr:	0x20000~0x3ffff */
} nb_fpga_regs_t;




extern nb_fpga_regs_t *nb_fpga_mmap_addr;

int nb_fpga_init();
int nb_fpga_exit();
int nb_fpga_write_mdio(u_int8_t slot_mask, u_int8_t slice, u_int16_t reg_addr, u_int16_t data);
int nb_fpga_read_mdio(u_int8_t slot_mask, u_int8_t slice, u_int16_t reg_addr, u_int16_t *data);
int nb_fpga_write_media_byte(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data);
//no use int nb_fpga_write_media_word(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data);
int nb_fpga_read_media_byte(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data);
int nb_fpga_set_i2c_mux(int slot, u_int16_t i2c_addr, u_int16_t i2c_channel);
//int nb_fpga_set_i2c_mux16(int slot, u_int16_t i2c_addr, u_int16_t i2c_channel);
int nb_fpga_write_led(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data);
int nb_fpga_read_led(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data);
int nb_fpga_write_mem32(int offset, u_int32_t data);
int nb_fpga_read_mem32(int offset);
int nb_firmware_write(const char *filename);
int nb_firmware_read(const char *filename);
int nb_firmware_prog(u_int8_t slot_mask);
int nb_fpga_nba810_read_i2c(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data);
int nba810_led_status_check(void);
int nba810_i2c_status_check(int slot);
int nb_fpga_nba810_write_i2c(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data);
int nb_fpga_nba810_led(int led_port, u_int8_t data);
int nb_fpga_nba810_QSFP_Read(int QSFP_channel,u_int16_t Reg_addr,u_int16_t **R_data);
int nb_fpga_nba810_QSFP_Write(int QSFP_channel,u_int16_t Reg_addr,u_int16_t w_data);

