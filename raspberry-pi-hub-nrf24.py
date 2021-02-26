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




import re
import os
import shutil
from datetime import *
import smbus
import math
import signal
import socket
from threading import Thread
from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket


from nrf24 import NRF24
import time
import sys
import array
from struct import *
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
pipes = [[0xbc, 0xbc, 0xbc, 0xbc, 0xbc],[0xed, 0xed, 0xed, 0xed, 0xed]]

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
radio.openWritingPipe(pipes[1]) 
radio.openReadingPipe(1, pipes[0])
radio.powerUp()
radio.startListening()
radio.stopListening()
radio.printDetails()


#### GLOBAL VARIABLES ######
TCP_IP=''
HTTP_PORT = 62246
WS_PORT= 8000
TCP_PORT=9999

running_flag = True #used for graceful shutdown 
watchdog_flag = False#used for watchdog

#global commandQ
commandQ=[]

# global value for plotchart
mydict ={}

#WebSocket OPCODES
STREAM = 0x0
TEXT = 0x1
BINARY = 0x2
CLOSE = 0x8
PING = 0x9
PONG = 0xA
#WebSocket clients
clients = []



#Maximum number of websocket clients allowed to connect simultaneously = NUMofWebSocketClientsAllowed
NUMofWebSocketClientsAllowed=2
#Radio receive function
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
#Radio tx function
def master():
	print("Now sending")
	while True:
		#mybuf="V3535  8.28"
		#myencodedbuf = mybuf.encode()
		#mybytearray=bytearray(myencodedbuf)
		#max min values to be sent as maxBminBmaxCminCmaxDminD 2 bytes each x 8 = 16 bytes
		mymaxminvalues=[889,137,1000,200,900,300,1023,128]
		mybytearray=pack('<8H',*mymaxminvalues) #H = unsigned short 2 bytes <=little endian. create 8 short datasets
		radio.write(mybytearray)
		time.sleep(200/1000000.0)
		print radio.whatHappened()
		time.sleep(60000000/1000000.0)

## Commands are handled in the order they are received - this function is invoked by commandthread
def commandhandler(command):
	#global variable declaration needed if modifying global variables - not needed if just using global variables
	if (command.split("=")[0]=="MOTOR"):
		print "haha"
###SENSOR VALUES ARE RECEIVED, SAVED and UPDATED by THIS FUNCTION
def tcp_sensorthread(client_socket):
	#Data on tcp connection from ESPs as previously 1:54:3230 = sensornode:distance in cms:vcc
	#Data on NRF24L01 V4447t-1.2 from outside temp
	#B100.0C100.0 D100.0E100.0 A-23.3a-23.3 where A is inside temp1 a is inside temp2 t is outside temp B is moisture 1 C is moisture 2 D is moisture 3 and E moisture 4
	#Everything gets saved in one file and file is named daily.
	#everything gets saved as datetime:sensor:value:batterylevel
	#for ESP data - 2021-02-20 21-04-33:2:153:3403
	#for nrf data - 2021-02-20 21-04-33:t:-1.09:4465
	#send data to clients in the same format.
	if (client_socket):
		request = client_socket.recv(1024)
		#print 'Received {}'.format(request)
		#1:54:3230 = sensornode:distance in cms:vcc. Saving it uniformly as time:sensor:value. Assigning v for this sensors battery.
		client_socket.close()
		currtime=time.time()
		sensorvalue=request
		SENSORTIME=datetime.now().strftime("%Y-%m-%d %H-%M-%S")
		for ws in clients:
			ws.sendMessage(u'SensorData='+str(SENSORTIME)+u':'+sensorvalue.split(':')[0]+u':'+sensorvalue.split(':')[1])
			ws.sendMessage(u'SensorData='+str(SENSORTIME)+u':v:'+sensorvalue.split(':')[2])
		savesensordatatofile(sensorvalue.split(':')[0]+":"+sensorvalue.split(':')[1])
		savesensordatatofile("v:"+sensorvalue.split(':')[2])
#This function matches incoming data for invalid chars. Doesn't solve all the problems of not having CRC (if the error is 4445 instead of 4465 - it will pass the check but if it is 44*5, it will fail.
def checkfordataintegrity(receivedsensorvalue):
	if not (re.match("[A|V|B|D][\s\-0-9]{1,3}.+[a|C|E|t][\s\-0-9]{1,3}[\.0-9]",receivedsensorvalue)):
		return False
	for char in receivedsensorvalue:
		if not (re.match("[a-zA-Z0-9\-\.\s\:]",char)):#re.match only looks at the 1st position. look for all the chars which should be there - for each char in the recvd data. If any outside the expected vals - return false.
			return False
	return True
