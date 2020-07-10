# Space API control panel firmware

Firmware for the box in the entry of the lab with buttons that control open time of the lab and number of people there.

## Build steps

### With `Arduino IDE`

Install the following libraries:

    Bounce2
    Ethernet

Click on the `Upload` button and you're done!

### With `arduino-cli`
Install libraries, compile and upload:

    arduino-cli lib install Bounce2 Ethernet
    arduino-cli compile --fqbn arduino:avr:uno
    arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno

## Communication

For the server backend, see the [PTL-Status-API](https://github.com/PostTenebrasLab/PTL-Status-API) project.

### Ethernet shield

As of January 2016, Ethernet shield support has been added to the Arduino sketch. This allow the arduino to be autonomous and do HTTP request on it's own. A simple POST request is used for updating the status.

To see how it was working before, check the commits from 2016 (need to be tagged).