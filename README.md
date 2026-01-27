# wifi2c – network-accessible I2C (TWI) bus master based on ESP-01S

## Overview

The project is simple – it turns an ESP-01S (or possibly any other ESP8266-based board) into a remotely-controlled I2C (TWI) bus master. Board connects to your WiFi network, starts a TCP server on port 2430, listens for commands, and handles the bus.

The project is divided into four parts, which reside in their own directories.

1. **esp** – ESP8266 firmware
2. **lib** – simple C library for Linux systems to communicate with the board
3. **cli** – example command-line (CLI) program using this library
4. **test** – simple Bash script that uses the CLI program to communicate with PCF8574 and perform basic testing of everything

I2C clock, measured as a delay after each signal change, can be set between approximately 10 µs and 2550 µs (2.55 ms), but other factors (like TCP request-response round-trip time) will significantly lower the speed. It's not a fast master for bulk transfers.

What's **NOT** implemented is clock stretching and bus arbitration (multi-master bus), because they would complicate the code and protocol somewhat (to signal failures, like losing arbitration or timeouts during clock stretching) and weren't needed by me.

## Connecting to the PC

To load firmware (but also to set WiFi parameters) you need to connect your ESP-01S to the PC. There are many ways to do it, and here's one recommended by me.

You need a FT232-based USB-to-serial converter that has a jumper to set voltage to 3.3 V. If you have DTR and RTS pins available, you can use them. If not, don't worry.

**Make sure you set the jumper to 3.3 V, or you can damage the board**. It's not 5 V tolerant.

Then, connect it as follows.

| ESP-01S | Converter |
| ------- | --------- |
| GND     | GND       |
| 3V3     | VCC       |
| RX      | TXD       |
| TX      | RXD       |
| RST     | RTS       |
| EN      | -         |
| IO0     | DTR       |
| IO2     | -         |

If you don't have RTS or DTR, read on.

## ESP8266 firmware loading

Before loading the firmware, you have to pull IO0 line low.

If your converter has RTS and DTR lines, it's best to connect DTR to IO0 and RTS to RST. It will make everything nice and automatic.

If your converter lacks these lines, just connect IO0 to GND and then connect the converter to the PC. In this case don't touch RST (leave it unconnected).

If you don't want to modify sources (normal use case), just install *esptool*, go to `esp` directory, and execute: `./flash.sh`. If you do want to modify sources, you can do it with Arduino IDE.

If everything goes well and you used RTS and DTR lines, you'll see a blue LED on the ESP module blinking fast (5 Hz). If you pulled IO0 to GND yourself, then disconnect the converter from the PC (to remove power from the board), disconnect IO0 from GND (leave it unconnected), and reconnect the converter to the PC. A blue LED should start blinking.

## LED function

Blue LED on the ESP-01S module is connected to its IO2 pin. Firmware uses it to communicate its state and assigned IP address.

Fast blinking (5 Hz) – firmware waits for ENTER on the RXD pin to enter configuration (more on it below).

Slow blinking (1 Hz) – firmware is in the configuration state, waiting for WiFi SSID and PSK to be entered.

LED off – firmware is trying to connect to the network.

LED blinks erratically – firmware is connected to the network and IP address has been obtained.

This erratic blinking is actually slow Morse code, with which the board is constantly communicating its IP address. Speed is set to 5 WPM to allow people who don't know Morse code to count slower and faster blinks and use some online decoders to decode them, or the below table. Between each cycle a &lt;BT&gt; prosign is transmitted (`-...-`).

For your reference:

| Char | Morse  |
| ---- | ------ |
| 0    | -----  |
| 1    | .----  |
| 2    | ..---  |
| 3    | ...--  |
| 4    | ....-  |
| 5    | .....  |
| 6    | -....  |
| 7    | --...  |
| 8    | ---..  |
| 9    | ----.  |
| .    | .-.-.- |
| &lt;BT&gt; | -...-  |

For example, if your board has the IP 192.168.2.72, you'll see in Morse:

`192.168.2.72 <BT> 192.168.2.72 <BT> 192.168.2.72 <BT> `…

## Initial WiFi configuration

When board is powered up, it configures the serial port (only RX line) to 115200 bps 8n1 and waits for ENTER. This state is signaled by the fast LED blinking.

If configuration has already been set, it blinks for 5 seconds and starts connecting to the network. If it's not set (it's the first run), it waits indefinitely.

Use a terminal emulator (like *minicom*) to connect to the serial port, but disconnect RTS and DTR lines first, as once terminal emulator opens the port, it will pull these lines low, which will keep the device in a reset state.

Before communicating with the device, make sure to disable flow control.

Once you press ENTER, you will be prompted to enter SSID. Enter it and confirm with ENTER. Then you'll be prompted to enter the network password. Enter it and press ENTER again. Device will save these settings in flash and leave the configuration mode.

There are two limitations at this stage:

1. Only DHCP configuration is supported
2. Only personal networks are supported (no EAP)
3. Only single network configuration is supported at the time

In most cases it shouldn't be a problem.

## WiFi reconfiguration

