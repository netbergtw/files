#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <string.h>

#include <libgen.h>



#include "nb_fpga.h"

#define  SINGLE_INTERFACE 

#define  HEADER     "\033[95m"

#define  OKBLUE     "\033[94m"

#define  OKGREEN    "\033[92m"

#define  WARNING    "\033[93m"

#define  FAIL       "\033[91m"

#define  ENDC       "\033[0m"

#define  BOLD       "\033[1m"

#define  UNDERLINE  "\033[4m"



#if defined (EN_DEBUG_MSG)

 #define DEBUG_MSG(fmt, args...)                     \

        printf("==== [%s][%s][%d]:" fmt "\n",__FILE__,__FUNCTION__,__LINE__,##args)

#else

 #define DEBUG_MSG(fmt, args...)        while(0)



#endif



nb_fpga_regs_t *nb_fpga_mmap_addr;



void usage(char *prgname)

{



    printf (HEADER"Netberg FPGA Utility ver:%1d.%.2d build: %s %s \n"ENDC, NB_FPGA_UTIL_VERSION_MAJOR , 	\

    				NB_FPGA_UTIL_VERSION_MINOR, __DATE__, __TIME__);	

    if(	getenv("nb_fpga_dev")	== NULL)

    	printf (HEADER"export nb_fpga_dev at first time\n"ENDC);

    else

    	printf(HEADER"NB_FPGA_RES_DEV: %s\n"ENDC, getenv("nb_fpga_dev"));

        

    printf (BOLD);
/*
    printf ("================Angel eyes command========================\n");

    printf ("Usage: %s \n",basename(prgname));

    printf ("         <slot>       Slot Number.  (1 ~ 8) \n" );

    printf ("         <slice>      Slice Number. (0 ~ 7) \n" );

	printf ("         <i2caddr>    Base on HW board \n" );

    printf ("\n" );

    printf ("  wmdio      <slot_mask> <slice> <regaddr> <data> \n" );

    printf ("  rmdio      <slot_mask> <slice> <regaddr> \n" );

    printf ("  wmedia     <slot> <i2caddr> <regaddr> <data_8bit> \n" );

    printf ("  rmedia     <slot> <i2caddr> <regaddr> \n" );

    printf ("  i2cmux     <slot> <i2caddr> <channel 1~8 ## 0 disable all port> \n" );

    printf ("  qsfpmux    <slot> <channel 1~16 ## 0 disable all port> \n" );

    printf ("  pgbfw      <slot_mask> ## start program gearbox FW\n" );

    printf ("=================================================\n"   );

    printf ("  wled       <slot> <i2caddr> <regaddr> <data> \n" );

    printf ("  rled       <slot> <i2caddr> <regaddr> \n" );

    printf ("  wgbfw      <filename>  ## write gearbox FW to RAM\n" );

    printf ("  rgbfw      <filename>  ## read gearbox FW from RAM\n" );

    printf ("  wmem32     <mdio/media/led> <slot> <offset> <data>\n" );

    printf ("  rmem32     <mdio/media/led> <slot> <offset> \n" );
    
    printf ("  wmem32 <offset> <data>\n" );

    printf ("  rmem32 <offset> \n" );
*/
//==================================jim add 20220401====================================
    	printf ("=================NBA810 command=========================\n" );
    	
    	printf ("  wmem32 <offset> <data>\n" );

    	printf ("  rmem32 <offset> \n" );
    	
	printf ("  i2c_w   <i2c port> <i2caddr> <regaddr> <data_8bit> \n" );

	printf ("  i2c_r   <i2c port> <i2caddr> <regaddr>\n" );

	printf ("  led_w   <led group> <led_data_32bit>\n" );

	printf ("  QSFP_r  <QSFP port> <regaddr> \n" );

	printf ("  QSFP_w  <QSFP port> <regaddr> <data_16bit>\n" );
	
	printf ("  QSFP_init   ## QSFP I/O initial \n" );
	
	printf ("  SQLPM <data_32bit>  ## Set QSFP Channel Low Power Mode\n");
	
	printf ("  RQLPM 	       ## Read QSFP Channel Low Power Mode\n");
	
	printf ("  SQRST <data_32bit>  ## Set QSFP  channel Reset\n");
	
	printf ("  RQPST  ## Read QSFP Present\n");
	
	printf ("  RQRL   ## Read QSFP Rx Loss\n");

	printf ("********************i2c Description****************************\n" );
	
	printf (" <i2c port>   i2c port Number.  (1 ~ 8) \n" );     

	printf (" <i2caddr>    Base on HW board \n" );

	printf ("********************LED Description****************************\n" );

	printf (" <led group>  LED group X , X=1~8 \n" );
	
	printf (" <led_data_32bit> :{LED(x+3)_RBG,LED(x+2)_RBG,LED(x+1)_RBG,LEDx_RBG}  \n" );
	
	printf (" RBG=0:None,1:Green,2:Blue,3:Cyan-blue \n" );
	
	printf (" RBG=4:Red,5:Yellow,6:Purple,7:White\n" );

	printf ("\n" );

	

    printf (ENDC);

//=======================================================================================

}

//						  					  |--------------------------set LPMode-----------------------|---------------------------set Reset-----------------------|----------------set Present------------|-------------set Rx loss---------------|
int nba810_inital_i2c_port[40]				= {0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08};
u_int16_t nba810_inital_i2c_addr[40]		= {0x20,0x20,0x21,0x21,0x20,0x20,0x21,0x21,0x20,0x20,0x21,0x21,0x26,0x26,0x27,0x27,0x26,0x26,0x27,0x27,0x26,0x26,0x27,0x27,0x22,0x22,0x23,0x23,0x22,0x22,0x23,0x23,0x24,0x24,0x25,0x25,0x24,0x24,0x25,0x25};
u_int16_t nba810_inital_i2c_regaddr[40]	= {0x06,0x07,0x06,0x07,0x02,0x03,0x02,0x03,0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,0x02,0x03,0x02,0x03,0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,0x04,0x05,0x04,0x05};
u_int16_t nba810_initial_i2c_data[40]		= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
int nba810_initial_err_count;


u_int16_t _ori_present_8to1   = 0;
u_int16_t _ori_present_16to9  = 0;
u_int16_t _ori_present_24to17 = 0;
u_int16_t _ori_present_32to25 = 0;
char *pst_tmp_file = "/tmp/nb_fpga_pst.tmp";


int _set_orignal_present(void) {
    FILE *fp_w;

    fp_w = fopen(pst_tmp_file,"w");
    if (fp_w == NULL) {
		return 0;
	} else {
		fprintf(fp_w, "0x%02x%02x%02x%02x\n", _ori_present_8to1,
                                              _ori_present_16to9,
                                              _ori_present_24to17,
                                              _ori_present_32to25);
        fclose(fp_w);
		return 1;
	}
}


int _get_orignal_present(void) {
    FILE *fp_r;
	char buf[128];
	int tmp;

    fp_r = fopen(pst_tmp_file,"r");
    if (fp_r == NULL) {
		// Case-1: temp file not exist
		return _set_orignal_present();
	} else {
		// Case-2: temp file existed
		if ((fgets(buf, sizeof(buf), fp_r)) == NULL) {
			// Case-2.1: file is empty
            tmp = _set_orignal_present();
			fclose(fp_r);
			return tmp;
		}
        tmp = (int)strtol(buf, NULL, 16);
		if ((tmp < 0) || (tmp > 0xffffffff)) {
			// Case-2.2: Abnormal value
            tmp = _set_orignal_present();
			fclose(fp_r);
			return tmp;
		}
		// Case-2.3: Normal value
		_ori_present_8to1   = (u_int16_t)((tmp >> 24) & 0xff);
		_ori_present_16to9  = (u_int16_t)((tmp >> 16) & 0xff);
		_ori_present_24to17 = (u_int16_t)((tmp >> 8) & 0xff);
		_ori_present_32to25 = (u_int16_t)(tmp & 0xff);
	}
	fclose(fp_r);
	return 1;
}





int main(int argc, char **argv) {



int failreturn = 0;

int slot = 1;

int i=0;

int ret = 0;

int ret1=0;

int ret2=0;


int ret3=0;

int ret4=0;

int offset = 0;

int op_mode = NB_FPGA_OP_NONE;

int i2c_port = 0;

int led_port = 0;

u_int16_t slice = 0, regaddr= 0, i2caddr = 0;

u_int16_t i2c_channel = 0;

u_int16_t data = 0;

u_int32_t data32 = 0;

u_int32_t data32_shiftL_one=0;

u_int32_t data32_shiftR_one=0;

u_int32_t data_swap =0;

u_int32_t  leddata = 0;


u_int16_t	QSFP_data_8to1=0;	
u_int16_t	QSFP_data_16to9=0;
u_int16_t	QSFP_data_24to17=0;
u_int16_t	QSFP_data_32to25=0;

u_int16_t	QSFP_data_16to9_bitswap=0;
u_int16_t	QSFP_data_24to17_bitswap=0;
u_int16_t	QSFP_data_32to25_bitswap=0;


int QSFP_channel = 0;

int QSFP_mem_select = 0;

u_int16_t *QSFP_R_data;


int NBA810_addr = 0;




	

if	(argc > 1)	{	   /* avoid Segmentation fault when argc <=1*/

    if ((strcmp(argv[1],"wmdio") == 0 ) && (argc == 6 )) {

        slot    = strtol(argv[2],NULL,0);

        slice   = strtol(argv[3],NULL,0);

        regaddr = strtol(argv[4],NULL,0);

        data    = strtol(argv[5],NULL,0);

        op_mode	=	NB_FPGA_OP_WRITE_MDIO;

    }

    else if ((strcmp(argv[1],"rmdio") == 0 ) && (argc == 5 )) {

        slot     = strtol(argv[2],NULL,0);

        slice    = strtol(argv[3],NULL,0);

        regaddr  = strtol(argv[4],NULL,0);

        op_mode	=	NB_FPGA_OP_READ_MDIO;

    }

    else if ((strcmp(argv[1],"wmedia") == 0 ) && (argc == 6 )) {

        slot    = strtol(argv[2],NULL,0);

        i2caddr = strtol(argv[3],NULL,0);

        regaddr = strtol(argv[4],NULL,0);

        data    = strtol(argv[5],NULL,0);

        op_mode	=	NB_FPGA_OP_WRITE_MEDIA_BYTE;

    }

    else if ((strcmp(argv[1],"rmedia") == 0 ) && (argc == 5 )) {

        slot     = strtol(argv[2],NULL,0);

        i2caddr  = strtol(argv[3],NULL,0);

        regaddr  = strtol(argv[4],NULL,0);

        op_mode	 =	NB_FPGA_OP_READ_MEDIA_BYTE;

    }

    else if ((strcmp(argv[1],"i2cmux") == 0 ) && (argc == 5 )) {

        slot    = strtol(argv[2],NULL,0);

        i2caddr = strtol(argv[3],NULL,0);

        i2c_channel  = strtol(argv[4],NULL,0);

        op_mode	=	NB_FPGA_OP_SET_I2C_MUX;

        if ((i2c_channel < 0) || (i2c_channel > 8 )) {

        printf(FAIL BOLD "i2cmux channel (%d) out of range. \n" ENDC,i2c_channel);

        failreturn = 1 ;

    		}

    }

    else if ((strcmp(argv[1],"qsfpmux") == 0 ) && (argc == 4 )) {

        slot    = strtol(argv[2],NULL,0);

        i2c_channel  = strtol(argv[3],NULL,0);

        op_mode	=	NB_FPGA_OP_SET_QSFP_MUX;

    		if ((i2c_channel < 0) || (i2c_channel > 16 )) {

        printf(FAIL BOLD "qsfpmux channel (%d) out of range. \n" ENDC,i2c_channel);

        failreturn = 1 ;

    		}

    }  

    else if ((strcmp(argv[1],"wled") == 0 ) && (argc == 6 )) {

        slot    = strtol(argv[2],NULL,0);

        i2caddr = strtol(argv[3],NULL,0);

        regaddr = strtol(argv[4],NULL,0);

        data    = strtol(argv[5],NULL,0);

        op_mode	=	NB_FPGA_OP_WRITE_LED;

    }

    else if ((strcmp(argv[1],"rled") == 0 ) && (argc == 5 )) {

        slot     = strtol(argv[2],NULL,0);

        i2caddr  = strtol(argv[3],NULL,0);

        regaddr  = strtol(argv[4],NULL,0);

        op_mode	 = NB_FPGA_OP_READ_LED;

    }

    else if ((strcmp(argv[1],"wmem32") == 0 ) && (argc == 6 )) {

        slot    = strtol(argv[3],NULL,0);

        data32  = strtol(argv[5],NULL,0);

        op_mode	=	NB_FPGA_OP_WRITE_MEM32;

    		

    		if (strcmp(argv[2],"mdio") == 0 ) {

					offset = NB_FPGA_MDIO_START_ADDR + strtol(argv[4],NULL,0);



    		}	

    		else if (strcmp(argv[2],"led") == 0 ) {

    			offset = NB_FPGA_LED_START_ADDR + ((slot-1) * NB_FPGA_FORMAT_SZIE) + strtol(argv[4],NULL,0);

    		}

    		else if (strcmp(argv[2],"media") == 0 ) {

    			offset = NB_FPGA_MEDIA_START_ADDR + ((slot-1) * NB_FPGA_FORMAT_SZIE) + strtol(argv[4],NULL,0);

    		}

    		else	{

    			printf (FAIL BOLD "Unknow parameter with write mem32\n" ENDC);

    			failreturn = 1;	

    		}	

    			

    }

    else if ((strcmp(argv[1],"rmem32") == 0 ) && (argc == 5 )) {

        slot    = strtol(argv[3],NULL,0);

        op_mode	 = NB_FPGA_OP_READ_MEM32;

    		if (strcmp(argv[2],"mdio") == 0 ) {

					offset = NB_FPGA_MDIO_START_ADDR + strtol(argv[4],NULL,0);



    		}	

    		else if (strcmp(argv[2],"led") == 0 ) {

    			offset = NB_FPGA_LED_START_ADDR + ((slot-1) * NB_FPGA_FORMAT_SZIE) + strtol(argv[4],NULL,0);

    		}

    		else if (strcmp(argv[2],"media") == 0 ) {

    			offset = NB_FPGA_MEDIA_START_ADDR + ((slot-1) * NB_FPGA_FORMAT_SZIE) + strtol(argv[4],NULL,0);

    		}

    		else	{

    			printf (FAIL BOLD "Unknow parameter with read mem32\n" ENDC);

    			failreturn = 1;	

    		}      

    }

    else if ((strcmp(argv[1],"rmem32") == 0 ) &&  (argc == 3 )) {

        op_mode	 = NB_FPGA_OP_READ_MEM32;

        slot = 1;

        offset = strtol(argv[2],NULL,0);

    }

    else if ((strcmp(argv[1],"wmem32") == 0 ) &&  (argc == 4 )) {

        op_mode	 = NB_FPGA_OP_WRITE_MEM32;

        data32  = strtol(argv[3],NULL,0);

        slot = 1;

        offset = strtol(argv[2],NULL,0);

    }

    

    else if ((strcmp(argv[1],"wgbfw") == 0 ) && (argc == 3 )) {

        op_mode	 = NB_FPGA_OP_WRITE_GEARBOX_FW;

        slot = 1;

    }  

    else if ((strcmp(argv[1],"rgbfw") == 0 ) && (argc == 3 )) {

        op_mode	 = NB_FPGA_OP_READ_GEARBOX_FW;

        slot = 1;

    } 

    else if ((strcmp(argv[1],"pgbfw") == 0 ) && (argc == 3 )) {

        op_mode	 = NB_FPGA_OP_PROG_GEARBOX_FW;

        slot = strtol(argv[2],NULL,0);

    }



//============================jim add 20220401==================================================================

	else if((strcmp(argv[1],"i2c_w") == 0 ) && (argc == 6 )) {

	i2c_port    	= strtol(argv[2],NULL,0);

        i2caddr 	= strtol(argv[3],NULL,0);

        regaddr 	= strtol(argv[4],NULL,0);

        data    	= strtol(argv[5],NULL,0);

        op_mode		= NBA810_OP_I2C_WRITE;

	}

	else if((strcmp(argv[1],"i2c_r") == 0 ) && (argc == 5 )) {

	i2c_port    	= strtol(argv[2],NULL,0);

        i2caddr 	= strtol(argv[3],NULL,0);

        regaddr 	= strtol(argv[4],NULL,0);

        op_mode		= NBA810_OP_I2C_READ;

	}

	else if((strcmp(argv[1],"led_w") == 0 ) && (argc == 4 )) {

	led_port    	= strtol(argv[2],NULL,0);

	leddata	= strtol(argv[3],NULL,0);

        op_mode	= NBA810_OP_LED_WRITE;

	}
	else if((strcmp(argv[1],"QSFP_r") == 0 ) && (argc == 4)) {

	QSFP_channel  	= strtol(argv[2],NULL,0);

	regaddr  	= strtol(argv[3],NULL,0);

        op_mode	= NBA810_OP_QSFP_READ;

	}
	else if((strcmp(argv[1],"QSFP_w") == 0 ) && (argc == 5)) {

	QSFP_channel  	= strtol(argv[2],NULL,0);

	regaddr  	= strtol(argv[3],NULL,0);

	data		= strtol(argv[4],NULL,0);

        op_mode	= NBA810_OP_QSFP_WRITE;

	}
	else if((strcmp(argv[1],"QSFP_init") == 0 ) && (argc == 2)) {

        op_mode	= NBA810_OP_QSFP_initial;
	}
	else if((strcmp(argv[1],"SQLPM") == 0 ) && (argc == 3)) {
	data32		= strtol(argv[2],NULL,0);
        op_mode	= NBA810_OP_QSFP_Set_LPMode;
	}
	else if((strcmp(argv[1],"RQLPM") == 0 ) && (argc == 2)) {
        op_mode	= NBA810_OP_QSFP_Read_LPMode;
	}
	else if((strcmp(argv[1],"SQRST") == 0 ) && (argc == 3)) {
	data32		= strtol(argv[2],NULL,0);
        op_mode	= NBA810_OP_QSFP_Set_Reset;
	}
	else if((strcmp(argv[1],"RQPST") == 0 ) && (argc == 2)) {
        op_mode	= NBA810_OP_QSFP_Read_Present;
	}
	else if((strcmp(argv[1],"RQRL") == 0 ) && (argc == 2)) {
        op_mode	= NBA810_OP_QSFP_Read_RxLoss;
	}


//==============================================================================================================

    else {

    		printf (FAIL BOLD "Unknow command (%s) or missing some parameter. \n" ENDC,argv[1]);

				failreturn = 1;

  	}

  } //if(argc > 1)

    

    // check command format

    // slot 1~8 

    if (op_mode != NB_FPGA_OP_READ_MDIO && op_mode != NB_FPGA_OP_WRITE_MDIO && op_mode != NB_FPGA_OP_PROG_GEARBOX_FW)

    {

    	if ((slot < 1) || (slot > 8 )) {

        	printf(FAIL BOLD "Slot number (%d) out of range. \n" ENDC,slot);

        	failreturn = 1 ;

    	}

    	else	{

  				slot--;	//slot 1~8

  		}	

    }

    // slice 0~7

    if ((slice < 0 ) || (slice > 7 )) {

        printf (FAIL BOLD "Slice number (%d) out of range. \n" ENDC,slice);

        failreturn = 1;

    }		

    

    

        

    if ( failreturn ) {

        usage(argv[0]);

        exit(3);

    }

    	

		// get FPGA reource

      ret = nb_fpga_init();

		if (ret) {

        printf(FAIL BOLD"FPGA init failed\n" ENDC);

				exit(1);

		}

		     

    

    // doing command

		switch (op_mode) {

		case NB_FPGA_OP_WRITE_MDIO:

			ret = nb_fpga_write_mdio(slot, slice, regaddr, data);

			if (ret == 0) {

      		nb_fpga_debug("write_mdio: success");

      }

      else {

      		nb_fpga_err("write_mdio: fail");

    	}

			break;

			

		case NB_FPGA_OP_READ_MDIO:

			ret = nb_fpga_read_mdio(slot, slice, regaddr, (u_int16_t *)&data);

			if (ret == 0) {

      		nb_fpga_print("mdio_data: 0x%x", nb_fpga_mmap_addr->mdio_data.r_data);

      }

      else {

      		nb_fpga_err("read_mdio: fail");

    	}		

			break;

			

		case NB_FPGA_OP_WRITE_LED:

			ret = nb_fpga_write_led(slot, i2caddr, regaddr, data);

			if (ret == 0) {

      		nb_fpga_debug("write_led: success");

      }

      else {

      		nb_fpga_err("write_led: fail");

    	}

			break;

			

		case NB_FPGA_OP_READ_LED:

			ret = nb_fpga_read_led(slot, i2caddr, regaddr, (u_int16_t *)&data);

			if (ret == 0) {

      		nb_fpga_print("led_data: 0x%x", nb_fpga_mmap_addr->led_i2c_data[slot].r_data);

      }

      else {

      		nb_fpga_err("read_media: fail");

    	}

			break;

				

		case NB_FPGA_OP_WRITE_MEDIA_BYTE:

			ret = nb_fpga_write_media_byte(slot, i2caddr, regaddr, data);

			if (ret == 0) {

      		nb_fpga_debug("write_media_byte: success");

      }

      else {

      		nb_fpga_err("write_media_byte: fail");

    	}

			break;

			

		case NB_FPGA_OP_READ_MEDIA_BYTE:

			ret = nb_fpga_read_media_byte(slot, i2caddr, regaddr, (u_int16_t *)&data);

			if (ret == 0) {

      		nb_fpga_print("media_data: 0x%x", nb_fpga_mmap_addr->media_i2c_data[slot].r_data &0xff);

      }

      else {

      		nb_fpga_err("read_media: fail");

    	}		

			break;

		

		case NB_FPGA_OP_SET_I2C_MUX:

			ret = nb_fpga_set_i2c_mux(slot, i2caddr, i2c_channel);

			if (ret == 0) {

      		nb_fpga_debug("write_media_word: success");

      }

      else {

      		nb_fpga_err("write_media_word: fail");

    	}

			break;	

		

		case NB_FPGA_OP_SET_QSFP_MUX:

			

			if	(i2c_channel <= 8) {

				ret = nb_fpga_set_i2c_mux(slot, NB_FPGA_QSFP2_ADDR, 0); //close all channel with another MUX

				if (ret == 0) {

					ret = nb_fpga_set_i2c_mux(slot, NB_FPGA_QSFP1_ADDR, i2c_channel);

				}

			}

			else {

				i2c_channel = i2c_channel -8;

				ret = nb_fpga_set_i2c_mux(slot, NB_FPGA_QSFP1_ADDR, 0); //close all channel with another MUX

				if (ret == 0) {

					ret = nb_fpga_set_i2c_mux(slot, NB_FPGA_QSFP2_ADDR, i2c_channel);		

				}

      }

			

			

			if (ret == 0) {

      		nb_fpga_debug("write_media_word: success");

      }

      else {

      		nb_fpga_err("write_media_word: fail");

    	}

			break;	

		

		

		case NB_FPGA_OP_WRITE_MEM32:		 

		  ret = nb_fpga_write_mem32(offset, data32);

			if (ret == 0) {

      		nb_fpga_debug("write_mem32: success");

      }

      else {

      		nb_fpga_err("write_mem32: fail");

    	}

			break;

		

		case NB_FPGA_OP_READ_MEM32:

			ret = nb_fpga_read_mem32(offset);

			if (ret == 0) {
#ifndef SINGLE_INTERFACE
      		nb_fpga_debug("read_mem32: success");
#endif
      }

      else {
#ifndef SINGLE_INTERFACE
      		nb_fpga_err("read_mem32: fail");
#else
			printf("Fail\n");
#endif
    	}



			break;

			

		case NB_FPGA_OP_WRITE_GEARBOX_FW:

			ret = nb_firmware_write(argv[2]);

			if (ret == 0) {

      		nb_fpga_debug("write gearbox fw to RAM: success");

      }

      else {

      		nb_fpga_err("write gearbox fw to RAM: fail");

    	}



			break;

			

		case NB_FPGA_OP_READ_GEARBOX_FW:

			ret = nb_firmware_read(argv[2]);

			if (ret == 0) {

      		nb_fpga_debug("read gearbox fw to RAM: success");

      }

      else {

      		nb_fpga_err("read gearbox fw to RAM: fail");

    	}



			break;

		

		case NB_FPGA_OP_PROG_GEARBOX_FW:

			if ((nb_fpga_mmap_addr->gearbox_fw[0] == 0) && (nb_fpga_mmap_addr->gearbox_fw[1] == 0) \

					&& (nb_fpga_mmap_addr->gearbox_fw[2] == 0) && (nb_fpga_mmap_addr->gearbox_fw[3] == 0)) {

    		nb_fpga_err("Please wgbfw before pgbfw\n");

    		break;

    	}		

			ret = nb_firmware_prog(slot);

			if (ret == 0) {

      		nb_fpga_debug("program gearbox fw: success");

      }

      else {

      		nb_fpga_err("program gearbox fw: fail");

    	}



			break;

//=========================================Jim  add 20220401=======================================================

		case NBA810_OP_I2C_WRITE:

			ret = nb_fpga_nba810_write_i2c(i2c_port,i2caddr,regaddr,data);

			if (ret == 0) {
#ifndef SINGLE_INTERFACE
      		nb_fpga_debug("nba810_i2c_write: success");
#else
			printf("Success\n");
#endif

			}

			else {
#ifndef SINGLE_INTERFACE
			nb_fpga_err("nba810_i2c_write: fail");
#else		
			printf("Fail\n");
#endif

			}

			break;

		case NBA810_OP_I2C_READ:

			ret = nb_fpga_nba810_read_i2c(i2c_port,i2caddr,regaddr,(u_int16_t *)&data);

			if (ret == 0) {
#ifndef SINGLE_INTERFACE
      		nb_fpga_print("led_data: 0x%x", nb_fpga_mmap_addr->nba810_i2c_data[i2c_port-1].r_data);
#else
			printf("Success:0x%x\n",nb_fpga_mmap_addr->nba810_i2c_data[i2c_port-1].r_data);
#endif
			}

			else {
#ifndef SINGLE_INTERFACE
			nb_fpga_err("nba810_i2c_read: fail");
#else
			printf("Fail\n");
#endif
			}

			break;

		case NBA810_OP_LED_WRITE:

			NBA810_addr = 0x00000a0c + ((led_port-1)*4);

			ret = nb_fpga_write_mem32(NBA810_addr, leddata);

			nb_fpga_write_mem32(0x00000a00,0x01);

			

			//ret= 0;

			if (ret == 0) {
#ifndef SINGLE_INTERFACE
      			nb_fpga_print("nba810_Led_write: success");
				printf("LEDdata=%x , LED_port = %x",leddata,led_port);
#else
				printf("Success\n");
#endif
			}

			else {
#ifndef SINGLE_INTERFACE
			nb_fpga_err("nba810_led_read: fail");
#else
			printf("Fail\n");
#endif
			}

			break;

		case NBA810_OP_QSFP_READ:	
			ret = nb_fpga_nba810_QSFP_Read(QSFP_channel,regaddr,&QSFP_R_data);

			QSFP_mem_select =( QSFP_channel -1 )/8;

			if (ret == 0) {
#ifndef SINGLE_INTERFACE
				nb_fpga_print("QSFP%x_Regaddr:0x%x Data: 0x%x",
						QSFP_channel,regaddr,
						nb_fpga_mmap_addr->nba810_i2c_data[QSFP_mem_select].r_data);
#else
				printf("Success:0x%x\n", nb_fpga_mmap_addr->nba810_i2c_data[QSFP_mem_select].r_data);
#endif						
			}

			else {
#ifndef SINGLE_INTERFACE
				nb_fpga_err("nba810_QSFP_read: fail");
#else
			printf("Fail\n");
#endif
			}

			break;

		case NBA810_OP_QSFP_WRITE:

			ret = nb_fpga_nba810_QSFP_Write(QSFP_channel,regaddr,data);

			if (ret == 0) {
#ifndef SINGLE_INTERFACE
      			nb_fpga_print("nba810_QSFP_write: success");
#else
				printf("Success\n");
#endif
			}

			else {
#ifndef SINGLE_INTERFACE
			nb_fpga_err("nba810_QSFP_write: fail");
#else
			printf("Fail\n");
#endif

			}

			break;
		case NBA810_OP_QSFP_initial:
			nba810_initial_err_count = 0;
			for(i=0;i<=39; i++)
			{
       		ret = nb_fpga_nba810_write_i2c(nba810_inital_i2c_port[i],nba810_inital_i2c_addr[i],nba810_inital_i2c_regaddr[i],nba810_initial_i2c_data[i]);
       		if (ret == 0) {

			}

			else {
			nba810_initial_err_count = nba810_initial_err_count + 1;

			}
       		}

       		if (nba810_initial_err_count == 0) {
#ifndef SINGLE_INTERFACE
      			nb_fpga_print("QSFP I/O initial success");
#else
				printf("Success\n");
#endif
			}

			else {
#ifndef SINGLE_INTERFACE
			nb_fpga_print("QSFP I/O initial Fail. Please initial QSFP again");
#else
			printf("Fail\n");
#endif
			}

			break;

		case NBA810_OP_QSFP_Set_LPMode:
			
			data32_shiftL_one= (data32 << 1)&(0xa0aaaa00);	

			data32_shiftR_one= (data32 >> 1)&(0x50555500);

			data_swap = (data32 & 0x0f0000ff) + data32_shiftL_one + data32_shiftR_one;
			
			ret1 = nb_fpga_nba810_write_i2c(0x05,0x20,0x02,data_swap);
			
			ret2 = nb_fpga_nba810_write_i2c(0x05,0x20,0x03,data_swap>>8);
			
			ret3 = nb_fpga_nba810_write_i2c(0x05,0x21,0x02,data_swap>>16);
			
			ret4 = nb_fpga_nba810_write_i2c(0x05,0x21,0x03,data_swap>>24);
			
			
			
			if ((ret1 || ret2 || ret3 || ret4) == 1) {
#ifndef SINGLE_INTERFACE	
      			nb_fpga_err("QSFP Set LPMode: fail");
#else
				printf("Fail\n");
#endif
			}

			else {
#ifndef SINGLE_INTERFACE	
			nb_fpga_print("QSFP Set LPMode success");
#else
			printf("Success\n");
#endif
			}
			
			
			break;
			
		case NBA810_OP_QSFP_Read_LPMode:

			ret1 = nb_fpga_nba810_read_i2c(0x05,0x20,0x0,(u_int16_t *)&data);
			QSFP_data_8to1 = nb_fpga_mmap_addr->nba810_i2c_data[4].r_data;
			
			ret2 = nb_fpga_nba810_read_i2c(0x05,0x20,0x1,(u_int16_t *)&data);
			QSFP_data_16to9 = nb_fpga_mmap_addr->nba810_i2c_data[4].r_data;

			ret3 = nb_fpga_nba810_read_i2c(0x05,0x21,0x0,(u_int16_t *)&data);
			QSFP_data_24to17 = nb_fpga_mmap_addr->nba810_i2c_data[4].r_data;
		
			ret4 = nb_fpga_nba810_read_i2c(0x05,0x21,0x1,(u_int16_t *)&data);
			QSFP_data_32to25 = nb_fpga_mmap_addr->nba810_i2c_data[4].r_data;
		
			QSFP_data_16to9_bitswap = ((QSFP_data_16to9>>1)&0x55)+((QSFP_data_16to9<<1)&0xaa);
			QSFP_data_24to17_bitswap= ((QSFP_data_24to17>>1)&0x55)+((QSFP_data_24to17<<1)&0xaa);
			QSFP_data_32to25_bitswap= (QSFP_data_32to25 & 0x0f)+((QSFP_data_32to25>>1)&0x50)+((QSFP_data_32to25<<1)&0xa0);
		
			if ((ret1 || ret2 || ret3 || ret4) == 1) {
#ifndef SINGLE_INTERFACE
      			nb_fpga_err("QSFP Read LPMode: fail");
#else
				printf("Fail\n");
#endif
			}

			else {
#ifndef SINGLE_INTERFACE			
			nb_fpga_print("QSFP8to1   Present:0x%x\n",QSFP_data_8to1);
			nb_fpga_print("QSFP16to9  Present:0x%x\n",QSFP_data_16to9_bitswap);
			nb_fpga_print("QSFP24to17 Present:0x%x\n",QSFP_data_24to17_bitswap);
			nb_fpga_print("QSFP32to25 Present:0x%x\n",QSFP_data_32to25_bitswap);

			nb_fpga_print("Read QSFP32to0 LPMode success\n");
#else
			u_int32_t all_pres =  (QSFP_data_32to25_bitswap << 24) | (QSFP_data_24to17_bitswap << 16) | (QSFP_data_16to9_bitswap << 8) | QSFP_data_8to1;
			printf("Success:0x%x\n",all_pres);
#endif
			}
			

			break;

		case NBA810_OP_QSFP_Set_Reset:
		
			data32_shiftL_one= (data32 << 1)&(0xa0aaaa00);

			data32_shiftR_one= (data32 >> 1)&(0x50555500);

			data_swap = (data32 & 0x0f0000ff) + data32_shiftL_one + data32_shiftR_one;
		
			ret1 = nb_fpga_nba810_write_i2c(0x06,0x26,0x02,data_swap);
			
			ret2 = nb_fpga_nba810_write_i2c(0x06,0x26,0x03,data_swap>>8);
			
			ret3 = nb_fpga_nba810_write_i2c(0x06,0x27,0x02,data_swap>>16);
			
			ret4 = nb_fpga_nba810_write_i2c(0x06,0x27,0x03,data_swap>>24);
			
			
			if ((ret1 || ret2 || ret3 || ret4) == 1) {
#ifndef SINGLE_INTERFACE
      			nb_fpga_err("QSFP Set Reset: fail");
#else
				printf("Fail\n");
#endif
			}

			else {
#ifndef SINGLE_INTERFACE
			nb_fpga_print("QSFP Set Reset success");
#else
			printf("Success\n");
#endif
			}
#ifndef SINGLE_INTERFACE
			//We don't need to recover reset signle automatically,
			//because of the high level(barefoot BSP) behavior.
			
			usleep(1000000);
			ret1 = nb_fpga_nba810_write_i2c(0x06,0x26,0x02,0xff);
			ret2 = nb_fpga_nba810_write_i2c(0x06,0x26,0x03,0xff);
			ret3 = nb_fpga_nba810_write_i2c(0x06,0x27,0x02,0xff);
			ret4 = nb_fpga_nba810_write_i2c(0x06,0x27,0x03,0xff);
			
			if ((ret1 || ret2 || ret3 || ret4) == 1) {
			nb_fpga_err("QSFP Recover Reset: fail");
			}
            else {
            nb_fpga_print("QSFP Recover Reset success");
            }
			
#endif
			break;


		case NBA810_OP_QSFP_Read_Present:

			if( ! _get_orignal_present() ) {
#ifndef SINGLE_INTERFACE
				nb_fpga_err("QSFP Read Present: fail");
   				nb_fpga_print("<Err>: Read temp file fail\n");
#else
				printf("Fail\n");
#endif
				break;
			}

			ret1 = nb_fpga_nba810_read_i2c(0x07,0x22,0x0,(u_int16_t *)&data);
			QSFP_data_8to1 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
			if ((QSFP_data_8to1 != _ori_present_8to1) && (ret1 != 1)) {
				for(i=0; i<3; i++) {
					usleep(100000);
					nb_fpga_nba810_read_i2c(0x07,0x22,0x0,(u_int16_t *)&data);
					QSFP_data_8to1 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
					if (QSFP_data_8to1 == _ori_present_8to1) {
						break;
					}
				}
			}
			_ori_present_8to1 = QSFP_data_8to1;

			ret2 = nb_fpga_nba810_read_i2c(0x07,0x22,0x1,(u_int16_t *)&data);
			QSFP_data_16to9 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
			if ((QSFP_data_16to9 != _ori_present_16to9) && (ret2 != 1)) {
				for(i=0; i<3; i++) {
					usleep(100000);
					nb_fpga_nba810_read_i2c(0x07,0x22,0x1,(u_int16_t *)&data);
					QSFP_data_16to9 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
					if (QSFP_data_16to9 == _ori_present_16to9) {
						break;
					}
				}
			}
			_ori_present_16to9 = QSFP_data_16to9;

			ret3 = nb_fpga_nba810_read_i2c(0x07,0x23,0x0,(u_int16_t *)&data);
			QSFP_data_24to17 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
			if ((QSFP_data_24to17 != _ori_present_24to17) && (ret3 != 1)) {
				for(i=0; i<3; i++) {
					usleep(100000);
					nb_fpga_nba810_read_i2c(0x07,0x23,0x0,(u_int16_t *)&data);
					QSFP_data_24to17 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
					if (QSFP_data_24to17 == _ori_present_24to17) {
						break;
					}
				}
			}
			_ori_present_24to17 = QSFP_data_24to17;

			ret4 = nb_fpga_nba810_read_i2c(0x07,0x23,0x1,(u_int16_t *)&data);
			QSFP_data_32to25 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
			if ((QSFP_data_32to25 != _ori_present_32to25) && (ret4 != 1)) {
				for(i=0; i<3; i++) {
					usleep(100000);
					nb_fpga_nba810_read_i2c(0x07,0x23,0x1,(u_int16_t *)&data);
					QSFP_data_32to25 = nb_fpga_mmap_addr->nba810_i2c_data[6].r_data;
					if (QSFP_data_32to25 == _ori_present_32to25) {
						break;
					}
				}
			}
			_ori_present_32to25 = QSFP_data_32to25;

			QSFP_data_16to9_bitswap = ((_ori_present_16to9>>1)&0x55)+((_ori_present_16to9<<1)&0xaa);
			QSFP_data_24to17_bitswap= ((_ori_present_24to17>>1)&0x55)+((_ori_present_24to17<<1)&0xaa);
			QSFP_data_32to25_bitswap= (_ori_present_32to25 & 0x0f)+((_ori_present_32to25>>1)&0x50)+((_ori_present_32to25<<1)&0xa0);

			if ((ret1 || ret2 || ret3 || ret4) == 1) {
#ifndef SINGLE_INTERFACE
      			nb_fpga_err("QSFP Read Present: fail");
#else
				printf("Fail\n");
#endif 
			} else {
                if ( ! _set_orignal_present()) {
#ifndef SINGLE_INTERFACE
      			    nb_fpga_err("QSFP Read Present: Write tmp file fail\n");
#endif 	
			    }

#ifndef SINGLE_INTERFACE
			nb_fpga_print("QSFP8to1   Present:0x%x\n",QSFP_data_8to1);
			nb_fpga_print("QSFP16to9  Present:0x%x\n",QSFP_data_16to9_bitswap);
			nb_fpga_print("QSFP24to17 Present:0x%x\n",QSFP_data_24to17_bitswap);
			nb_fpga_print("QSFP32to25 Present:0x%x\n",QSFP_data_32to25_bitswap);

			nb_fpga_print("Read QSFP32to0 Present success\n");
#else
			u_int32_t all_pres =  (QSFP_data_32to25_bitswap << 24) | (QSFP_data_24to17_bitswap << 16) | (QSFP_data_16to9_bitswap << 8) | QSFP_data_8to1;
			printf("Success:0x%x\n",all_pres);
#endif
			}
			

			break;

		case NBA810_OP_QSFP_Read_RxLoss:
		
			ret1 = nb_fpga_nba810_read_i2c(0x08,0x24,0x0,(u_int16_t *)&data);
			QSFP_data_8to1 = nb_fpga_mmap_addr->nba810_i2c_data[7].r_data;
			
			ret2 = nb_fpga_nba810_read_i2c(0x08,0x24,0x1,(u_int16_t *)&data);
			QSFP_data_16to9 = nb_fpga_mmap_addr->nba810_i2c_data[7].r_data;

			ret3 = nb_fpga_nba810_read_i2c(0x08,0x25,0x0,(u_int16_t *)&data);
			QSFP_data_24to17 = nb_fpga_mmap_addr->nba810_i2c_data[7].r_data;
		
			ret4 = nb_fpga_nba810_read_i2c(0x08,0x25,0x1,(u_int16_t *)&data);
			QSFP_data_32to25 = nb_fpga_mmap_addr->nba810_i2c_data[7].r_data;
		
			QSFP_data_16to9_bitswap = ((QSFP_data_16to9>>1)&0x55)+((QSFP_data_16to9<<1)&0xaa);
			QSFP_data_24to17_bitswap= ((QSFP_data_24to17>>1)&0x55)+((QSFP_data_24to17<<1)&0xaa);
			QSFP_data_32to25_bitswap= (QSFP_data_32to25 & 0x0f)+((QSFP_data_32to25>>1)&0x50)+((QSFP_data_32to25<<1)&0xa0);
		
		
			if ((ret1 || ret2 || ret3 || ret4) == 1) {

      			nb_fpga_err("QSFP Read Rx_Loss: fail");

			}

			else {
			nb_fpga_print("QSFP8to1   Rx_loss:0x%x\n",QSFP_data_8to1);
			nb_fpga_print("QSFP16to9  Rx_loss:0x%x\n",QSFP_data_16to9_bitswap);
			nb_fpga_print("QSFP24to17 Rx_loss:0x%x\n",QSFP_data_24to17_bitswap);
			nb_fpga_print("QSFP32to25 Rx_loss:0x%x\n",QSFP_data_32to25_bitswap);
			printf("Read QSFP32to0 Rx_loss success\n");
			}

			break;


		default:

			usage(argv[0]);

      exit(3);

			break;

		} 

		// release FPGA resource

		nb_fpga_exit(); 

		exit(0);

}
