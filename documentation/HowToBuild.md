# 如何编译RealBoard 4088例程 #

RealBoard 4088 git repo带的都是LPC4088相关的例程，本身并不携带RT-Thread及其GUI的代码，所以在编译RT-Thread相关的例程前，需要准备相关的代码。

## 准备RT-Thread代码 ##

RT-Thread相关的例程，通常可以使用RT-Thread的数个版本，可以方便的来回切换。包括RT-Thread 1.2.0/1.2.1和RT-Thread 2.0.0系列。（不同的系列可能在RT-Thread/GUI的使用上有稍许不同，所以建议使用较新版本的RT-Thread来适配最新的RT-Thread/GUI）

在下载了RT-Thread代码后，需要在自己的机器上把RTT_ROOT环境变量设置到RT-Thread的根目录。同时请注意下这些路径不要使用包括中文字符的路径。


## 准备RT-Thread/GUI代码 ##

因为目前RT-Thread/GUI组件已经独立于RT-Thread进行开发，并且许可证也有不同，所以在使用GUI相关例程前，需要把GUI的代码下载下来，并把/components/rtgui目录的rtgui目录放置到RT-Thread/components目录下。


## 按照Python 2.7.x & scons ##

这部分参见 ...

## 编译 ##

当上述工作都准备了时，可以在RealBoard 4088各个例程目录下用scons来编译，也可以使用如下命令行：

    scons --target=mdk -s
    
来生成MDK的工程文件；如果需要使用IAR编译器，也可以使用如下命令行：

    scons --target=iar -s 
    
注：IAR工程文件可能缺少工程模版文件，即一个名称为template的IAR空工程文件。用户可以自己提供。
