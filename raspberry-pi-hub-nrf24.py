#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Example program to receive packets from the radio
#
# João Paulo Barraca <jpbarraca@gmail.com>
#
from nrf24 import NRF24
import time
import sys

pipes = [[0xed, 0xed, 0xed, 0xed, 0xed], [0xbc, 0xbc, 0xbc, 0xbc, 0xbc]]

radio = NRF24()
radio.begin(0, 0, 22,17)

radio.setRetries(15,15)

radio.setPayloadSize(16)
radio.setChannel(125)
radio.setDataRate(NRF24.BR_250KBPS)
radio.setPALevel(NRF24.PA_MAX)

radio.setAutoAck(0)
radio.setCRCLength(NRF24.CRC_DISABLED)
radio.openWritingPipe(pipes[1])
radio.openReadingPipe(1, pipes[0])

radio.startListening()
radio.stopListening()

radio.printDetails()

radio.startListening()

def slave():
	while True:
		pipe = [1]
		while not radio.available(pipe, True):
			time.sleep(1000/1000000.0)
		
		recv_buffer = []
#radio.read(recv_buffer)

#print recv_buffer
try:
	slave()
except KeyboardInterrupt:
	radio.powerDown()
	print(" Keyboard Interrupt detected. Exiting...")
	sys.exit()


