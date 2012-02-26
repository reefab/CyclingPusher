# CyclingPusher

## Description

This projet provides an [Arduino](http://arduino.cc/) sketch that can power an Arduino + Ethernet Shield (or Arduino with built-in ethernet) to make a stationary bike computer that can upload Cycling sessions results to [Runkeeper](http://runkeeper.com/)'s [Healthgraph API](http://developer.runkeeper.com/healthgraph), where it get added to the user's activities feed.

It'll work as a normal "computer", displaying either current speed and total distance or average speed and elasped time.

For that, I use the reed switch that was already present in my stationary bike connected to the arduino with a pulldown resistor.

The elasped time only counts 15 seconds chunks when there was some activity, otherwise it'll pause.

When pausing, pressing the Action button ends the cycling session and uploads the:

 * Starting Time
 * Total Distance
 * Elasped Time

for the current session. Afterwards, the arduino needs to be restarted to start a new one.

## Setup

You'll need:

 * Runkeeper user account
 * create a Runkeeper app to get an access token (See below)
 * Arduino Uno + Ethernet shield or equivalent
 * 16*2 lcd
 * Stationary bike with a reed switch
 * [Time library](http://arduino.cc/playground/Code/Time)
 * [HTTPClient library](http://interactive-matter.eu/how-to/arduino-http-client-library/)

The reed switch needs to be plugged in Pin2 and the Action button in Pin3 to use the Arduino's hardware interrupts.

You'll need to modify/provide the MAC address and Healthgraph's access token in the Bike.ino file.
You might want to change the number of meters per reed switch activation too.

The wiring diagram will be provided later.

## Use

Plug the ethernet cable and the USB cable in.
The Arduino will display its IP address and the current UTC time that it obtained via NTP.

Pedal away and it'll display the current speed and total distance. Press the Action button to switch to average speed and elasped time.

Stop pedaling for ~ 15 seconds and the pause screen will come up. Press the Action button and it'll end the session and upload the result.

To start a new session, you'll need to restart the Arduino.

## In action

### Prototype during testing
![Prototype during testing](https://github.com/reefab/CyclingPusher/raw/master/images/prototype.jpg)
### Runkeeper's screenshot
![Runkeeper screenshot](https://github.com/reefab/CyclingPusher/raw/master/images/BikeProjectRunkeeper.png)
### Reed Switch (in the top of the picture)
![Stationary bike's reed switch and magnetic brake](https://github.com/reefab/CyclingPusher/raw/master/images/bike.jpg)
