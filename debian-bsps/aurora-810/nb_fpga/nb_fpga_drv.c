#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "nb_fpga.h"

#define SINGLE_INTERFACE

u_int32_t channel_table[32]={0x00000001,0x00000002,0x00000004,0x00000008,0x00000010,0x00000020,0x00000040,0x00000080,
			     0x00000002,0x00000001,0x00000008,0x00000004,0x00000020,0x00000010,0x00000080,0x00000040,
			     0x00000002,0x00000001,0x00000008,0x00000004,0x00000020,0x00000010,0x00000080,0x00000040,
			     0x00000001,0x00000002,0x00000004,0x00000008,0x00000020,0x00000010,0x00000080,0x00000040};

int fpga_fd;

u_int16_t mem_data;

u_int16_t aaa[100];

int ii=0;

int mdio_status_check(u_int8_t slot_mask)
{
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    
    nb_fpga_mmap_addr->mdio_data.slot = slot_mask;
    while ( nb_fpga_mmap_addr->mdio_data.status != NB_FPGA_STATUS_DONE && time_count) {
 				if( nb_fpga_mmap_addr->mdio_data.status == NB_FPGA_STATUS_TIMEOUT) {
    			nb_fpga_mmap_addr->mdio_data.control = NB_FPGA_MDIO_CLEAR;
    			nb_fpga_err("slot (%d) time out,clear status and retry", slot_mask);
    		} 
 				NB_FPGA_ACCESS_DELAY;
 				time_count--;		
    }
    if(time_count) {   	
    	return 0;
    } 
    else	{
    	if( nb_fpga_mmap_addr->mdio_data.status == NB_FPGA_STATUS_BUSY) {
   			nb_fpga_err("slot (%d) busy", slot_mask);
    	} 
    	else if( nb_fpga_mmap_addr->mdio_data.status == NB_FPGA_STATUS_TIMEOUT) {
    		nb_fpga_mmap_addr->mdio_data.control = NB_FPGA_MDIO_CLEAR;
    		nb_fpga_err("slot (%d) timeout", slot_mask);
    	}	
    	return -1;
    }	
}

int fw_download_status_check()
{
    int time_count = NB_FPGA_FW_DOWNLOAD_TIME_COUNT;
    
    nb_fpga_print("");
    while ( nb_fpga_mmap_addr->mdio_data.status != NB_FPGA_STATUS_DONE && time_count) {
 				if( nb_fpga_mmap_addr->mdio_data.status == NB_FPGA_STATUS_TIMEOUT) {
    			nb_fpga_mmap_addr->mdio_data.control = NB_FPGA_MDIO_CLEAR;
    			nb_fpga_err("prog gbfw time out,clear status and retry");
    		} 
 				NB_FPGA_ACCESS_DELAY;
 				time_count--;		
    }
    if(time_count) {
    	return 0;
    } 
    else	{
    	if( nb_fpga_mmap_addr->mdio_data.status == NB_FPGA_STATUS_BUSY) {
   			nb_fpga_err("prog gbfw busy");
    	} 
    	else if( nb_fpga_mmap_addr->mdio_data.status == NB_FPGA_STATUS_TIMEOUT) {
				nb_fpga_mmap_addr->mdio_data.control = NB_FPGA_MDIO_CLEAR;
    		nb_fpga_err("prog gbfw timeout");
    	}	
    	return -1;
    }	
}

int led_status_check(int slot)
{
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while ( nb_fpga_mmap_addr->led_i2c_data[slot].status != NB_FPGA_STATUS_DONE && time_count) {
				if( nb_fpga_mmap_addr->led_i2c_data[slot].status == NB_FPGA_STATUS_TIMEOUT) {
    			nb_fpga_mmap_addr->led_i2c_data[slot].control = NB_FPGA_LED_CLEAR;
    			nb_fpga_err("slot (%d) time out,clear status and retry", slot);
    		} 
        NB_FPGA_ACCESS_DELAY;
 				time_count--;
    }
    if(time_count) {   	
    	return 0;
    } 
    else	{
    	if( nb_fpga_mmap_addr->led_i2c_data[slot].status == NB_FPGA_STATUS_BUSY) {
   			nb_fpga_err("slot (%d) busy", slot);
    	} 
    	else if( nb_fpga_mmap_addr->led_i2c_data[slot].status == NB_FPGA_STATUS_TIMEOUT) {
    		nb_fpga_err("slot (%d) timeout", slot);
    	}	
    	return -1;
    }	
}