If you already configured the network, you can press ENTER in five seconds after booting the firmware to enter the configuration menu again. If you don't, the board will continue booting and will try to connect to the network it has saved.

## Connecting the board

Here's the tricky part – firmware reuses RX (IO3) and TX (IO0) lines as I2C lines. RX (IO3) is SCL and TX (IO0) is SDA.

Why not the other way around? It's because ESP8266 writes some diagnostic information to the serial port when it starts. From the I2C point of view, it's just flipping the SDA line while SCL is high due to a pull-up, so it's harmless.

Note that you need pull-ups on the bus. Any value between 1 kΩ and 10 kΩ should be OK.

You need only four lines to use the board as an I2C master – 3.3 V, GND, TX (as SDA) and RX (as SCL). Leave everything else unconnected.

**Remember that the logic here is 3.3 V, so you need level shifters to talk to devices that use 5 V.**

## Building the library

It's extremely simple. Enter `lib` directory and type `make`. If everything goes well and you have some basic packages (like `GNU Make` and `gcc`) installed, two files will be created: `libwifi2c.a` and `libwifi2c.so`. Type `sudo make install` to install `wifi2c.h` in `/usr/local/include` and these two newly created library files in `/usr/local/lib`.

Because the library is installed in the local directory, programs dynamically linked with it might not be able to find it. You can either add `/usr/local/lib` to `/etc/ld.so.conf` (or create a file like `local.conf` in `/etc/ld.so.conf.d`) and call `ldconfig`, or add this directory to `LD_LIBRARY_PATH` environment variable. I prefer the former solution.

One-liner for systems that have `/etc/ld.so.conf.d` directory:

`echo /usr/local/lib | sudo tee /etc/ld.so.conf.d/local.conf && sudo ldconfig`

## Building the CLI program

It's as simple as building the library. Enter `cli` directory and type `make`. A file called `wifi2c-cli` will be created. Again, type `sudo make install` to install the program, this time in `/usr/local/bin`.

## Using the CLI program

When you run `wifi2c-cli` without arguments, it will show help and available commands. If it complains that it can't find `libwifi2c.so` library, then you have to make sure you did the ldconfig magic stated above, or alternatively execute: `LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH wifi2c-cli`.

CLI program will print data read from and written to the bus as hex numbers, appending `!` to them if the byte was not ACK-ed.

Note that when you read data from the bus, you shouldn't ACK the last one (use `rn` command instead of `ra`). Otherwise slave might not release the bus.

## Test script

A simple test script, called `pcf8574.sh`, is present in the `test` directory. You should connect PCF8574 port expander board to the bus, pull its address bits (A0, A1 and A2) high, and put a jumper between P2 and P3 pins, leaving other pins unconnected. Then execute the test script. It will print what it's doing and what is the expected result. Compare the actual result with what's expected.

You don't have to use PCF8574. It's just what I had at hand when testing the board.

## Bus reset

As the bus can be potentially left in various states (mid-transfer, etc.), there's a *bus reset* (`e`) command that attempts to reset the bus. See the source code (`esp/wifi2c-fw/twi.cpp` file, `void Twi::reset()` method) for details.

## Protocol

If for whatever reason you need to write your own library to communicate with the device, here's the protocol description.

Communication protocol is based on TCP/IP. Once you connect to port 2430 (arbitrarily chosen number), device sends:

`wifi2c v01`

*01* is the protocol version number – currently the only version available is 01 (bigger numbers are reserved for features that might be added in the future). Note that there's no newline after this (nor after anything else that is transmitted).

Then you send commands and device echoes them back and executes them. Always wait for the echo before sending new command. If a command results in some data being available (like data in *read* command, or ACK/NAK bit in *write* command), it's returned after the command is echoed back.

Any data that's unrecognized (invalid commands) is just ignored, and is not echoed back.

Here are the commands:

### Keepalive

Used to keep the TCP/IP link open / check if it's still open.

* Command: k
* Response: k

### Clock selection

Clock is just a delay after each port state change, in 10 µs units. Example to set clock to 0xa3 (so 163 × 10 µs = 1.63 ms):

* Command: ca3
* Response: ca3

Note that hex number has to be lowercase. Another example to set the fastest clock:

* Command: c01
* Response: c01

And slowest:

* Command: cff
* Response: cff

You can also use c00 to disable delay, but it's not guaranteed to work reliably.

### Bus reset

Resets the bus (tries to clock out of the unknown bus state). It's good to execute this at the beginning.

* Command: e
* Response: e

### Start

Sends start or repeated start.

* Command: s
* Response: s

### Stop

Sends stop.

* Command: p
* Response: p

### Write

Writes a byte to the slave and receives ACK or NAK.

Example to write the byte 0xbf and receive ACK:

* Command: wbf
* Response: wbfa

Another example – write 0x01, receive NAK:

* Command: w01
* Response: w01n

### Read

Reads a byte from the slave and sends ACK or NAK.

Example to read byte 0xbf and send ACK:

* Command: ra
* Response: rabf

Another example – read 0x01, send NAK:

* Command: rn
* Response: rn01

## Contact

Please report issues using the GitHub bug tracking system.
