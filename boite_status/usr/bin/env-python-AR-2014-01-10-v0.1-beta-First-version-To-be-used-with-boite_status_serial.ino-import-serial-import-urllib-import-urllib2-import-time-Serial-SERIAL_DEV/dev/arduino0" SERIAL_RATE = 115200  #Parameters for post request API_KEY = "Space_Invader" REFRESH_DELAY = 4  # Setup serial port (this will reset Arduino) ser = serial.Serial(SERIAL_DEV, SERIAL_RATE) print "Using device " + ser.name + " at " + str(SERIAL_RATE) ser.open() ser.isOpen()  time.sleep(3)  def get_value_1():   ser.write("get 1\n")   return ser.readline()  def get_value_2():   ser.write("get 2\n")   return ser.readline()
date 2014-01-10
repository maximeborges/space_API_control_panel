#! /usr/bin/env python                                                                                          

# AR 2014-01-10 v0.1 (beta) First version
# To be used with boite_status_serial.ino

import serial
import urllib
import urllib2
import time

#Serial 
SERIAL_DEV = "/dev/arduino0"
SERIAL_RATE = 115200

#Parameters for post request
API_KEY = "SECRET_API_KEY"
REFRESH_DELAY = 60

# Setup serial port (this will reset Arduino)
ser = serial.Serial(SERIAL_DEV, SERIAL_RATE)
print "Using device " + ser.name + " at " + str(SERIAL_RATE)
ser.open()
ser.isOpen()

time.sleep(3)

def get_value_1():
  ser.write("get 1\n")
  return ser.readline()

def get_value_2():
  ser.write("get 2\n")
  return ser.readline()
  
def set_value_1(i):
  ser.write("set 1 " + str(i) + "\n")

def set_value_2(i):
  ser.write("set 2 " + str(i) + "\n")

while True:
  time.sleep(REFRESH_DELAY)
  minute = int(get_value_1())
  print minute

  if minute == 0:
      open_closed = "closed"
      status_text = "The lab is closed [Status changed from PTL control panel]"
  elif minute > 1 and minute < 60:
      open_closed = "open"
      status_text = "The lab is open - remaining time is " + str(minute) + " min " + "[Status changed from PTL control panel]"
  elif minute > 60:
      open_closed = "open"
      hour = minute // 60
      status_text = "The lab is open - planned to be open for " + str(hour) + " more hour " + "[Status changed from PTL control panel]"
  
  url = 'http://www.posttenebraslab.ch/api/change_status'
  values = {'api_key' : API_KEY,
            'open_closed' : open_closed,                                                                        
            'status' : status_text }
  
  data = urllib.urlencode(values)
  req = urllib2.Request(url, data)
  response = urllib2.urlopen(req)
  the_page = response.read()
  
  print the_page