int media_status_check(int slot)
{
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while ( nb_fpga_mmap_addr->media_i2c_data[slot].status != NB_FPGA_STATUS_DONE && time_count) {
				if( nb_fpga_mmap_addr->media_i2c_data[slot].status == NB_FPGA_STATUS_TIMEOUT) {
    			nb_fpga_mmap_addr->media_i2c_data[slot].control = NB_FPGA_MEDIA_CLEAR;
    			nb_fpga_err("slot (%d) time out,clear status and retry", slot);
    		}
        NB_FPGA_ACCESS_DELAY;
 				time_count--;
    }
    
    if(time_count) {   	
    	return 0;
    } 
		else	{
    	if( nb_fpga_mmap_addr->media_i2c_data[slot].status == NB_FPGA_STATUS_BUSY) {
   			nb_fpga_err("slot (%d) busy", slot);
    	} 
    	else if( nb_fpga_mmap_addr->media_i2c_data[slot].status == NB_FPGA_STATUS_TIMEOUT) {
    		nb_fpga_err("slot (%d) timeout", slot);
    	}	
    	return -1;
    }	
}

int nb_fpga_init()
{
    const char *device = getenv("nb_fpga_dev");//NB_FPGA_RES_ADDR
    struct stat sb;

		if (device == NULL) {
			nb_fpga_err("Please export [nb_fpga_dev] env");
			return 1;
		}
     
    fpga_fd = open(device, O_RDWR | O_SYNC | O_RSYNC);
    if(fpga_fd < 0) {
        nb_fpga_err("open %s failed", device);
        return 1;
    }
    
    fstat(fpga_fd, &sb);
    nb_fpga_debug("BAR size: 0x%lx ",sb.st_size);
    nb_fpga_mmap_addr = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fpga_fd, 0);

    if (nb_fpga_mmap_addr == MAP_FAILED) {
        nb_fpga_err("PCI BAR0 mmap failed");
        close(fpga_fd);
        return 1;
    }    
    nb_fpga_debug("FPGA mmap success");
    
    return 0;	
}

int nb_fpga_exit()
{
    munmap(nb_fpga_mmap_addr, NB_FPGA_MEM_SIZE);
    close(fpga_fd);
    nb_fpga_debug("FPGA mmap released");
    return 0;	
}


int nb_fpga_write_mdio(u_int8_t slot_mask, u_int8_t slice, u_int16_t reg_addr, u_int16_t data) {

		u_int32_t mdio_tmp = slot_mask <<16 | slice <<8 | NB_FPGA_MDIO_WRITE;
		u_int32_t * addr = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + NB_FPGA_MDIO_START_ADDR);

		// 1.read 0x0008  check status
    if( mdio_status_check(slot_mask) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->mdio_data.reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x000c Write data
    	nb_fpga_mmap_addr->mdio_data.w_data = data;  	
    	NB_FPGA_CMD_DELAY;
    	// 4.write 0x0000 Slice and Control
  		//nb_fpga_mmap_addr->mdio_data.slot = slot_mask;
  		//nb_fpga_mmap_addr->mdio_data.slice = slice;
  		//nb_fpga_mmap_addr->mdio_data.control = NB_FPGA_MDIO_WRITE;
  		//fixed me /* FPGA bug? */
  		*addr = mdio_tmp;
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
	  
	  // 5.read  0x0008  check status
	  if( mdio_status_check(slot_mask) == NB_FPGA_STATUS_DONE) {			
  		return 0;
  	} else {
			return 1;
	  }
	   
}

