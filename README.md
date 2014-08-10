apollo10
========

Many instructions grabbed from here:
``` http://www.strainu.ro/programming/embedded/programming-for-the-tiva-c-launchpad-on-linux/ ``` 

Install the toolchain.  Ubuntu has an incorrect cross compiler, so make sure
the installed compiler comes from the PPA below

```
sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi=4-8-2014q2-0trusty10
```

Setting up the ICDI
```
echo 'ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", GROUP="users", MODE="0660"' | \
> sudo tee /etc/udev/rules.d/99-stellaris-launchpad.rules
```

Tool chain information

+ GCC Version 4.8.4
+ FreeRTOS Version - 8.0.1
+ OpenOCD 0.9.0-dev





