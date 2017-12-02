# linux sysfs driver sample
Really easy sample implementation for linux driver using sysfs,
you can only get the essense of basic information.

### How to build and install
```bash
make
sudo insmod sysfs.ko
lsmod | grep sysfs
sudo rmmod sysfs.ko
```
### How to use
```bash
echo "orehatensaida" > /sys/kernel/kset_hoge/hoge/genius
echo "orehabakada" > /sys/kernel/kset_hoge/hoge/stupid 

cat /sys/kernel/kset_hoge/hoge/genius
from genius orehatensaida

cat /sys/kernel/kset_hoge/hoge/stupid
from stupid orehabakada
```
