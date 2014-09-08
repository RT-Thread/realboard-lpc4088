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
使用WiFi时，需要关联到一个AP上，需要在代码中进行少些修改以把SSID和密码填写正确。打开applications/spi_wifi.c，修改SSID_NAME和SSID_PASSWORD宏定义为需要连接的路由器名称和密码。
因AP加密类型不同，可能还需要修改第392行的加密方式，有各下多种可自由组合。AP的加密方式一般可以进入AP（路由）管理界面查看。
```
  cmd_join->security = WPA_SECURITY | TKIP_ENABLED | TKIP_ENABLED;
  cmd_join->security = WPA2_SECURITY | TKIP_ENABLED | AES_ENABLED;
```

[1]: http://item.taobao.com/item.htm?id=40813298723
