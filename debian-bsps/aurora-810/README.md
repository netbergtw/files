# Installing IPMItool
```bash
sudo apt install ipmitool
```

# Installing kernel modules

```bash
git clone https://github.com/netbergtw/files.git
cd files/debian-bsps/aurora-810/nba810
make 
insmod ./nb_platform.ko 
insmod ./nb_ports_ioctl.ko 
insmod ./nb_leds_ioctl.ko 
insmod ./nb_ports_sysfs.ko 
```

```bash
cd files/debian-bsps/aurora-810/nct6775
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
depmod -a
modprobe i2c-nct6775
echo '24c02 0x50' > /sys/bus/i2c/devices/i2c-1/new_device
```
