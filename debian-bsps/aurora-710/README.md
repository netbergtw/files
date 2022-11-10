# Installing i2c_utils.sh
```bash
 wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-710/i2c_utils.sh -O /usr/sbin/i2c_utils.sh
 chmod +x /usr/sbin/i2c_utils.sh
```
# Init the switch
```bash
 apt install --install-recommends linux-generic-hwe-20.04
 git clone https://github.com/sonic-net/sonic-buildimage.git --branch master --single-branch
 cd sonic-buildimage/platform/barefoot/sonic-platform-modules-netberg/aurora-610/modules/
```

Since this you can make switch initialization:
```bash
 i2c_utils.sh i2c_init
```
There are a lot of usefull i2c_utils.sh options.
Just run it without parameters to show them all.

= Example of a simple systemd monitoring services
```bash
 wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-710/aurora-710-monitor.service -O /etc/systemd/system/aurora-710-monitor.service

 wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-710/aurora_710_monitor.sh -O /usr/sbin/aurora_710_monitor.sh
 chmod +x /usr/sbin/aurora_710_monitor.sh

 systemctl enable aurora-710-monitor
```
This script will automatically start during system boot 
and run i2c_init and will control leds.

