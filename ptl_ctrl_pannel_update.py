#! /usr/bin/env python

# AR 2014-01-10 v0.1 (beta) First version
# To control and receive data from control panel in PTL space
# Arduino sketch for control panel: 
# https://github.com/PostTenebrasLab/space_API_control_panel

import serial
import urllib
import urllib2
import time
import sys

# Verbose mode if "-v" is passed as 1st arg
if len(sys.argv) > 1 and sys.argv[1] == "-v":
    verbose = 1
else:
    verbose = 0

#Serial 
SERIAL_DEV = "/dev/arduino0"
SERIAL_RATE = 115200

#Parameters for post request
API_KEY = "Space_Invader"
REFRESH_DELAY = 60

# Setup serial port (this will reset Arduino)
ser = serial.Serial(SERIAL_DEV, SERIAL_RATE)
if verbose:
	print "Using device " + ser.name + " at " + str(SERIAL_RATE)
ser.open()
ser.isOpen()

time.sleep(3)

# Get value from control Panel left counter
def get_value_1():
  ser.write("get 1\n")
  return ser.readline()

# Get value from control Panel left counter
def get_value_2():
  ser.write("get 2\n")
  return ser.readline()

# Set value from control Panel left counter
def set_value_1(i):
  ser.write("set 1 " + str(i) + "\n")

# Set value from control Panel right counter
def set_value_2(i):
  ser.write("set 2 " + str(i) + "\n")

while True:
  time.sleep(REFRESH_DELAY)
  minute = int(get_value_1())
  # Ppl count returns 10x the value. Need to be rounded off to closest number
  ppl_count = int(round(int(get_value_2()) / 10.0))

  if minute == 0:
      open_closed = "closed"
      txt_open_closed = "The lab is closed."
  elif minute > 0 and minute <= 60:
      open_closed = "open"
      txt_open_closed =  "The lab is open - remaining time is " + str(minute) + " min."
  elif minute > 60:
      open_closed = "open"
      hour = minute // 60
      txt_open_closed = "The lab is open - planned to be open for " + str(hour) + " more hours."

  if ppl_count == 0:
      txt_ppl = "Nobody here !"
  elif ppl_count == 1:
      txt_ppl = "One lonely hacker in the space."
  else:
      txt_ppl = "There are " + str(ppl_count) + " hacker in the space."

  status_text = txt_open_closed + " " + txt_ppl + "  " + "[Set by PTL control panel]"

  if verbose:
      print minute
      print ppl_count
      print status_text
 
  url = 'http://www.posttenebraslab.ch/api/change_status'
  values = {'api_key' : API_KEY,
            'open_closed' : open_closed,
            'status' : status_text }
  
  data = urllib.urlencode(values)
  req = urllib2.Request(url, data)

  try:
    response = urllib2.urlopen(req)
    the_page = response.read()
    print the_page                                                                               
  except BadStatusLine:
    print "Could not fetch %s" % url


  the_page = response.read()
  
  print the_page
