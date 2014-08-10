apollo10
========

Many instructions grabbed from here:
``` http://www.strainu.ro/programming/embedded/programming-for-the-tiva-c-launchpad-on-linux/ ``` 

Install the toolchain

```
sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi
```

Setting up the ICDI
```
echo 'ATTRS{idVendor}=="1cbe", ATTRS{idProduct}=="00fd", GROUP="users", MODE="0660"' | \
> sudo tee /etc/udev/rules.d/99-stellaris-launchpad.rules
```


