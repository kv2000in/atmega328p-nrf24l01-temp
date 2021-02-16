#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Example program to receive packets from the radio
#
# João Paulo Barraca <jpbarraca@gmail.com>
#
#
#Working raspberry pi (both tx and rx) has 
# uname -a Linux raspberrypi 4.9.59+ #1047 Sun Oct 29 11:47:10 GMT 2017 armv6l GNU/Linux
# python -V 2.7.13
# RPi.GPIO-0.6.3.egg-info
# spidev-3.3.egg-info
#
#Tx only raspberry pi has 
#uname - a Linux raspberrypi 4.9.59+ #1047 Sun Oct 29 11:47:10 GMT 2017 armv6l GNU/Linux
#python -V 2.7.13
#RPi.GPIO-0.6.5.egg-info changed it RPi.GPIO-0.6.3.egg-info via sudo pip install rpi.gpio==0.6.3 - reboot - still doesn't work Rx
#spidev-3.0.egg-info - updating to spidev-3.3.egg-info via sudo apt-get install python-spidev - reboot - still doesn't work Rx
# So, ultimately - same software - as much as I can investigate - same HW. RX from Atmega328 NRF24L01 works on one SW setup and doesn't work on another.

from nrf24 import NRF24
import time
import sys
import array
import struct
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
pipes = [[0xed, 0xed, 0xed, 0xed, 0xed], [0xbc, 0xbc, 0xbc, 0xbc, 0xbc], [0xbc, 0xbc, 0xbc, 0xbc, 0xbd]]

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
radio.openWritingPipe(pipes[2]) # will be used to send config to atmega with display
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
			time.sleep(1000000/1000000.0)		
		recv_buffer = []
		radio.read(recv_buffer)
		sensorvalue=array.array('B',recv_buffer).tostring().strip('\x00')
		print sensorvalue
def master():
	print("Now sending")
	while True:
		#max min values to be sent as maxBminBmaxCminCmaxDminD 2 bytes each x 6 = 12 bytes
		mymaxminvalues=[1023,100,1023,100,1023,100]
		mybytearray=struct.pack('hhhhhh',*mymaxminvalues) #h = unsigned short 2 bytes. create 8 short datasets
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
