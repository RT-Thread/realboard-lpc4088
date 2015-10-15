# 例程说明

这个是一份使用板载四线SPI flash的说明。它会把一些代码放到板载外置的SPI flash上，当然也可以直接在SPI flash上执行代码。

## 关键点说明

请注意rtthread-lpc40xx.sct链接脚本，它指定了一些文件在最终的链接位置。当片内flash不够时，可以把一些文件挪动到SPI Flash上。
