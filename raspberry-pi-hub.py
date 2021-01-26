"""
	A simple example of sending data from 1 nRF24L01 transceiver to another.
	This example was written to be used on 2 devices acting as 'nodes'.
	"""
import sys
import time
import struct
from RF24 import RF24, RF24_PA_MAX, RF24_2MBPS, RF24_250KBPS




########### USER CONFIGURATION ###########
# See https://github.com/TMRh20/RF24/blob/master/pyRF24/readme.md
# Radio CE Pin, CSN Pin, SPI Speed
# CE Pin uses GPIO number with BCM and SPIDEV drivers, other platforms use
# their own pin numbering
# CS Pin addresses the SPI bus number at /dev/spidev<a>.<b>
# ie: RF24 radio(<ce_pin>, <a>*10+<b>); spidev1.0 is 10, spidev1.1 is 11 etc..

# Generic:
radio = RF24(17, 0) 
################## Linux (BBB,x86,etc) #########################
# See http://nRF24.github.io/RF24/pages.html for more information on usage
# See http://iotdk.intel.com/docs/master/mraa/ for more information on MRAA
# See https://www.kernel.org/doc/Documentation/spi/spidev for more
# information on SPIDEV

# using the python keyword global is bad practice. Instead we'll use a 1 item
# list to store our float number for the payloads sent/received
payload = [0.0]


def slave():
	"""Listen for any payloads and print the transaction
		:param int timeout: The number of seconds to wait (with no transmission)
		until exiting function.
		"""
	radio.startListening()  # put radio in RX mode
	print("now listening")
	radio.printPrettyDetails()
	while True:
		has_payload, pipe_number = radio.available_pipe()
		if has_payload:
			print("received payload")
			recv_buffer = []
			radio.read(recv_buffer)
			print(recv_buffer)


if __name__ == "__main__":
	# initialize the nRF24L01 on the spi bus
	if not radio.begin():
		raise RuntimeError("radio hardware is not responding")
	
	# For this example, we will use different addresses
	# An address need to be a buffer protocol object (bytearray)
	address = [0xBCBCBCBCBC,0xEDEDEDEDED]
	# It is very helpful to think of an address as a path instead of as
	# an identifying device destination
	

	
	# set the Power Amplifier level to -12 dBm since this test example is
	# usually run with nRF24L01 transceivers in close proximity of each other
	radio.setPALevel(RF24_PA_MAX)  # RF24_PA_MAX is default
	
	# set the TX address of the RX node into the TX pipe
	radio.openWritingPipe(0xEDEDEDEDED)  # always uses pipe 0
		
	# set the RX address of the TX node into a RX pipe
	radio.openReadingPipe(1, 0xBCBCBCBCBC)  # using pipe 1
	radio.setPayloadSize(16);
	radio.setChannel(0x80);
	radio.setDataRate(RF24_250KBPS);
	radio.setAutoAck(0);
	radio.disableCRC();
	# for debugging, we have 2 options that print a large block of details
	# (smaller) function that prints raw register values
	# radio.printDetails()
	# (larger) function that prints human readable data
	#radio.printPrettyDetails()
			
	try:
		slave()
	except KeyboardInterrupt:
		radio.powerDown()
		print(" Keyboard Interrupt detected. Exiting...")
		sys.exit()

