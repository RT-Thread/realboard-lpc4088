# Wifi硬件连接 #
WIFI_CS  <---> P2_24
WIFI_IRQ <---> P2_25
WIFI_MO  <---> P2_27
WIFI_SO  <---> P2_26
WIFI_SCK <---> P2_22

# Wifi软件配置 #
打开applications/spi_wifi.c，修改SSID_NAME和SSID_PASSWORD宏定义为需要连接的路由器名称和密码。