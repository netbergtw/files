# Installing IPMItool
```bash
sudo apt install ipmitool
```

# Installing kernel modules

```bash
git clone https://github.com/netbergtw/files.git
cd files/debian-bsps/aurora-621/nba621
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
depmod -a
```
