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
* Install script to system ( /usr/local/bin/ptl_ctrl_pannel_update.py ) if wanted

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

## CRON to run the script

You might want to run the update every x minutes. Example bellow for 2min interval.

Note: this only work properly with the auto-reset on the arduino disabled

```
#Run python script to get info from PTL Control pannel from Arduino (serial)
#And update the status by sending a HTTP get request
*/2 * * * *      nobody timeout 60 /usr/local/bin/ptl_ctrl_pannel_update.py
```

Note that in this example, the python script is running as user "nobody".
Also make sure to change script (log file..) as appropriate for your setup.