def nrf_sensorthread():
	#Data on tcp connection from ESPs as previously 1:54:3230 = sensornode:distance in cms:vcc
	#Data on NRF24L01 V4447t-1.2 from outside temp
	#B100.0C100.0 D100.0E100.0 A-23.3a-23.3 where A is inside temp1 a is inside temp2 t is outside temp B is moisture 1 C is moisture 2 D is moisture 3 and E moisture 4
	#Everything gets saved in one file and file is named daily.
	#everything gets saved as datetime:sensor:value:batterylevel
	#for ESP data - 2021-02-20 21-04-33:2:153:3403
	#for nrf data - 2021-02-20 21-04-33:t:-1.09:4465
	#send data to clients in the same format.
	radio.startListening()
	print("Now Listening")
	while True:
		pipe = [0]
		while not radio.available(pipe, True):
			time.sleep(1000000/1000000.0)		
		recv_buffer = []
		radio.read(recv_buffer)
		rawsensorvalue=array.array('B',recv_buffer).tostring().strip('\x00')
		sensorvalue=re.split(r'([a-z]|[A-Z])',rawsensorvalue) #"V4465t-3.91" becomes ['', 'V', '4465', 't', '-3.91'] ,"B  99.9 C  88.8" becomes ['', 'B', '  99.9 ', 'C', '  88.8'] 
		SENSORTIME=datetime.now().strftime("%Y-%m-%d %H-%M-%S")
		try:
			for ws in clients:
				ws.sendMessage(u'SensorData='+str(SENSORTIME)+u':'+sensorvalue[1]+u':'+sensorvalue[2])
				ws.sendMessage(u'SensorData='+str(SENSORTIME)+u':'+sensorvalue[3]+u':'+sensorvalue[4])
			if checkfordataintegrity(rawsensorvalue):
				savesensordatatofile(sensorvalue[1]+":"+sensorvalue[2])
				savesensordatatofile(sensorvalue[3]+":"+sensorvalue[4])
		except Exception as e:
			if hasattr(e, 'message'):
				print(e.message)
			else:
				print(e)
			pass
def savesensordatatofile(formattedsensordata):
	fobj = open("sensordata"+str(date.today()), 'a+')
	fobj.write(datetime.now().strftime("%Y-%m-%d %H-%M-%S"))
	fobj.write(":")
	fobj.write(formattedsensordata)
	fobj.write('\n')
	fobj.close()
### This function can return chunks of data from the saved sensordata file - useful for plotting by clients - not in use as of 2/19/18
def plotcharts(inputfile,starttime,endtime,*vars):
	try:
		for k in vars:
			mydict["x"+k]=[] # define the x and y variable lists
			mydict["y"+k]=[]
		frobj = open(inputfile, 'r') # input file has data in the form of 1480055273.46,T21.06
		#Modified in order for it to work with saving Tanklevel instead of raw values (1/6/18)
		#nOW - inputfile has data in the form of 1516156620.4,P 4.0\n
		#1/21/18 - WILL NOT WORK with new format of saving 1516156620.4,P 4.0,B5000\n
		#1/21/18 - modified - will work with new format
		for line in frobj: # read the input file line by line
			# line.split results in a list split at comma
			step1=line.split(",") # step1[0]=1516156620.4 step1[1]=P 4.0 step1[2]=B5000
			#-1 is the index from the right = last object in the list.
			# strip removes leading and trailing characters, 
			step2_1=str(step1[1]).strip()
			step2_2=str(step1[2]).strip()
			#[:1] gives the the ?2nd character P,T,L or B [1:] gives anything after ?2nd char
			for variable in vars:
				if (step2_1[:1]==variable):
					if endtime > float(step1[0]) > starttime:
					#Convert unix epoh time to human readable time
						mydict["x"+variable].append(datetime.datetime.fromtimestamp(float(step1[0])).strftime('%Y-%m-%d %H:%M:%S'))
						mydict["y"+variable].append(step2_1[1:])
				elif (step2_2[:1]==variable):
					if endtime > float(step1[0]) > starttime:
					#Convert unix epoh time to human readable time
						mydict["x"+variable].append(datetime.datetime.fromtimestamp(float(step1[0])).strftime('%Y-%m-%d %H:%M:%S'))
						mydict["y"+variable].append(step2_2[1:])
		return mydict
	except Exception as e:
		if hasattr(e, 'message'):
			print(e.message)
		else:
			print(e)

