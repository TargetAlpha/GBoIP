GBoIP
=====

Short for Gameboy over IP.

Designed to teleport the Nintendo Gameboy into the 21st century. GBoIP aims to provide Nintendo's classic handheld console with network capabilities.


At the core is an Atmega8 microcontroller connected to an ENC28J60 ethernet module.
It tunnels the serial multiplayer data from one Gameboy to another over a local area network using the UDP protocol.

With this it will be possible to connect two Gameboys in seperate rooms or over longer distances.

Due to timing restrictions it will probably not be possible to send the data over the internet. At least not on an unmodified Gameboy.