int nb_fpga_read_mdio(u_int8_t slot_mask, u_int8_t slice, u_int16_t reg_addr, u_int16_t *data)
{
		u_int32_t mdio_tmp = slot_mask <<16 | slice <<8 | NB_FPGA_MDIO_READ;
		u_int32_t * addr = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + NB_FPGA_MDIO_START_ADDR);
		// 1.read 0x0008  check status
    if( mdio_status_check(slot_mask) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->mdio_data.reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x0000 Slice and Control
  		//nb_fpga_mmap_addr->mdio_data.slot = slot_mask;
  		//nb_fpga_mmap_addr->mdio_data.slice = slice;
  		//nb_fpga_mmap_addr->mdio_data.control = NB_FPGA_MDIO_READ;
 		
  		//fixed me /* FPGA bug? */
  		*addr = mdio_tmp;
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
		// 4.read  0x0008 check status
		if( mdio_status_check(slot_mask) == NB_FPGA_STATUS_DONE) {	 
       // 5.read  0x0010 Register data
       *data = nb_fpga_mmap_addr->mdio_data.r_data;
  	} else {
			return 1;
	  }
    return 0;
}


int nb_firmware_prog(u_int8_t slot_mask) {
	u_int32_t mdio_tmp = slot_mask <<16 | NB_FPGA_MDIO_FW_DOWNLOAD;
	u_int32_t * addr = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + NB_FPGA_MDIO_START_ADDR);
  int ret;  	
	nb_fpga_print("programing..");
	/* set broadcast fw download */
	nb_fpga_write_mdio(slot_mask, 0, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 1, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 2, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 3, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 4, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 5, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 6, 0x985a, 0x8888);
	nb_fpga_write_mdio(slot_mask, 7, 0x985a, 0x8888);
	
	//fixed me /* FPGA bug? */
 	*addr = mdio_tmp;
	sleep(3);
	ret = fw_download_status_check();

	/* clear broadcast fw download */
	nb_fpga_write_mdio(slot_mask, 0, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 1, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 2, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 3, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 4, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 5, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 6, 0x985a, 0x0);
	nb_fpga_write_mdio(slot_mask, 7, 0x985a, 0x0);
 	nb_fpga_print("success");
	return ret;
}



