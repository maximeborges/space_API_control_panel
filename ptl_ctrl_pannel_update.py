#!/usr/bin/env python

# AR 2014-09-10 v0.2 version
# v0.2 added some exception handling
# To control and receive data from control panel in PTL space
# Arduino sketch for control panel + more: 
# https://github.com/PostTenebrasLab/space_API_control_panel

import serial
import time
import sys

import urllib
import urllib2

import logging

##########

#Loging
LOG_FILE = "/var/log/ptl_ctrl_pannel_update/ptl_ctrl_pannel_update.py.log"
DEBUG_LEVEL = logging.INFO
# Implented debug level: logging.INFO, logging.ERROR

#Serial 
SERIAL_DEV = "/dev/arduino0"
SERIAL_RATE = 115200

#Parameters for post request
API_KEY = "Space_Invader"
REFRESH_DELAY = 60

##########

logging.basicConfig(filename=LOG_FILE,level=DEBUG_LEVEL)
logging.info("Script starting: " + (time.strftime("%a, %d %b %Y %H:%M:%S")))

# Setup serial port (this will reset Arduino)
try:
  ser = serial.Serial(SERIAL_DEV, SERIAL_RATE)
  logging.info(("Using device " + str(ser.name) + " at " + str(SERIAL_RATE)))
  ser.open()
  ser.isOpen()
except Exception as error:
  print (error)
  logging.error("Error when opening / initializing serial device (Arduino): ")
  logging.error(error)
  logging.error("Script will exit")
  logging.error(error)
  sys.exit(1)

time.sleep(3)

# Get value from control Panel left counter
def get_value_1():
  try:
    ser.write("get 1\n")
    return ser.readline()
  except Exception as error:
    logging.error("Error when reading value: ")
    logging.error(error)
  return None

# Get value from control Panel left counter
def get_value_2():
  try:
    ser.write("get 2\n")
    return ser.readline()
  except Exception as error:
    logging.error("Error when reading value: ")
    logging.error(error)
  return None

# Set value from control Panel left counter
def set_value_1(i):
  try:
    ser.write("set 1 " + str(i) + "\n")
  except Exception as error:
    logging.error("Error when writing value: ")
    logging.error(error)
  return None

# Set value from control Panel right counter
def set_value_2(i):
  try:
    ser.write("set 2 " + str(i) + "\n")
  except Exception as error:
    logging.error("Error when writing value: ")
    logging.error(error)
  return None

# Main loop
while True:
  time.sleep(REFRESH_DELAY)

  value_1 = get_value_1()
  value_2 = get_value_2()

  if value_1 == None or value_2 == None:
    logging.error("Get Value failed (returned None). Will skip update.")
    continue
  else:
    minute = int(value_1)
    ppl_count = int(round(int(get_value_2()) / 10.0))
    # Ppl count returns 10x the value. Need to be rounded off to closest number

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

  logging.info(time.strftime("%a, %d %b %Y %H:%M:%S"))
  logging.info("Info used for update: ")
  logging.info("minute: " + minute)
  logging.info("ppl_count: " + ppl_count)
  logging.info("status_text: " + status_text)
 
  url = 'http://www.posttenebraslab.ch/api/change_status'
  values = {'api_key' : API_KEY,
            'open_closed' : open_closed,
            'status' : status_text }
  
  try:
    data = urllib.urlencode(values)
    req = urllib2.Request(url, data)
    response = urllib2.urlopen(req)
    the_page = response.read()
  except Exception as error:
    logging.error("Error while doing POST request to http://www.posttenebraslab.ch/api/change_status")
    logging.error(error)
    continue

  logging.info("HTTP response page")
  logging.info(the_page)
