# Space API control panel - Ardiono sketch

sketch for the control panel of the lab status (a box in the lab with buttons that control open time of the lab...)

# Build steps
## Requirements
To use gcc-avr you need to set:

    export BOARD=nan0328        #in this case but make boards will show you all board available
    export ARDUINODIR=~/your_arduino_install_dir

and install the following package under ubuntu 12.04

    sudo apt-get install avrdude binutils-avr gcc-avr avr-libc gdb-avr arduino-mk

## Compilation

You can simply run

    make
    make upload

And voila !

# Space API control panel - python script

The PTL Control Pannel Ardiono is connected by USB to a linux computer.
A python script retrieves data (serial) from the Arduino and sends a POST request to the PTL Status API on a remote server.

## Python script

* Get the latest version of "ptl_ctrl_pannel_update.py" on Github (in folder python_status_updater)
* Edit the script and change constant value
* Install script to system ( /usr/local/bin/ptl_ctrl_pannel_update.py ) is wanted

## Udev rule for Arduino

If you run the script as unpriviledged user (recommanded), you will not be able to access the Arduino. You can use following udev rule (test on Debian). 

/etc/udev/rules.d/70-arduino.rules

```
#AR 2014-09 Updated comment
#AR 2013-12 	Added udev rules for PTL Control Panel arduino which is connected to coltello
# 		For retrieving status information via serial
# Github project page: https://github.com/PostTenebrasLab/space_API_control_panel
SUBSYSTEM=="tty" ATTRS{manufacturer}=="Arduino*" SYMLINK+="arduino%n", MODE="0666"
```

Note that this will allow all user full access to the Arduino serial interface.

## CRON to make sure script is running

Once the script is running properly, you can add a CRON job to make sure it is always running. 
This is usefull if the pc is restarted and/or arduino has been disconnected.

Note that in this example, the python script is running as user "nobody".
Also make sure to change script (log file..) as appropriate for your setup.

* /usr/local/sbin/check_running_ptl_ctrl_pannel_update.sh

```
#!/bin/sh

#AR 2014-09-13
#       Script to check if PTL Control pannel script is running - if not start it
#       For use in CRON job - run as root or user that can su to nobody without password
#       NOTE: Control pannel script will fail if arduino is not conected

# Check if python script is running as nobody
if ps ux -u nobody | grep -v grep | fgrep -q 'ptl_ctrl_pannel_update.py'
then   
  exit 0
else
# Restart the script and append to log
  su nobody -c 'cd /tmp && nohup /usr/local/bin/ptl_ctrl_pannel_update.py &'
  su nobody -c 'echo "CRON job detected script is not running: Started at $(date)" >> "/var/log/ptl_ctrl_pannel_update/ptl_ctrl_pannel_update.py.log"'
  exit 0                                                                                               
fi
```

Create CRON job. Here using /etc/cron.d/ on Debian. Set to run every 3minutes.

* /etc/cron.d/ptl_ctrl_pannel_update 
    
```
# m	h	dom	mon	dow	user	command
#AR 2014-09-13
#Run sh script to make sure PTL control pannel status update process running
#If not script will start ptl_ctrl_pannel_update.py as nobody (check script)
*/3 * * * *      root   /usr/local/sbin/check_running_ptl_ctrl_pannel_update.sh
```