int nb_fpga_write_media_byte(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data) {

		// 1.read 0x0008  check status
    if( media_status_check(slot) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->media_i2c_data[slot].reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x000c Write data
    	nb_fpga_mmap_addr->media_i2c_data[slot].w_data = data &0xff;  	
    	NB_FPGA_CMD_DELAY;
    	// 4.write 0x0000 i2c_addr and Control
    	nb_fpga_mmap_addr->media_i2c_data[slot].i2c_addr = i2c_addr;
    	nb_fpga_mmap_addr->media_i2c_data[slot].control = NB_FPGA_MEDIA_WRITE_BYTE;
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
	  
	  // 5.read  0x0008  check status
	  if( media_status_check(slot) == NB_FPGA_STATUS_DONE) {			
  		return 0;
  	} else {
			return 1;
	  }
	   
}

int nb_fpga_read_media_byte(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data)
{

		// 1.read 0x0008  check status
    if( media_status_check(slot) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->media_i2c_data[slot].reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x0000 i2c_addr and Control
    	nb_fpga_mmap_addr->media_i2c_data[slot].i2c_addr = i2c_addr;
    	nb_fpga_mmap_addr->media_i2c_data[slot].control = NB_FPGA_MEDIA_READ_BYTE;
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
    
		// 4.read  0x0008 check status
		if( media_status_check(slot) == NB_FPGA_STATUS_DONE) {	 
       // 5.read  0x0010 Register data
      *data = nb_fpga_mmap_addr->media_i2c_data[slot].r_data &0xff;
  	} else {
			return 1;
	  }
    return 0;
}

int nb_fpga_set_i2c_mux(int slot, u_int16_t i2c_addr, u_int16_t i2c_channel) {

		// 1.read 0x0008  check status
    if( media_status_check(slot) == NB_FPGA_STATUS_DONE) {	
    	// 2.writ I2C 9548 control register 
			i2c_channel = 0x1 << (i2c_channel - 1); // convert to bit mask
    	nb_fpga_mmap_addr->media_i2c_data[slot].reg_addr = i2c_channel &0xff; 	
    	NB_FPGA_CMD_DELAY;
    	// 4.write 0x0000 i2c_addr and Control
    	nb_fpga_mmap_addr->media_i2c_data[slot].i2c_addr = i2c_addr;
    	nb_fpga_mmap_addr->media_i2c_data[slot].control = NB_FPGA_MEDIA_WRITE_CONTROL;
  		NB_FPGA_CMD_DELAY;
  		NB_FPGA_CMD_DELAY;// set mux need extra delay
  	} else {
			return 1;
	  }
	  
	  // 5.read  0x0008  check status
	  if( media_status_check(slot) == NB_FPGA_STATUS_DONE) {			
  		return 0;
  	} else {
			return 1;
	  }
	   
}

int nb_fpga_write_led(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data) {
				
		// 1.read 0x0008  check status
    if( led_status_check(slot) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->led_i2c_data[slot].reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x000c Write data
    	nb_fpga_mmap_addr->led_i2c_data[slot].w_data = data;  	
    	NB_FPGA_CMD_DELAY;
    	// 4.write 0x0000 i2c_addr and Control
    	nb_fpga_mmap_addr->led_i2c_data[slot].i2c_addr = i2c_addr;
    	nb_fpga_mmap_addr->led_i2c_data[slot].control = NB_FPGA_LED_WRITE;
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
	  
	  // 5.read  0x0008  check status
	  if( led_status_check(slot) == NB_FPGA_STATUS_DONE) {			
  		return 0;
  	} else {
			return 1;
	  }
	   
}

int nb_fpga_read_led(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data)
{

		// 1.read 0x0008  check status
    if( led_status_check(slot) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->led_i2c_data[slot].reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x0000 i2c_addr and Control
    	nb_fpga_mmap_addr->led_i2c_data[slot].i2c_addr = i2c_addr;
    	nb_fpga_mmap_addr->led_i2c_data[slot].control = NB_FPGA_LED_READ;
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
    
		// 4.read  0x0008 check status
		if( led_status_check(slot) == NB_FPGA_STATUS_DONE) {	 
       // 5.read  0x0010 Register data
      *data = nb_fpga_mmap_addr->led_i2c_data[slot].r_data;
  	} else {
			return 1;
	  }
    return 0;
}

int nb_fpga_write_mem32(int offset, u_int32_t data) {
		NB_FPGA_CMD_DELAY;
		u_int32_t *addr =(u_int32_t*) ( (u_int8_t *)nb_fpga_mmap_addr + offset);
		NB_FPGA_CMD_DELAY;
		*addr = data;

	  //nb_fpga_print("offset:0x%x data:0x%08X",offset,*addr);
	  return 0; 
}

int nb_fpga_read_mem32(int offset)
{
  
    NB_FPGA_CMD_DELAY;
    u_int32_t *addr = (u_int32_t *) ((u_int8_t *)nb_fpga_mmap_addr + offset);
//    mem_data = *addr;
    if(mem_data >= 0) {
		NB_FPGA_CMD_DELAY;
	}
#ifndef SINGLE_INTERFACE
    nb_fpga_print("offset:0x%x data:0x%08X",offset,*addr);
#else
	printf("Success:%d\n", *addr);
#endif

//nb_fpga_print("info[3][2]:0x%x%x",nb_fpga_mmap_addr->fpga_info.info[3],nb_fpga_mmap_addr->fpga_info.info[2]);
//	volatile u_int32_t *addr=(u_int32_t *)((u_int8_t *)nb_fpga_mmap_addr + offset);
//	read(fpga_fd,aaa,100);
//	if(msync(nb_fpga_mmap_addr,NB_FPGA_MEM_SIZE,MS_SYNC)==-1) {
//	    	nb_fpga_err("mmap sync to file error");
//	    	return 1;
//	}
//        nb_fpga_print("msync = %d",msync(nb_fpga_mmap_addr,NB_FPGA_MEM_SIZE,MS_SYNC));	
//	nb_fpga_print("offset:0x%x data:0x%08X",offset,*addr);

//	nb_fpga_print("info[%d]=0x%x \n",offset,nb_fpga_mmap_addr->fpga_info.info[offset]);
//	for(ii=0;ii<=20;ii++) {
//		nb_fpga_print("info[%d]=0x%x \n",ii,nb_fpga_mmap_addr->fpga_info.info[ii]);
//	}
	return 0;
}


int nb_firmware_write(const char *filename) {

	int rc,fd,fw_size=0;
	int i=0;
	unsigned char *firmware_image;
	struct stat sb;

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		nb_fpga_err("file:%s size:%d open failed", filename, fw_size);
		close(fd);
		return 1;
	}

	rc = fstat(fd, &sb);	
	if (rc == -1) {
		nb_fpga_err("file:%s size:%d check failed", filename, fw_size);
		close(fd);
		return 1;
	}
	fw_size = sb.st_size;
	if (fw_size < 40*1024) {
		nb_fpga_err("file:%s size:%d size failed", filename, fw_size);
		close(fd);
		return 1;
	}
	
	firmware_image = malloc(fw_size);
	rc = read(fd, firmware_image, fw_size);	
	nb_fpga_print("[file:%s size:%d]", filename, fw_size);
	if (rc != fw_size) {
		printf("[file:%s size:%d] read failed\n", filename, fw_size);
		free(firmware_image);
		close(fd);
		return 1;
	}
	// Copy GB firmware image to FPGA RAM
	//fixed me /* FPGA bug? */ memcpy(nb_fpga_mmap_addr->gearbox_fw, firmware_image, fw_size);
	for (i=0; i < fw_size; i=i+4)
	{		
		memcpy(nb_fpga_mmap_addr->gearbox_fw+i, firmware_image+i, 4);	
		usleep(100);
	}	
	
	nb_fpga_print("success");
	free(firmware_image);
	close(fd);
	return 0;
}

int nb_firmware_read(const char *filename) {

	//int rc,fd,fw_size=0;
	int fd;
	int i=0;
	unsigned char *firmware_image;
	//struct stat sb;

	fd = open(filename, O_CREAT|O_RDWR|O_TRUNC, 00777);
	if (fd == -1) {
		nb_fpga_err("file:%s open failed", filename);
		close(fd);
		return 1;
	}
	
	firmware_image = malloc(NB_FPGA_GBFW_SIZE);
	// Copy FPGA FW buffer to file for debug
	//fixed me /* FPGA bug? */ memcpy(firmware_image, nb_fpga_mmap_addr->gearbox_fw, NB_FPGA_GBFW_SIZE);
	for (i=0; i < NB_FPGA_GBFW_SIZE; i=i+4)
	{		
		memcpy(firmware_image+i, nb_fpga_mmap_addr->gearbox_fw+i, 4);	
		usleep(100);
	}	
	
	write(fd, firmware_image, NB_FPGA_GBFW_SIZE);
	nb_fpga_print("Read firmware from FPGA mem success");
	free(firmware_image);
	close(fd);
	return 0;
}


//===================jim add 20220401=================================================
int nba810_i2c_status_check(int slot)
{
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while ( nb_fpga_mmap_addr->nba810_i2c_data[slot].status != NB_FPGA_STATUS_DONE && time_count) {
				if( nb_fpga_mmap_addr->nba810_i2c_data[slot].status == NB_FPGA_STATUS_TIMEOUT) {
    			nb_fpga_mmap_addr->nba810_i2c_data[slot].control = NB_FPGA_MEDIA_CLEAR;
    			nb_fpga_err("slot (%d) time out,clear status and retry", slot+1);
    		}
        NB_FPGA_ACCESS_DELAY;
 				time_count--;
    }
    
    if(time_count) {   	
    	return 0;
    } 
		else	{
    	if( nb_fpga_mmap_addr->media_i2c_data[slot].status == NB_FPGA_STATUS_BUSY) {
   			nb_fpga_err("slot (%d) busy", slot);
    	} 
    	else if( nb_fpga_mmap_addr->media_i2c_data[slot].status == NB_FPGA_STATUS_TIMEOUT) {
    		nb_fpga_err("slot (%d) timeout", slot);
    	}	
    	return -1;
    }	
}


int nb_fpga_nba810_write_i2c(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data) {
				
		// 1.read 0x0008  check status
    if( nba810_i2c_status_check(slot-1) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	u_int32_t i2c_addr_control = (i2c_addr <<16) + 0x00000002;
	u_int32_t *addr = (u_int32_t *)((u_int8_t *)nb_fpga_mmap_addr +( 0x200+(0x0100*(slot-1))));
	nb_fpga_mmap_addr->nba810_i2c_data[slot-1].reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x000c Write data
    	nb_fpga_mmap_addr->nba810_i2c_data[slot-1].w_data = data;  	    	
	NB_FPGA_CMD_DELAY;
	*addr = i2c_addr_control;
	NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
	  
	  // 5.read  0x0008  check status
	  if( led_status_check(slot-1) == NB_FPGA_STATUS_DONE) {
  		return 0;
  	} else {
			return 1;
	  }
	   
}

int nb_fpga_nba810_read_i2c(int slot, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data)
{

		// 1.read 0x0008  check status
    if( nba810_i2c_status_check(slot-1) == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	u_int32_t i2c_addr_control = (i2c_addr <<16) + 0x00000001;
	u_int32_t *addr = (u_int32_t *)((u_int8_t *)nb_fpga_mmap_addr +( 0x200+(0x0100*(slot-1))));
    	
    	nb_fpga_mmap_addr->nba810_i2c_data[slot-1].reg_addr = reg_addr;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x0000 i2c_addr and Control
	*addr = i2c_addr_control;
	NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
    
		// 4.read  0x0008 check status
		if( nba810_i2c_status_check(slot-1) == NB_FPGA_STATUS_DONE) {	 
       // 5.read  0x0010 Register data
      *data = nb_fpga_mmap_addr->nba810_i2c_data[slot-1].r_data;
  	} else {
			return 1;
	  }
    return 0;
}

int nba810_led_status_check(void)
{
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while ( nb_fpga_mmap_addr->nba810_led_dara.status != NB_FPGA_STATUS_DONE && time_count) {
				if( nb_fpga_mmap_addr->nba810_led_dara.status == NB_FPGA_STATUS_TIMEOUT) {
    			nb_fpga_mmap_addr->nba810_led_dara.control = NB_FPGA_MEDIA_CLEAR;
    			nb_fpga_err("NBA810 LED time out,clear status and retry");
    		}
        NB_FPGA_ACCESS_DELAY;
 				time_count--;
    }
    
    if(time_count) {   	
    	return 0;
    } 
		else	{
    	if( nb_fpga_mmap_addr->nba810_led_dara.status == NB_FPGA_STATUS_BUSY) {
   			nb_fpga_err("NBA810 LED busy");
    	} 
    	else if( nb_fpga_mmap_addr->nba810_led_dara.status == NB_FPGA_STATUS_TIMEOUT) {
    		nb_fpga_err("NBA810 LED Time out");
    	}	
    	return -1;
    }	
}

/*
int nb_fpga_nba810_QSFP_Mux(int QSFP_channel)
{
u_int32_t channel;
u_int16_t i2c_mux_addr;
int  	i2c_number;

u_int32_t * i2c_addr_1 = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + 0x00000200  );
u_int32_t * i2c_addr_2 = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + 0x00000300  );
u_int32_t * i2c_addr_3 = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + 0x00000400  );
u_int32_t * i2c_addr_4 = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + 0x00000500  );

	if(QSFP_channel==0) {
	channel = 0
	NB_FPGA_CMD_DELAY
	
		if(( nba810_i2c_status_check(0) == NB_FPGA_STATUS_DONE) && ( nba810_i2c_status_check(1) == NB_FPGA_STATUS_DONE) &&
		   ( nba810_i2c_status_check(2) == NB_FPGA_STATUS_DONE) &&( nba810_i2c_status_check(3) == NB_FPGA_STATUS_DONE) ) 
		{
		   nb_fpga_mmap_addr->nba810_i2c_data[0].reg_addr = 0;
		   nb_fpga_mmap_addr->nba810_i2c_data[1].reg_addr = 0;
		   nb_fpga_mmap_addr->nba810_i2c_data[2].reg_addr = 0;
		   nb_fpga_mmap_addr->nba810_i2c_data[3].reg_addr = 0;
		   NB_FPGA_CMD_DELAY
		   * i2c_addr_1 = 0x00700004;
		   * i2c_addr_1 = 0x00710004;
		   * i2c_addr_1 = 0x00720004;
		   * i2c_addr_1 = 0x00730004;
		   NB_FPGA_CMD_DELAY;
		}
  		else 
  		{
			return 0;
		}
		
		
		if(( nba810_i2c_status_check(0) == NB_FPGA_STATUS_DONE) && ( nba810_i2c_status_check(1) == NB_FPGA_STATUS_DONE) &&
		   ( nba810_i2c_status_check(2) == NB_FPGA_STATUS_DONE) && ( nba810_i2c_status_check(3) == NB_FPGA_STATUS_DONE)) {	 
  			return 0;
  		} else {
			return 1;
	  	}
	}
	} else if((QSFP_channel>0)&&(QSFP_channel<=8){
	channel = 0x00000001 <<(QSFP_channel-1);
	i2c_number   = 0;
	i2c_mux_addr = 0x70;
	} else if((QSFP_channel>8)&&(QSFP_channel<=16){
	channel = 0x00000001 <<(QSFP_channel-9);
	i2c_number   = 1;
	i2c_mux_addr = 0x71;
	} else if((QSFP_channel>16)&&(QSFP_channel<=24){
	channel = 0x00000001 <<(QSFP_channel-17);
	i2c_number   = 2;
	i2c_mux_addr = 0x72;
	} else if((QSFP_channel>24)&&(QSFP_channel<=32){
	channel = 0x00000001 <<(QSFP_channel-25);
	i2c_number   = 3;
	i2c_mux_addr = 0x73;
	} else {
	nb_fpga_err("error:QSFP_channel over range");
	}
5`
 	u_int32_t qsfp_mux_control =  i2c_mux_addr <<16 | 0x0004;
	u_int32_t * addr = (u_int32_t *) ( (u_int8_t *)nb_fpga_mmap_addr + 0x00000200 + (0x00000100 * i2c_number));

// 1.read 0x0008  check status
    if( nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE) {	
  	// 2.write 0x0004 Register address
    	nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].reg_addr = channel;
    	NB_FPGA_CMD_DELAY;
    	// 3.write 0x0000 i2c_addr and Control
    	* addr = qsfp_mux_control;
    	
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
    
		// 4.read  0x0008 check status
     if( nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE) {	 
  			return 0;
  	} else {
			return 1;
	  }
    return 0;
}
*/


int nb_fpga_nba810_QSFP_Read(int QSFP_channel,u_int16_t Reg_addr,u_int16_t **R_data)
{
u_int32_t channel;
u_int16_t i2c_mux_addr;
int  	i2c_number;
u_int16_t R_QSFP_data;
	if((QSFP_channel>0)&&(QSFP_channel<=8)) {
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 0;
	i2c_mux_addr = 0x70;
	}
	else if((QSFP_channel>8)&&(QSFP_channel<=16)){
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 1;
	i2c_mux_addr = 0x71;
	}
	else if((QSFP_channel>16)&&(QSFP_channel<=24)){
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 2;
	i2c_mux_addr = 0x72;
	}
	else if((QSFP_channel>24)&&(QSFP_channel<=32)){
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 3;
	i2c_mux_addr = 0x73;
	}
	else {
	nb_fpga_err("error:QSFP_channel over range");
	return 1;
	}

	u_int32_t * i2c_addr = (u_int32_t *) ((u_int8_t *)nb_fpga_mmap_addr + 0x00000200 + (0x00000100 * i2c_number));


	if( nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE){
	nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].reg_addr = channel;
	* i2c_addr = (i2c_mux_addr << 16) + 0x00000004;
	NB_FPGA_CMD_DELAY;
	}
	else {
	return 0;
	}
	
	if(nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE) {	 
		nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].reg_addr = Reg_addr;
		* i2c_addr = 0x00500001;
		NB_FPGA_CMD_DELAY;
	}
	else {
		return 1;
	}



    if( nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE) {	
  	// 2.write 0x0004 Register address
		R_QSFP_data = nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].r_data;
		
		if(R_QSFP_data>=0) {
			NB_FPGA_CMD_DELAY;
		}
		
		return 0;
  	} 
  	else {
		return 1;
	}

}


int nb_fpga_nba810_QSFP_Write(int QSFP_channel,u_int16_t Reg_addr,u_int16_t w_data)
{
u_int32_t channel;
u_int16_t i2c_mux_addr;
int  	i2c_number;

	if((QSFP_channel>0)&&(QSFP_channel<=8)) {
	channel = channel_table[QSFP_channel-1];
	printf("QSFP channel number = %x \n",channel);
	i2c_number   = 0;
	i2c_mux_addr = 0x70;
	}
	else if((QSFP_channel>8)&&(QSFP_channel<=16)){
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 1;
	i2c_mux_addr = 0x71;
	}
	else if((QSFP_channel>16)&&(QSFP_channel<=24)){
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 2;
	i2c_mux_addr = 0x72;
	}
	else if((QSFP_channel>24)&&(QSFP_channel<=32)){
	channel = channel_table[QSFP_channel-1];
	i2c_number   = 3;
	i2c_mux_addr = 0x73;
	}
	else {
	nb_fpga_err("error:QSFP_channel over range");
	return 1;
	}

	u_int32_t * i2c_addr = (u_int32_t *) ((u_int8_t *)nb_fpga_mmap_addr + 0x00000200 + (0x00000100 * i2c_number));


	if( nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE){
	nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].reg_addr = channel;
	* i2c_addr = (i2c_mux_addr << 16) + 0x00000004;
	NB_FPGA_CMD_DELAY;
	}
	else {
	return 1;
	}
	
	if(nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE) {	 
	nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].w_data   = w_data;
	nb_fpga_mmap_addr->nba810_i2c_data[i2c_number].reg_addr = Reg_addr;
	NB_FPGA_CMD_DELAY;
	* i2c_addr = 0x00500002;
	
	NB_FPGA_CMD_DELAY;
	}
	else {
		return 1;
	}



    	if( nba810_i2c_status_check(i2c_number) == NB_FPGA_STATUS_DONE) {	
  	// 2.write 0x0004 Register address
  			return 0;
  	} 
  	else {
			return 1;
	}

}

