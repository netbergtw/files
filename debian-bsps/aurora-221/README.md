# Installing lm-sensors
```bash
sudo apt install lm-sensors
```

# Installing kernel modules

```bash
git clone https://github.com/netbergtw/files.git
cd files/debian-bsps/aurora-221/nba221
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
depmod -a
```
