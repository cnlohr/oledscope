# CH32V003 USB-Controlled OLED Vectorscope for 128x128 OLED Module

This project uses [rv003usb](https://github.com/cnlohr/rv003usb), a pure-software bit-banged USB stack running on a ch32v003 to turn a 128x128 SPI OLED module into a USB controlled vector (or rather point) scope that can update at around 16kpts/sec!

You can play with an [online emulator here](https://cnlohr.github.io/oledscope/) (or if you have an RV003USB-compatible board and use a lame browser) you can connect it to your computer and play with it live.

Soor (Görg Pflug), from my [Discord Server](https://discord.gg/CCeyWyZ) uncovered some very cool hacking they were able to get in with the SSD1306 controller for OLED displays.  And from that they developed some very cool proof of concept demos like this [vectorscope](https://www.youtube.com/shorts/4UzBADBHos4) on a 64x64 display.

Then, I noticed that newer 128x128 displays were coming out. And Soor(Görg Pflug)'s research went more in the direction of accelerated shape rendering [video](https://www.youtube.com/watch?v=MwNGKHWkvP8) [code](https://github.com/GoergPflug/ssd1306-Hardware3D) but I remained fascinated specifically with vector scopes.

Another thing that came with the newer 128x128 displays was a new controller, the [SSH1107](https://www.displayfuture.com/Display/datasheet/controller/SH1107.pdf).  While it may be possible, with other register setting to use a [SSD1327](https://cdn.sparkfun.com/assets/1/a/5/d/4/DS-15890-Zio_OLED.pdf), this entire repo is written around the SSH1107.

With this knowledge, I decided to set out and see if I could make a USB controlled scope out of this 10 cent processor and $4 screen.  For that story, feel free to read below. But, let's just say it was a success.

![vector scope picture](https://github.com/cnlohr/oledscope/blob/master/docs/picture.jpg?raw=true)

## Does it work at all?

I opted to use SPI because I already had a library that could use SPI with DMA on the ch32v003 because that's how its [WS2812B Drirver](https://github.com/cnlohr/ch32v003fun/blob/master/extralibs/ws2812b_dma_spi_led_driver.h) works. In addition, there are sometimes problems when trying to make I2C go super fast.

I then initially started with [bit banging](https://github.com/cnlohr/oledscope128128/blob/master/earlierwork/basetest/basetest.c#L20) out something to get the OLED display up and running.  Once I had that going, I then did a test to see if I could [use the hardware SPI DMA](https://github.com/cnlohr/oledscope128128/blob/master/earlierwork/hwspitest/hwspitest.c#L172-L202) to output data to the display.  And it turned out I could.

So far so good. But, this still required an interrupt to feed the SPI DMA, or extremely fast updating of the main SPI DMA to keep everything going.  This isn't possible when the ch32v003 us doing software USB.

So, the next logical step was to see if it was possbile to use [another DMA to feed the SPI DMA](https://github.com/cnlohr/oledscope128128/blob/master/earlierwork/hwspidma/hwspidma.c#L284-L321) by using DMA Channel 2, triggered off of a timer to update the memory address the SPI DMA would pull from for outputting to the display. And as it turns out, yes! You can! As long as you don't set the mem2mem bit.  It's a little strange to use memory "as" a peripheral, but it worked.

Now it was possible to load up a buffer with commands that I wanted executed at specific times.  Then, in the time between each pixel update, I could modify the memory that would be used to send the next pixel.

Finally comes the ultimate test - with all this DMA going on, would it be possible to also run USB.  And it turns out YES! You can! So long as the DMA priority is set to low.  It doesn't seem to interfere with the USB bus at all.

From there, I can take in new USB Feature Reports, and queue up the requests (x,y) for each pixel into a queue.  Note that for this queue, if it becomes full, we can NAK the control message which will cause the host to hold off, so we can even have throttling!

This queue can then, at the processor's leisure (in the main loop) be processed from x,y coordnates into the commands that need to get sent to the display to cause the pixel to jump to the right location.

The idea is the processor can look to see if there's room in the destination buffer by looking at the second [DMA COUNT](https://github.com/cnlohr/oledscope/blob/master/usbfirmware/usbfirmware.c#L322), if so, process another coordinate into this buffer.

From there, the DMA engines take over.

On top of all of this, I even found out later with the project that rv003usb can work with WebHID!  (See link in beginning)

And what luck. With WebHID, I added two boxes so I can peek and poke at the registers, and I found out I can instead of blanking, pause the OLED between updates.

## Future work

Well, I remain unconvinced that there isn't a way to get this chip to update **now** instead of waiting for the 16kHz clock.  So, if you have any ideas for new register to peek or poke at, please do! I would love to improve this project.

I also tried writing out two pixels at once on the same line, this does make the display brighter but if you look very close, it makes your eyes feel like you are out of focus, so I decided to keep it slightly dimmer but nice and sharp.  Maybe there's a better way to do this or clean it up.

We blank the OLED display for a tiny window of time before moving a pixel around.  This is to prevent the display from sometimes showing a ghost pixel, which can happen if the display updates in the middle of a set x/y coordinate command.  There's got to be a way around this.  I just couldn't find it.

## Which OLED?

Many of the 128x128 OLED Modules are like this one from waveshare: https://www.waveshare.com/wiki/1.5inch_OLED_Module. 

The specific one I bought was a GME128128-01-SPI from here: https://www.aliexpress.us/item/3256805799076926.html

Connections: 
 * PC6 = SDA(MOSI)
 * PC5 = SCL(SCK)
 * PC4 = DC
 * PC2 = CS
 * PC1 = RST

## TODO

 * Create diagram of how data flows with the DMAs.



