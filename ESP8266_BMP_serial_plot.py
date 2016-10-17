#    FILE: ESP8266_BMP_serial_plot.py
#  AUTHOR: jpolton
#    DATE: 14 Oct 2016
# PURPOSE: Read streaming data from the serial port and plot graphs of temperature and pressure.
#          Data stream created by ESP8266_BMP_serial_plot.ino
#   BOARD: ESP8266 NodeMCU 1.0 (ESP-12E module)
#FIRMWARE: Arduino IDE
# SENSORS: BMP280
#     URL:
# HISTORY:
# 0.1.00 initial version. Gleaned from library demo. Works.
#
# STATUS: WORKS
#
#   NOTE: Can not have both Arduino IDE and python reading
#         the serial data stream at the same time
#
# WIRING: (note in some places the D3 and D4 pins were reversed, but this doesn't work)
# VCC      Red   3V3
# GND      blk   GND
# SCK,SCL  blue  D3
# SDA,SDI  purp  D4
# CSB
# SDO      brn   3V3
#
# To run launch ipython and type run ESP8266_BMP_serial_plot.py
#$ ipython
#>> run ESP8266_BMP_serial_plot.py
#
# COMMENTS
# * This method produces plots at 30Hz. This is the speed you get with random data ==> rate is not limited by comms but by matplotlib method
# * It would be good to plot both temperature and pressure, as relic code suggests.

import serial # import the serial library
import numpy
import matplotlib.pyplot as plt
import time
from drawnow import *

tempArr = []
pressArr = []
timeArr = []
arduinoData = serial.Serial('/dev/tty.SLAB_USBtoUART',9600) # create serial object called arduinoData
plt.ion() # Tell matplotlib want ot interactive mode to plot data
cnt=0

duration = 20  # Duration for sampling (s)
N = 250 # number of data points stored in memory


def makeFig(): # Create a function for the entire plot
#	plt.ylim(15,25)                                 #Set y min and max values
#	plt.title('My Live Streaming Sensor Data')      #Plot the title
#	plt.grid(True)                                  #Turn the grid on
#	plt.ylabel('Temp C')                            #Set ylabels
	plt.plot(tempArr, 'ro-', label='Degrees C')       #plot the temperature
#	plt.legend(loc='upper left')                    #plot the legend
	plt2=plt.twinx()                                #Create a second y axis
#	plt.ylim(98000,102000)                          #Set limits of second y axis- adjust to readings you are getting
	plt2.plot(pressArr, 'b^-', label='Pressure (Pa)') #plot pressure data
#	plt2.set_ylabel('Pressure (Pa)')                    #label second y axis
#	plt2.ticklabel_format(useOffset=False)           #Force matplotlib to NOT autoscale y axis
#	plt2.legend(loc='upper right')                  #plot the legend
					    
# Skip first reading as it may be incomplete
while (arduinoData.inWaiting()==0): #Wait here until there is data
	pass #do nothing
arduinoString = arduinoData.readline() #read the line of text from the serial port

tstart = time.time()
while ( cnt <= N ): # While loop that loops forever
	while (arduinoData.inWaiting()==0): #Wait here until there is data
		pass #do nothing
	arduinoString = arduinoData.readline() #read the line of text from the serial port
	dataArray = arduinoString.split(',')   #Split it into an array called dataArray
	press = float( dataArray[1])            #Convert first element to floating number and put in temp
	temp = float( dataArray[0])            #Convert second element to floating number and put in P
	tempArr.append(temp)                     #Build our tempF array by appending temp readings
	pressArr.append(press)                     #Building our pressure array by appending P readings
	timeArr.append(time.time()-tstart)
	print press
	cnt=cnt+1
	if(cnt>N):                            #If you have 50 or more points, delete the first one from the array
		tempArr.pop(0)                       #This allows us to just see the last 50 data points
		pressArr.pop(0)
		timeArr.pop(0)

# Setting the plot up
fig, ax = plt.subplots()
line, = ax.plot(numpy.linspace(-float(N)/float(30), 0, N), numpy.random.randn(N))
ax.lines.remove(line) # remove line from bg
fig.canvas.draw()
line, = ax.plot(range(N), 101020*numpy.random.randn(N))

plt.pause(1)

# Set up the plot details
dp = numpy.max(pressArr) - numpy.min(pressArr)
plt.ylim( numpy.min(pressArr)-dp*0.5, numpy.max(pressArr)+dp*0.5 )
plt.xlim( -float(N)/float(30), 0 )
plt.xlabel('Time since now (s)')
plt.ylabel('Pressure (Pa)')
plt.grid(True)
line, = ax.plot( timeArr, pressArr ) 

cnt = 0

tstart = time.time()
while (time.time()-tstart < duration): # While loop that loops forever
	while (arduinoData.inWaiting()==0): #Wait here until there is data
		pass #do nothing
	arduinoString = arduinoData.readline() #read the line of text from the serial port
	dataArray = arduinoString.split(',')   #Split it into an array called dataArray
	press = float( dataArray[1])            #Convert first element to floating number and put in temp
	temp = float( dataArray[0])            #Convert second element to floating number and put in P
	tempArr.append(temp)                     #Build our tempF array by appending temp readings
	pressArr.append(press)                     #Building our pressure array by appending P readings
	timeArr.append(time.time()-tstart)
	cnt=cnt+1
	tempArr.pop(0)                       #This allows us to just see the last 50 data points
	pressArr.pop(0)
	timeArr.pop(0)
	line.set_xdata([ timeArr[i] - timeArr[-1] for i in range(len(timeArr))])
	line.set_ydata(pressArr)
	fig.canvas.draw()
	fig.canvas.flush_events()

print 'Freq:',cnt/float(duration),'Hz'
arduinoData.close()



