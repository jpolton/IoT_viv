# ESP8266_BMP_AccessPoint.py
#
#########################################################
# THESE HEADER COMMENTS AND CODE ARE COPIED FROM ESP8266_BMP_serial_plot.py
# THEY NEED EDITING.
#########################################################
#
#    FILE: ESP8266_BMP_AccessPoint.py
#  AUTHOR: jpolton
#    DATE: 18 Oct 2016
# PURPOSE: Read streaming data from the WiFi Access Point 192.168.1.4/pressure and plot graphs of temperature and pressure.
#          Data stream created by ESP8266_BMP_AccessPoint.ino
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



import requests
import numpy
import matplotlib.pyplot as plt
import time

tempArr = []
pressArr = []
timeArr = []

plt.ion() # Tell matplotlib want ot interactive mode to plot data
cnt=0

duration = 60  # Duration for sampling (s)
N = 250 # number of data points stored in memory


def readsensor():
    link = "http://192.168.4.1/pressure"
    f = requests.get(link)
    dataString = f.text
    #print dataString
    press = float(dataString.split(' ')[1])
    return press

#####

tstart = time.time()
while ( cnt <= N ): # While loop that loops forever
	press = readsensor()
	pressArr.append(press)                     #Building our pressure array by appending P readings
	timeArr.append(time.time()-tstart)
	print press
	cnt=cnt+1
	if(cnt>N):                            #If you have 50 or more points, delete the first one from the array
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
dp = numpy.max([numpy.max(pressArr) - numpy.min(pressArr), 10])
plt.ylim( numpy.min(pressArr)-dp*0.5, numpy.max(pressArr)+dp*0.5 )
plt.xlim( -float(N)/float(30), 0 )
plt.xlabel('Time since now (s)')
plt.ylabel('Pressure (Pa)')
plt.grid(True)
line, = ax.plot( timeArr, pressArr )

cnt = 0

tspinup = time.time() - tstart
while (time.time()-tstart < tspinup + duration): # While loop that loops forever
	press = readsensor()
	pressArr.append(press)                     #Building our pressure array by appending P readings
	timeArr.append(time.time()-tstart)
	cnt=cnt+1
	pressArr.pop(0)
	timeArr.pop(0)
	line.set_xdata([ timeArr[i] - timeArr[-1] for i in range(len(timeArr))])
	line.set_ydata(pressArr)
	fig.canvas.draw()
	fig.canvas.flush_events()

print 'Freq:',cnt/float(duration),'Hz'
