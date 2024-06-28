# Installing i2c_(de)init.sh
```bash
 sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-610/i2c_init.sh -O /usr/sbin/i2c_init.sh
 sudo chmod +x /usr/sbin/i2c_init.sh
 sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-610/i2c_deinit.sh -O /usr/sbin/i2c_deinit.sh
 sudo chmod +x /usr/sbin/i2c_deinit.sh
```
# Init the switch
Install and build platform kernel modules
```bash
 git clone https://github.com/sonic-net/sonic-buildimage.git --branch master --single-branch
 cd sonic-buildimage/platform/barefoot/sonic-platform-modules-netberg/aurora-610/modules/
 make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
 sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
 sudo depmod -a
```

Since this you can make switch initialization:
```bash
 sudo i2c_init.sh
```

# Example of a simple systemd init services
```bash
 sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-610/aurora-610.service -O /etc/systemd/system/aurora-610.service

 sudo systemctl enable aurora-610
```
This script will automatically start during system boot,
load specific modules and initialize i2c devices.
