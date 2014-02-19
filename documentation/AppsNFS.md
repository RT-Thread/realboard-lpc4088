## 功能复杂系统的挑战 ##

对于一些功能复杂的系统，开发起来会很有挑战性，因为其功能复杂，所以相应的代码、功能也组件变多、变得庞大：

* 编译时间长；
* 烧写时间长；
* 占用体积过大，片内Flash已经不能放下；

自RT-Thread 1.0.0以来就支持了应用模块，但是因为内存占用的缘故，一直应用范围不大。而在新的RealBoard 4088平台上，因为携带32MB SDRAM，内存绰绰有余，所以可以安心的使用应用模块了，复杂系统的一些问题也都可以比较好的得以解决：

* 把系统划分成固件（Firmware）和应用（Applications）；
* 固件提供基本的功能，放于芯片Flash中；
* 应用提供业务功能的实现，可以加载到SDRAM中执行；

## 搭建RT-Thread应用开发环境

RT-Thread的应用开发环境类似的也分成两类：

**固件**

固件的开发环境和传统的RT-Thread RTOS开发环境类似，没有太多的不同，但是需要注意的是以下几点：

* 使能应用模块支持（`RT_USING_MODULE`），这样才能够把内核的API导出到应用环境中；
* [可选]使能网络及NFS网络文件系统（`RT_USING_DFS_NFS`），这样可以使用NFS网络文件系统直接访问开发主机上的文件系统，或者说加载开发主机上的应用程序，并运行起来；

rtconfig.h中的配置如下：

```
// <bool name="RT_USING_MODULE" description="Using Application Module" default="true" />
#define RT_USING_MODULE

// <bool name="RT_USING_DFS_NFS" description="Using NFS" default="false" />
#define RT_USING_DFS_NFS
// <string name="RT_NFS_HOST_EXPORT" description="The exported NFS host path" default="192.168.1.10:/" />
#define RT_NFS_HOST_EXPORT	"192.168.137.1:/"
```

其中`RT_NFS_HOST_EXPORT`指示出NFS Server的位置。建议使用github中的[realboard][1]工程来作为RealBoard 4088的固件平台，因为它的配置全面，支持的组件也最多。

**应用**

在RealBoard 4088的[github仓库][2]里，已经准备了一些应用的例子，同时也包括基本的应用开发环境。当前开发RT-Thread应用，还必须使用GNU GCC编译器，Windows/Linux上可以安装[GNU Tools for ARM Embedded Processors][3]的工具链。当然这里也假设默认已安装了RT-Thread用到的[scons][4]构建系统。

在RealBoard 4088的rtthread_apps目录下，还需要修改其中的配置文件(rtthread_apps/rtconfig.py)以设置正确的目录，其中包括：

    # RT-Thread root directory
    RTT_ROOT = r'Z:\github\rt-thread'
    
    # toolchains
    EXEC_PATH = r'C:\Program Files (x86)\GNU Tools ARM Embedded\4.6 2012q2\bin'

RTT_ROOT指向RT-Thread的根目录，建议从[RT-Thread github仓库][5]中取出最新版本。EXEC_PATH指向你本地安装的GNU GCC工具链安装位置目录。

在编译应用时，可以在rtthread_apps目录下执行如下的命令行：

    scons --app=hello

这个命令将会编译rtthread_apps下的hello子目录的应用程序。我们可以看看一个简单的RT-Thread应用是什么样的：

```
#include <rtthread.h>

int main(int argc, char** argv)
{
    rt_kprintf("Hello World!\n");
    return 0;
}
```

一个RT-Thread应用是一个携带`int main(int argc, char** argv)`主函数的代码集合（可以是一个main.c文件，也可以是数个.c甚至是.cpp文件）。另外，也需要一个SConscript文件来进行编译，这个SConscript文件非常简单：
```
from building import *

src = Glob('*.c') + Glob('*.cpp')
cwd = GetCurrentDir()

CPPPATH = [cwd]
group = DefineGroup('', src, depend = [''], CPPPATH=CPPPATH)

Return('group')
```

这个SConscript文件把应用当前目录下所有的C文件、C++文件都编译在一起，而生成的应用名字则是这个目录的名字。

## 搭建NFS网络文件系统

首先需要说明的是，NFS网络文件系统并不是必须的，但具备NFS网络文件系统后，可以把开发主机的目录挂接到设备上，在设备上相当于访问本地文件系统一样去访问开发主机上的文件、目录。

在Linux下，可以使用Linux自带的NFS server来进行，网上有很多这类的教程，可以根据您使用的Linux发行版搭建这样的NFS server。

在Windows上，有一款[FreeNFS][6]的小软件可以提供NFS server的功能，可以在[这里][7]直接下载并运行，它是绿色软件。运行它，会提示你NFS的根目录，这个也是RB4088上远程访问的目录，建议设置到RB4088代码中的bin目录，如下图所示：

![FreeNFS][8]

## 运行应用程序

当把realboard的固件下载到RB4088开发板后，就可以从RB4088的命令行中运行你的应用程序了。假设你已经把你的hello.mo应用程序放在software/bin目录下，同时主机上已经打开了FreeNFS（可以先把主机的防火墙先行关闭），可以在RB4088的命令行下运行：

    finsh > mountnfs
    finsh > hello

  [1]: https://github.com/RT-Thread/RealBoard4088/tree/master/software/realboard
  [2]: https://github.com/RT-Thread/RealBoard4088/tree/master/software/rtthread_apps
  [3]: https://launchpad.net/gcc-arm-embedded
  [4]: http://www.scons.org
  [5]: https://github.com/RT-Thread/rt-thread
  [6]: http://freenfs.sourceforge.net/
  [7]: http://sourceforge.net/projects/freenfs/files/latest/download
  [8]: image/FreeNFS.png "FreeNFS"
