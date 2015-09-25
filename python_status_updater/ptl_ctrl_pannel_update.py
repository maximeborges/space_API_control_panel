#!/usr/bin/env python

# AR 2015-09-25
# Script now run once and exit Intended for arduino with auto-reset disabled

# AR 2015-04-13 v0.3
# Constant SERIAL_DEV is now a tupple - allows trying to initiate with multiple dev name
# Workaround for dev name changing from /dev/arduino0 to /dev/arduino1 (...)
# - Changed serial initization code and loggigng
# - Misc: HTTP response moved to logging.debug
# - Misc: Script will exit if it can not read data

# AR 2014-09-10 v0.2
# v0.2 added some exception handling
# To control and receive data from control panel in PTL space
# Arduino sketch for control panel + more:
# https://github.com/PostTenebrasLab/space_API_control_panel

#Needed libraries
import serial
import time
import sys
import urllib
import urllib2
import logging

#### CONSTANT #####
# Loging
LOG_FILE = "/var/log/ptl_ctrl_pannel_update/ptl_ctrl_pannel_update.py.log"
DEBUG_LEVEL = logging.INFO
# Implented debug level: logging.INFO, logging.ERROR, logging.DEBUG
# Note: Debug will print API key to log file !

# Serial
SERIAL_DEV = "/dev/arduino"
SERIAL_RATE = 115200

# Parameters for post request
API_KEY = 'SECRET'
URL = 'http://...'
########

# Setup logging
try:
    logging.basicConfig(filename=LOG_FILE, level=DEBUG_LEVEL)
    logging.info("Script starting: " +
                 (time.strftime("%a, %d %b %Y %H:%M:%S")))
except Exception as error:
    print(error)
    sys.exit(1)

# Setup serial port (this will reset Arduino)
# Tries initiation with all device defined in tupple SERIAL_DEV
try:
    dev = SERIAL_DEV
    logging.info(("Trying to connect to serial port " + str(dev) + " at " + str(SERIAL_RATE)))
    ser = serial.Serial(dev, SERIAL_RATE)
    ser.open()
    ser.isOpen()
    logging.info("Serial setup done")
except Exception as error:
    print(error)
    logging.error(
        "Error when opening / initializing serial device")

# Helper function 

# Get value from control Panel right counter
def get_value_r():
    try:
        ser.write("get 1\n")
        return ser.readline()
    except Exception as error:
        logging.error("Error when reading value: ")
        logging.error(error)
    return None

# Get value from control Panel left counter
def get_value_l():
    try:
        ser.write("get 2\n")
        return ser.readline()
    except Exception as error:
        logging.error("Error when reading value: ")
        logging.error(error)
    return None

# Set value from control Panel right counter
def set_value_r(i):
    try:
        ser.write("set 1 " + str(i) + "\n")
    except Exception as error:
        logging.error("Error when writing value: ")
        logging.error(error)
    return None

# Set value from control Panel left counter
def set_value_l(i):
    try:
        ser.write("set 2 " + str(i) + "\n")
    except Exception as error:
        logging.error("Error when writing value: ")
        logging.error(error)
    return None

# Main

# Sleep and get value from Arduino
time.sleep(2)
value_r = get_value_r()
value_l = get_value_l()

# Generate update text based on value
if value_r is None or value_l is None:
    logging.error("Get Value failed (returned None). Will exit.")
    sys.exit(1)
else:
    minute = int(value_r)
    ppl_count = int(round(int(get_value_l()) / 10.0))
    # Ppl count returns 10x the value. rounded off to closest int

if minute == 0:
    open_closed = "closed"
    txt_open_closed = "The lab is closed."
elif minute > 0 and minute <= 60:
    open_closed = "open"
    txt_open_closed = "The lab is open - remaining time is " + \
        str(minute) + " min."
elif minute > 60:
    open_closed = "open"
    hour = minute // 60
    txt_open_closed = "The lab is open - planned to be open for " + \
        str(hour) + " more hours."

if ppl_count == 0:
    txt_ppl = "Nobody here !"
elif ppl_count == 1:
    txt_ppl = "One lonely hacker in the space."
else:
    txt_ppl = "There are " + str(ppl_count) + " hacker in the space."

status_text = txt_open_closed + " " + \
    txt_ppl + "  " + "[Set by PTL control panel]"

#Some Logging
logging.info(time.strftime("%a, %d %b %Y %H:%M:%S"))
logging.info("Info used for update: ")
logging.info("minute: " + str(minute))
logging.info("ppl_count: " + str(ppl_count))
logging.info("status_text: " + str(status_text))

#Send HTTP request to server to update status
values = {'api_key': API_KEY,
          'open_closed': open_closed,
          'status': status_text}

try:
    data = urllib.urlencode(values)
    req = urllib2.Request(URL, data)
    response = urllib2.urlopen(req)
    the_page = response.read()
except Exception as error:
    logging.error("Error while doing POST request to " + URL)
    logging.debug("values used: " + str(values))
    logging.error(error)

logging.debug("HTTP response page:")
logging.debug(the_page)
logging.debug("End of HTTP response")
