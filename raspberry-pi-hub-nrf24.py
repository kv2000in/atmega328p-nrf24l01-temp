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
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
pipes = [[0xed, 0xed, 0xed, 0xed, 0xed], [0xbc, 0xbc, 0xbc, 0xbc, 0xbc]]

radio = NRF24()
radio.begin(0, 0, 25,24)

radio.setRetries(15,15)

radio.setPayloadSize(16)
radio.setChannel(125)
radio.setDataRate(NRF24.BR_250KBPS)
radio.setPALevel(NRF24.PA_MAX)

radio.setAutoAck(0)
radio.disableCRC() # sets the EN_CRC as required by datasheet. For Rx packet to reach RX_FIFO address and CRC match is needed. 
#radio.setCRCLength(NRF24.CRC_DISABLED)
radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1, pipes[1])
radio.powerUp()
radio.startListening()
radio.stopListening()
radio.printDetails()

def slave():
	radio.startListening()
	print("Now Listening")
	while True:
		pipe = [0]
		while not radio.available(pipe, True):
			print radio.print_status(radio.get_status())
			time.sleep(1000000/1000000.0)		
		print("Received")
		recv_buffer = []
		radio.read(recv_buffer)
		print recv_buffer
def master():
	print("Now sending")
	while True:
		mybuf="V3535  8.28"
		myencodedbuf = mybuf.encode()
		mybytearray=bytearray(myencodedbuf)
		radio.write(mybytearray)
		time.sleep(200/1000000.0)
		print radio.whatHappened()
		time.sleep(6000000/1000000.0)
try:
	slave()
except KeyboardInterrupt:
	radio.powerDown()
	print(" Keyboard Interrupt detected. Exiting...")
	sys.exit()
