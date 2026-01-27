esptool \
	--chip esp8266 \
	--port /dev/ttyUSB0 \
	--baud 115200 \
	erase-region 0xFC000 0x4000

esptool \
	--chip esp8266 \
	--port /dev/ttyUSB0 \
	--baud 115200 \
	write-flash 0x0 wifi2c-fw.bin
