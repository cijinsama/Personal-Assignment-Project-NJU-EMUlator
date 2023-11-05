# ICS2022 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.
[Link](https://github.com/NJU-ProjectN/ics-pa)

The project includes:
> [Link](https://nju-projectn.github.io/ics-pa-gitbook/ics2022/index.html)
> 
> The fundamental way to understand "how a program runs on a computer" is to implement a complete computer system from "zero". The small project (Programming Assignment, PA) of the computer system basic course of the Department of Computer Science and Technology of Nanjing University will propose x86/mips32 /riscv32(64) architecture corresponding subset of the teaching version, guiding students to implement a simplified but fully functional x86/mips32/riscv32(64) emulator NEMU (NJU EMUlator), and finally run the game "Legend of Sword and Fairy" on NEMU ", to allow students to explore the basic principles of "programs running on computers". NEMU was inspired by QEMU and removed a large number of parts that were significantly different from the course content. PA includes a preparation experiment (configuring the experimental environment) and 5 parts Coherent experimental content:
> 
> 
> Turing machine and simple debugger
> 
> von Neumann computer system
> 
> batch processing system
> 
> time sharing multitasking
> 
> Program performance optimization

Follow the instructions below. You can play PAL( an old PC game ) while running Nterm( a simple terminal ) synchronously and get outputing "hello" simultaneously on my implemented emulator.

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

## Download PAL data file
You can download PAL data file at [Link](https://ci-jin.top/Resource/pal-data-new.tar.bz2)

make data dir:
```
cd navy-apps/apps/pal/repo
mkdir data
```
Then upzip datafile to `navy-apps/apps/pal/repo/data/`

After this, your files are supposed to be like:
```
.../ICS/navy-apps/apps/pal/repo
├── AUTHORS -> docs/AUTHORS
├── build
│   └── device
├── data
│   ├── 1.rpg
│   ├── 2.rpg
│   ├── 3.rpg
│   ├── 4.rpg
│   ├── 5.rpg
│   ├── abc.mkf
│   ├── ball.mkf
│   ├── data.mkf
│   ├── desc.dat
│   ├── fbp.mkf
│   ├── fire.mkf
│   ├── f.mkf
│   ├── gop.mkf
│   ├── map.mkf
│   ├── mgo.mkf
│   ├── m.msg
│   ├── mus.mkf
│   ├── pat.mkf
│   ├── rgm.mkf
│   ├── rng.mkf
│   ├── sdlpal.cfg
│   ├── sss.mkf
│   ├── voc.mkf
│   ├── wor16.asc
│   ├── wor16.fon
│   └── word.dat
├── docs
...
```


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

## Run the PAL and Nterm NOW!!!!
```
cd nanos-lite
make ARCH=$ISA-nemu run
```
After the loading animation, you can see the game running while stdout outputing Hello
![runningPAL](./picture/PAL.png)

You can press `F2` to change to Nterm, which is running synchronous with PAL
![runningNterm](./picture/Nterm.png)
You can also press `F1` to swich back.