#Request for plot will come as which sensor, starttime, stoptime.
#open the file with name sensordata+startdate name, read lines, seek until time is >time from starttime
#read line by line - if sensor matches the sensor requested - push the time and value data in the list
#reached EOF but time>stoptime - open next file sensordata+startdate+1 name and start reading lines.
# uniformly date time format yyyy-mm-dd|hh-mm-ss seems to work
#return data as "t$2021-02-20 21-04-33:-1.09,2021-02-20 21-04-41:-1.08,"
def sendstoreddata(starttime,endtime,*vars):
	mydatapointcounter=0
	mytempdataholdingdict={} #get all the data in a list and send it to javascript in chunks of "numberofdatapoints" length
	mytempfilecontentholdinglist=[]
	mystarttimedatetimeformat=datetime.strptime(starttime,"%Y-%m-%d %H-%M-%S")
	myendtimedatetimeformat=datetime.strptime(endtime,"%Y-%m-%d %H-%M-%S")
	mystartdate=mystarttimedatetimeformat.date()
	for sensorid in vars:
		mytempdataholdingdict[sensorid]=[]
	try:
		
		while (myendtimedatetimeformat.date()>=mystartdate):
			with open("sensordata"+str(mystartdate),"r") as fobj:
				for myline in fobj:
					mytempfilecontentholdinglist.append(myline.strip())
			mystartdate+=timedelta(days=1)
		#all date between startdate and enddate is in mytempfilecontentholdinglist, line list
		for myline  in mytempfilecontentholdinglist:
			mystoreddatetime=myline.split(":")[0]
			mystoredsensor=myline.split(":")[1]
			mystoredsensorvalue=myline.split(":")[2]
			if (datetime.strptime(mystoreddatetime,"%Y-%m-%d %H-%M-%S")>datetime.strptime(starttime,"%Y-%m-%d %H-%M-%S")):
				for sensorid in vars:
					if (mystoredsensor==sensorid):
						mytempdataholdingdict[sensorid].append(mystoreddatetime+":"+mystoredsensorvalue)
		#now all data is in the dictionary. Take one sensor at a time and return the data in number of datapoint chunks
		#for multiple simultaneous plots - number of data points should be equal
		print mytempdataholdingdict["t"][2]
		for sensorid in vars:
			mymessagetosend = sensorid+"#"
			for element in mytempdataholdingdict[sensorid]:
				mymessagetosend+=element
				mymessagetosend+=","
			for ws in clients:
				ws.sendMessage(u'StoredData='+mymessagetosend.strip(","))
	except Exception as e:
			if hasattr(e, 'message'):
				print(e.message)
			else:
				print(e)
			pass
		
###MAIN WEBSOCKET HANDLER - handle received messages, connecting and disconnecting clients
class SimpleChat(WebSocket):
	def handleMessage(self):
		for client in clients:
			#if client != self:
			#	client.sendMessage(self.address[0] + u' - ' + self.data)
			if (self.data.split("#")[0]=="COMMAND"): #commands sent to server as COMMAND-MOTOR=ON
				#commandhandler(self.data.split("-")[1])
				commandQ.append(self.data.split("-")[1])
			elif (self.data.split("#")[0]=="STOREDDATA"): #Send last hour data on request as opposed to at the time of initial connection - improves UX. Send request from client as STOREDDATA-3600 for last 3600 seconds data 
				#self.sendMessage(u'StoredData-'+str(plotcharts("./sensordata",(time.time()-int(self.data.split("-")[1])),time.time(),"P","p")))
				try:
					print "req for stored data "
					#req for plotting will come with sensor, to and from time. 
					sendstoreddata(self.data.split("#")[1],self.data.split("#")[2],self.data.split("#")[3],self.data.split("#")[4])
				except Exception as e:
					if hasattr(e, 'message'):
						print(e.message)
					else:
						print(e)
			elif client != self:
				client.sendMessage(self.address[0] + u' - ' + self.data)
	def handleConnected(self):
		if len(clients)>(NUMofWebSocketClientsAllowed-1):
			#There is a client already connected - send the connecting client a message - # 3/7/18 - changed it to 2 clients 
			#self.sendMessage(u'E-TRYAGAIN')
			#close the connecting client
			self.close()
			#print "Attempted to connect"
		else:
			clients.append(self)
			try:
				print "client connected"
			except Exception as e:
				if hasattr(e, 'message'):
					print(e.message)
				else:
					print(e)
	def handleClose(self):
		try:
			clients.remove(self)
		except ValueError:
			pass
