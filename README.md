# CH32V003 USB-Controlled OLED Vectorscope for 128x128 OLED Module

Using [rv003usb](https://github.com/cnlohr/rv003usb) 

## Story

Soor, from my [Discord](https://discord.gg/CCeyWyZ) uncovered some very cool hacking they were able to get in with the SSD1306 controller for OLED displays.  And from that they developed some very cool proof of concept demos like this [vectorscope](https://www.youtube.com/shorts/4UzBADBHos4) on a 128x96 display.

Then, I noticed that newer 128x128 displays were coming out.  And Soor's research went more in the direction of [accelerated shape rendering](https://www.youtube.com/watch?v=MwNGKHWkvP8) but I remained fascinated specifically with vector scopes.

Another thing that came with the newer 128x128 displays was a new controller, the [SSD1327](https://cdn.sparkfun.com/assets/1/a/5/d/4/DS-15890-Zio_OLED.pdf) controller chip.  While the documentation made it abundantly clear that the chip could not use a 0 mux ratio, setting the register anyway showed that to be a lie.

With this knowledge, I decided to set out and see if I could make a USB controlled scope out of this 10 cent processor and $4 screen.

### Does it work at all?

I opted to use SPI because I already had a library that could use SPI with DMA on the ch32v003 because that's how its [WS2812B Drirver](https://github.com/cnlohr/ch32v003fun/blob/master/extralibs/ws2812b_dma_spi_led_driver.h) works. In addition, there are sometimes problems when trying to make I2C go super fast.

I then initially started ..
[TODO PICK UP HERE]

### Future work

Well, I remain unconvinced that there isn't a way to get this chip to update **now** instead of waiting for the 16kHz clock.  So, if you have any ideas for new register to peek or poke at, please do! I would love to improve this project.

## Which OLED?

Many of the 128x128 OLED Modules are like this one from waveshare: https://www.waveshare.com/wiki/1.5inch_OLED_Module. 

The specific one I bought was a GME128128-01-SPI from here: https://www.aliexpress.us/item/3256805799076926.html

It uses a SSD1327.




