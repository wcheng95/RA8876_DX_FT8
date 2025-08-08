RA8876_DX_FT8  June 11, 2025

Background
Several years ago (June 2023) I put together a version of our FT 8 code based on the RA8876 Display.
The RA887 Display Code and Interface Board were plagiarized from this site: https://oshwlab.com/k7mdl/v2-1_7-inch_teensy_pcb

Also, the base FT8 Encoding / Decoding Algorithms are based on work by Karlis Goba:
https://github.com/kgoba/ft8_lib

As you will see this version made use of the 1024 X 600 Display to show locations of worked FT8 Stations on a World Map.  https://www.buydisplay.com/spi-7-inch-tft-lcd-dislay-module-1024x600-ra8876-optl-touch-screen-panel

For whatever reason I can not remember why I put this work aside.

Now with the success with the DX_FT8 project I have revived my interest in this project.

So I designed and had  fabricated a board which works with either a DX-FT8 Five Band
or Seven Band Board with the only modification of replacing a user fabricated audio interface cable
with a simple three pin header pin for a direct connection.

I was pleased that the five boards that were fabricated by JCLPCB required no modification to work with my DX_FT8 boards. Please use this link for board design and ordering details:

Here is a view of the current project  software taken while I was operating portable in Cornwall, UK
![image](https://github.com/user-attachments/assets/f26551ef-ac85-4fd2-af9b-fd68c74b24ef)

Here is a link to the EasyEDA Board Schematic and PCB Design Drawing:
https://oshwlab.com/chillmf20/v2-1_7-inch_teensy_pcb_copy_copy_copy

Version 1.2

In this version the device time can be synchonised to the Internet and reception reports uploaded to the PskReporter at https://www.pskreporter.info/
This requires an external ESP32 module
Please see https://github.com/g8kig/DX-FT8-TimeSync_PSKReporter for more information about constructing and programming the simple module that can be used with this transceiver