#This will reassess the given param (voltage, current, sensorvalue, sensorstatus etc after the timeout period and take appropriate action

#This is used to gracefully exit in incase of SIGINT/SIGTERM (ctrl+c)
class GracefulKiller:
	kill_now = False
	def __init__(self):
		signal.signal(signal.SIGINT, self.exit_gracefully)
		signal.signal(signal.SIGTERM, self.exit_gracefully)

	def exit_gracefully(self,signum, frame):
		self.kill_now = True
		print "SIGNAL received"
		global running_flag
		running_flag=False
#Thread # 1
def TCPserverthread(): 
	#ESP8266 sends sensor data to Raspberry Pi using TCP Server Client 
	#TCP Server for sensor
	TCPserverthread.srvr = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	#Fixes address already in use error due to socket being in "TIME_WAIT" stae. allow reusing of socket address
	TCPserverthread.srvr.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	TCPserverthread.srvr.bind((TCP_IP, TCP_PORT))
	TCPserverthread.srvr.listen(5)
	#print 'Listening on {}:{}'.format(TCP_IP, TCP_PORT)
	while running_flag:
		client_sock, address = TCPserverthread.srvr.accept()
		#print 'Accepted connection from {}:{}'.format(address[0], address[1])
		client_handler = Thread(
			target=tcp_sensorthread,
			args=(client_sock,)  # without comma you'd get a... TypeError: handle_client_connection() argument after * must be a sequence, not _socketobject
		)
		client_handler.start()
#Thread # 2
def websocketservarthread(): 
	try:
		websocketservarthread.server = SimpleWebSocketServer('', WS_PORT, SimpleChat)
		websocketservarthread.server.serveforever()
		#print time.asctime(), "Websocket Server Starts - %s:%s" % (TCP_IP, WS_PORT)
	except Exception as e:
		# Just print(e) is cleaner and more likely what you want,
		# but if you insist on printing message specifically whenever possible...
		if hasattr(e, 'message'):
			print(e.message)
		else:
			print(e)

#Thread # 3
def commandthread():
	while running_flag:
		#if there is a command in the Q - pop it and send it to commandhandler
		if (len(commandQ)>0):
			commandhandler(commandQ.pop(0)) #commandQ.pop() serves commands from the last to first, commandQ.pop(0) serves from first to last
		time.sleep(1)
if __name__ == '__main__':
	try:#Load the settings
		t1=Thread(target=TCPserverthread)  
		t2=Thread(target=websocketservarthread)
		t3=Thread(target=commandthread)
		t4=Thread(target=nrf_sensorthread)
		#Daemon - means threads will exit when the main thread exits
		t1.daemon=True
		t2.daemon=True
		t3.daemon=True
		t4.daemon=True
		#Start the threads 
		t1.start()
		t2.start()
		t3.start()
		t4.start()
		#For killing gracefully
		killer = GracefulKiller()
		while True:
			time.sleep(1) # Makes a huge difference in CPU usage - without >95%, with <10%
			if killer.kill_now:
				#Join means wait for the threads to exit
				TCPserverthread.srvr.shutdown(socket.SHUT_RDWR)
				TCPserverthread.srvr.close()
				t1.join #TCPserverthread - has runningflag
				print "TCP server closed"
				#3-7-18 - Using Apache2 HTTP server instead of python server
				'''
				servarthread.httpd.server_close()
				print "http server closed"
				'''
				t2.join #servarthread
				websocketservarthread.server.close()
				print "websocket closed"
				t3.join #websocketservarthread
				print "exited gracefully"
				t4.join #websocketservarthread
				radio.powerDown()
				print "radio shutdown"
				break
	except KeyboardInterrupt:
		#Signal is caught by graceful killer class
		pass

'''
try:
	master()
except KeyboardInterrupt:
	radio.powerDown()
	print(" Keyboard Interrupt detected. Exiting...")
	sys.exit()
	
'''
