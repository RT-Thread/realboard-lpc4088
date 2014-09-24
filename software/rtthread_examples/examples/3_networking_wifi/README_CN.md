# RealBoard 4088的WiFi例程 #

本例程需要在原有RB4088开发板基础上额外使用一个模块：RW009，在中国大陆可以从这里购买到：
[淘宝链接][1]

使用这种方式，运行在RB4088里的RT-Thread会把RW009作为一个网卡设备来使用，即使用RT-Thread内部的TCP/IP协议栈，依然使用BSD socket的方式进行软件编程开发。

## Wifi硬件连接 ##
硬件连接方式请使用如下的连接方式：
```
  WIFI_VCC  <---> 3.3V
  WIFI_MOSI <---> P2_27
  WIFI_MISO <---> P2_26
  WIFI_SCK  <---> P2_22
  WIFI_CS   <---> P2_24
  WIFI_RST  <---> P2_21
  WIFI_IRQ  <---> P2_25
  WIFI_GND  <---> GND
```

## Wifi软件配置 ##
使用WiFi时，需要关联到一个AP上，需要在代码中进行少些修改以把SSID和密码填写正确。打开applications/application.c，修改rw009_join的参数为需要连接的AP名称和密码。

```
    rw009_join("you_AP", "you_passwd");
```

[1]: http://item.taobao.com/item.htm?id=40813298723
