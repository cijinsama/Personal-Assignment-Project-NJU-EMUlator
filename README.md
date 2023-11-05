# ICS2021 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to http://nju-ics.gitbooks.io/ics2021-programming-assignment/content/

To initialize, run
```bash
bash init.sh subproject-name
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

# How to use

## Prepare
clone this repo
```
git clone git@git.nju.edu.cn:Ci_Jin/ICS.git .
```

install libaray
```
apt-get install build-essential    # build-essential packages, include binary utilities, gcc, make, and so on
apt-get install man                # on-line reference manual
apt-get install gcc-doc            # on-line reference manual for gcc
apt-get install gdb                # GNU debugger
apt-get install git                # revision control system
apt-get install libreadline-dev    # a library used later
apt-get install libsdl2-dev        # a library used later
apt-get install llvm llvm-dev      # llvm project, which contains libraries used later
apt-get install bison
apt-get install flex
apt-get install riscv64-linux-gnu-g++
apt-get install g++-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

set global variable: add into your `~/.zshrc` or `~/.bashrc`
```
export NEMU_HOME='/home/cijin/Code/ICS/nemu'
export AM_HOME='/home/cijin/Code/ICS/abstract-machine'
export NAVY_HOME='/home/cijin/Code/ICS/navy-apps'
export ISA='riscv32'
```



## Init NEMU
```
cd nemu
make menuconfig
```
when making the config, enter `Testing and Debugging`. Disable the tracer for performance(Or it will run SUPER slow). Enable the `Device`. Enter the device and disable the audio.

Save the config

## Init Navy-app
```
cd navy-apps
make ISA=$ISA ramdisk
```

## Init Nanos-lite
```
cd nanos-lite
make ARCH=$ISA-nemu update
```
