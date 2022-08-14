# Using RT-Thread Studio To Control ESP32 With STM32F469 Disco
![](https://hackster.imgix.net/uploads/attachments/1481209/ezgif_com-gif-maker_q5se8jLKAV.gif?auto=format%2Ccompress&gifq=35&w=900&h=675&fit=min&fm=mp4)
## Things used in this project
### Hardware components
STM32F469 DISCOVERY
Espressif ESP32 Development Board
Speaker, Mini
### Software apps and online services
RT-Thread IoT OS
STMicroelectronics STM32CubeMX
Microsoft VS Code
Engines of Our Ingenuity
ezgif.com
remove.bg
LVGL
### Hand tools and fabrication machines
Soldering iron (generic)
Solder Wire, Lead Free
## Story
### Background
Long before TED Talks became popular, there was a radio program that tells the story of how our culture is formed by human creativity. That radio program, [Engines of Our Ingenuity](https://www.uh.edu/engines/), has been featured in one of my previous projects, [If The Allegations Are Not Legit, You m. AWS. t EduKit](https://www.hackster.io/abouhatab/if-the-allegations-are-not-legit-you-m-aws-t-edukit-326d38). We wanted to make a device that plays episodes with more screen space to allow for user control and for that we used STM32F469 DISCOVERY board. We didn't just want random and forward/backward navigation, we wanted to be able to jump to specific episodes and see the title and description. Also, in this project we did not use AWS so we had to figure out how to make ESP32 load large pages without crashing. Lastly, the boards had to communicate over UART.
### Challenges
![](https://hackster.imgix.net/uploads/attachments/1481064/20220711_153034_82DaNzqIs1.jpg?auto=compress%2Cformat&w=1280&h=960&fit=max)
Our mail carrier used my board as booster seat. Miraculously, board still in one piece. The I2S amplifier that I ordered weeks ago has not been delivered yet. For that reason we used code from [Skydive Auto Reminder - Doing More With Less](https://www.hackster.io/abouhatab/skydive-auto-reminder-doing-more-with-less-37abd9) to produce a lower quality audio without I2S board.
### Board Support Package (BSP)
We started by downloading and extracting [https://github.com/RT-Thread/rt-thread/archive/refs/heads/master.zip](https://github.com/RT-Thread/rt-thread/archive/refs/heads/master.zip) to get the latest BSP. According to the [User Manual](https://www.st.com/resource/en/user_manual/um1932-discovery-kit-with-stm32f469ni-mcu-stmicroelectronics.pdf), we have four USART ports and, four UART ports on that board, however, only USART3 is enabled by default. Using STM32 CubeMX, we opened %userprofile%\Downloads\rt-thread-master\rt-thread-master\bsp\stm32\stm32f469-st-disco\board\CubeMX_Config\CubeMX_Config.ioc

![](https://hackster.imgix.net/uploads/attachments/1481085/image_tAvWWdWakd.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

After clicking Continue, we expanded connectivity on the left pane and clicked USART6.

![](https://hackster.imgix.net/uploads/attachments/1481089/image_MxuH4y31f8.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We replaced Disabled with Asynchronous in the Mode drop down menu. We then clicked Generate Code in the top right corner and closed the the application.

![](https://hackster.imgix.net/uploads/attachments/1481090/image_cDQQbvR1oC.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We located Kconfig (couple of folders up) and duplicated UART3 text then replaced with UART6\. Now, it looks like this.

![](https://hackster.imgix.net/uploads/attachments/1481092/image_nFmpfJZm4Q.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

BSP is ready to be used after saving this file.
### RT-Thread Studio
One minute into this tutorial shows how to import BSP that we just modified.

https://www.youtube.com/watch?v=fREPLuh-h8k

This is how our import looked.

![](https://hackster.imgix.net/uploads/attachments/1481175/image_trIpqYXFuw.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We opened RT-Thread Settings from the left pane then clicked on the chevrons on the right.

![](https://hackster.imgix.net/uploads/attachments/1481176/image_K7s110qSbx.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We came here to Enable LVGL for LCD which will automatically trigger all the needed toggles.

![](https://hackster.imgix.net/uploads/attachments/1481177/image_A7KtijgmOW.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We needed to also turn on UART6 we just enabled.

![](https://hackster.imgix.net/uploads/attachments/1481178/image_AKg1p3Xlnd.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We had to increase Rx buffer size from 64 to 1024.

![](https://hackster.imgix.net/uploads/attachments/1481179/image_OF2vNiZmIU.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

We deleted all files in applications folder then copied our files from Github.

![](https://hackster.imgix.net/uploads/attachments/1481180/image_O8yZvqLVcb.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

To allow auto reset after code download we needed to make this change.

![](https://hackster.imgix.net/uploads/attachments/1481182/image_8L5JmhzzcD.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

Please note that this tutorial is using LVGL v8.3.1 and if you have to look something up on their website, make sure you are looking at the correct version.

![](https://hackster.imgix.net/uploads/attachments/1481082/image_qYq7QjzPEX.png?auto=compress%2Cformat&w=1280&h=960&fit=max)

### Visual Studio Code
We used [mrfaptastic/WiFiConnect Lite](https://github.com/mrfaptastic/WiFiConnectLite) to connect ESP32 to Wi-Fi without hard coding credentials. We also used [earlephilhower/ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) to make the board translate mp3 files to audio. Our board was too old so we had to comment out the same three lines from AudioOutputI2S.cpp and AudioOutputSPDIF.cpp.

    .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT, // Unused
    .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT // Use bits per sample
    .mck_io_num = 0, // unused

In the setup we connect to a Wi-Fi and hit the website for the first time to get the latest episode number to be used in generating a random number for initial play. In the loop we go to the episode page to retrieve the title and description then start playing the audio. If a user sends a request for a different episode, the loop will be interrupted and the new episode will play.
### Hardware
We used the standard UART connection to make the boards communicate. Tx of one board is connected to Rx of the other and vice versa. We are powering ESP32 with USB cable, and powering STM32F469 DISCOVERY from ESP32\. NRST of STM32F469 DISCOVERY had to be connected to EN of ESP32 to keep the boards in synch. Finally, the audio output comes from D22 and we had to use a 0.01 uf (103K) Ceramic Disc Capacitor to remove some of the noise.
### Wiring
![](https://hackster.imgix.net/uploads/attachments/1481293/wiring_1ZTcNEUHUR.jpg?auto=compress%2Cformat&w=1280&h=960&fit=max)
### Demo
https://www.youtube.com/watch?v=1Ht8mKULjwo