/*
int nb_fpga_nba810_led(int led_port, u_int32_t data)
{
	u_int32_t offset = (u_int32_t(led_port/4)*4 +  0x0000000c; 				

	u_int32_t * addr = (u_int32_t *) ((u_int8_t *)nb_fpga_mmap_addr + NBA810_LED_START_ADDR + offset);
	u_int32_t original_data = *addr;
	nb_fpga_print ("Original data = %x\n",original_data);
	u_int32_t led_x_data;
	
	if(led_port % 4 == 0) {
	led_x_data = (original_data & 0xFFFFFF00) + ((u_int32_t)(data));
	}else if(led_port % 4 == 1){
	led_x_data = (original_data & 0xFFFF00FF) + ((u_int32_t)(data<<8));
	}else if(led_port % 4 == 2){
	led_x_data = (original_data & 0xFF00FFFF) + ((u_int32_t)(data<<16));
	}else if(led_port % 4 == 3){
	led_x_data = (original_data & 0x00FFFFFF) + ((u_int32_t)(data<<24));
	}else{	
	}

//----------------------------------------------------------
	int raa;
//----------------------------------------------------------	
//	nb_fpga_print ("led_x_data = %x\n",led_x_data);
	
	// 1.read 0x0008  check status
	if( nba810_led_status_check() == NB_FPGA_STATUS_DONE) {	
    	// 2.write 0x0004 Register address
    	
	if(led_port== 0) {
	raa = nb_fpga_write_(0x00000a0c,data);
	}else if(led_port== 1){
	raa = nb_fpga_write_mem32(0x00000a10,data);
	}else if(led_port== 2){
	raa = nb_fpga_write_mem32(0x00000a14,data);
	}else if(led_port== 3){
	raa = nb_fpga_write_mem32(0x00000a18,data);
	}else{	
	}	
	
    	NB_FPGA_CMD_DELAY;
	raa = in_fpga_write_mem32(0x00000a00,0x00000001);	
	// 3.write 0x0000 i2c_addr and Control
  		NB_FPGA_CMD_DELAY;
  	} else {
			return 1;
	  }
    
		// 4.read  0x0008 check status
	if( nba810_led_status_check() == NB_FPGA_STATUS_DONE) {	 
			return 0;
  	} else {
			return 1;
	  }
    return 0;
}
*/
//====================================================================================

