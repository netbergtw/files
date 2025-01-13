#!/bin/bash
# Netberg Aurora 221 basic init 

GPIO_OFFSET=0

function set_gpio_offset {
    GPIO_OFFSET=0
    gpio_base_min=`cat /sys/class/gpio/gpiochip*/base | sort -nr | head -1`
    GPIO_OFFSET=$(($gpio_base_min))
    echo "set GPIO_OFFSET=${GPIO_OFFSET}"
}

#Load modules
modprobe i2c-mux-pca954x

modprobe optoe

modprobe pmbus
modprobe lm75
modprobe ucd9000

#Init CPU_HOST_I2C
echo pca9546 0x72 > /sys/bus/i2c/devices/i2c-0/new_device 
echo '-2' > /sys/bus/i2c/devices/0-0072/idle_state

echo pca9546 0x70 > /sys/bus/i2c/devices/i2c-0/new_device
echo '-2' > /sys/bus/i2c/devices/0-0072/idle_state

#SYSEEPROM
echo 24c32 0x56 > /sys/bus/i2c/devices/i2c-0/new_device

# PS EEPROM
echo spd 0x50 > /sys/bus/i2c/devices/i2c-2/new_device #PS0
echo spd 0x51 > /sys/bus/i2c/devices/i2c-2/new_device #PS1

echo "EEPROM read:"
sleep 1
hexdump -C -n160 /sys/bus/i2c/devices/0-0056/eeprom
hexdump -C -n64 -s12  /sys/bus/i2c/devices/2-0050/eeprom
hexdump -C -n64 -s12  /sys/bus/i2c/devices/2-0051/eeprom

# PS PMBUS
echo pmbus 0x58 > /sys/bus/i2c/devices/i2c-2/new_device #PS0
echo pmbus 0x59 > /sys/bus/i2c/devices/i2c-2/new_device #PS1

echo tmp75 0x4A > /sys/bus/i2c/devices/i2c-3/new_device 
echo tmp75 0x49 > /sys/bus/i2c/devices/i2c-3/new_device 

echo ucd90124 0x34 > /sys/bus/i2c/devices/i2c-5/new_device

#Init CPU_LEG_I2C
## SFP SW
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-1/new_device
echo '-2' > /sys/bus/i2c/devices/1-0071/idle_state

##SFP EEPROMs
echo -n "SFP28 EEPROMs"
for I in {10..17}; 
do 
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-$I/new_device
    echo "port$(($I+38))" > /sys/bus/i2c/devices/$I-0050/port_name
    echo -n '.'
done
echo "done"

modprobe gpio-pca953x

#MOD_ABS_0_7
echo pca9535 0x20 > /sys/bus/i2c/devices/i2c-6/new_device
set_gpio_offset
for((i=${GPIO_OFFSET};i<${GPIO_OFFSET}+8;i++));
do
    echo $i > /sys/class/gpio/export
done

#RS_TXDIS_0_7
echo pca9535 0x21 > /sys/bus/i2c/devices/i2c-7/new_device
set_gpio_offset
for((i=${GPIO_OFFSET}+8;i<${GPIO_OFFSET}+16;i++));
do
    echo $i > /sys/class/gpio/export
    echo out > /sys/class/gpio/gpio${i}/direction
    echo 0 > /sys/class/gpio/gpio${i}/value
done

#9535_TXFLT_RXLOS_0_7
echo pca9535 0x22 > /sys/bus/i2c/devices/i2c-8/new_device
set_gpio_offset

for((i=${GPIO_OFFSET};i<${GPIO_OFFSET}+16;i++));
do
    echo $i > /sys/class/gpio/export
done

#FAN_DIR
echo tca9554 0x21 > /sys/bus/i2c/devices/i2c-3/new_device
set_gpio_offset

modprobe aurora-221-cpld
echo aurora-221-cpld 0x33 > /sys/bus/i2c/devices/i2c-0/new_device
