# Introduction #

These are the examples of RT-Thread APPs for RealBoard4088. All of them are the module applications of RT-Thread RTOS. RT-Thread RTOS loads these user applications and running in **RAM** space.

A simple user application can be a main function, for example the `hello` program:

    #include <rtthread.h>
    
    int main(void)
    {
        rt_kprintf("Hello World!\n");
        return 0;
    }

## Requirements ##

RT-Thread APPs only support GNU GCC toolchain so far. Therefore, please install [ARM GNU GCC toolchain][1] firstly and [scons][2] is used for APPs building. 

## Prepare Env in Firmware ##

Because we need the RT-Thread header files to build APPs, we should tell scons how to find the include pathes. In RT-Thread 2.0, we prepare a new config file, which named as rtua.py. We can use following command in BSP folder to generate this rtua.py file:

    scons --target=ua -s

## Build APPs ##

Please configure your toolchain in `apps/rtconfig.py` file:

    # toolchains
    EXEC_PATH = r'C:\Program Files (x86)\GNU Tools ARM Embedded\4.6 2012q2\bin'

    # BSP folder, please realboard (the rtua.py file should be in there)
    BSP_ROOT = cwd + '/../rtthread_examples/examples/realboard'

Then build APPs by scons, for example the `hello` APP:

    scons --app=hello

The executable file (*hello.mo*) is placed under `hello` folder.

## Run APPs ##

Before running the APP, please place the APPs file under `/bin` of RT-Thread file system by copying to your SD card or using NFS file system in the RT-Thread RTOS.

It's better to use msh (module shell) to run APPs in RT-Thread RTOS, for example using this project: `rtthread_examples\examples\2_filesystem_msh`.

In the RT-Thread module shell, use following command to run `hello` program:

     \ | /
    - RT -     Thread Operating System
     / | \     1.2.0 build Jan  3 2014
     2006 - 2013 Copyright by rt-thread team
    finsh />File System initialized!
    
    finsh />hello
    finsh />Hello World!

  [1]: https://launchpad.net/gcc-arm-embedded
  [2]: http://www.scons.org/

