# CyclingPusher

## Description

This project provides an [Arduino](http://arduino.cc/) sketch that can power an Arduino + Ethernet Shield (or Arduino with built-in ethernet) to make a stationary bike computer that can upload Cycling sessions results to [Runkeeper](http://runkeeper.com/)'s [Healthgraph API](http://developer.runkeeper.com/healthgraph), where it get added to the user's activities feed.
The activity can then be posted to Twitter or Facebook automatically by Runkeeper.

It doesn't need to be connected to a computer for that purpose, only a wired ethernet connection + dhcp is required.

It'll also display either current speed and total distance or average speed and elapsed time during normal use via a LCD.

The sensor used is the [reed switch](http://en.wikipedia.org/wiki/Reed_switch) that was already present in my stationary bike connected to the Arduino with a pulldown resistor.

The elapsed time only counts 15 seconds chunks when there was some activity, otherwise it'll pause.

When pausing, pressing the Action button ends the cycling session and uploads the:

 * Starting Time
 * Total Distance
 * Elapsed Time

for the current session. Afterwards, the Arduino needs to be restarted to start a new one.

## Use

Plug the ethernet cable and the USB cable in.
The Arduino will display its IP address and the current UTC time that it obtained via NTP.

Pedal away and it'll display the current speed and total distance. Press the Action button to switch to average speed and elapsed time.

Stop pedaling for ~ 15 seconds and the pause screen will come up. Press the Action button and it'll end the session and upload the result.

To start a new session, you'll need to restart the Arduino.

## In action

### Prototype during testing
![Prototype during testing](https://github.com/reefab/CyclingPusher/raw/master/images/prototype.jpg)
### Runkeeper's screenshot
![Runkeeper screenshot](https://github.com/reefab/CyclingPusher/raw/master/images/BikeProjectRunkeeper.png)
### Reed Switch (in the top of the picture)
![Stationary bike's reed switch and magnetic brake](https://github.com/reefab/CyclingPusher/raw/master/images/bike.jpg)

## Setup

You'll need:

 * Runkeeper user account
 * create a Runkeeper app to get an access token (See below)
 * Arduino Uno + Ethernet shield or equivalent
 * 16x2 LCD
 * Stationary bike with a reed switch
 * [Time library](http://arduino.cc/playground/Code/Time)
 * [HTTPClient library](http://interactive-matter.eu/how-to/arduino-http-client-library/)

The reed switch needs to be plugged in Pin2 and the Action button in Pin3 to use the Arduino's hardware interrupts.

You'll need to modify/provide the MAC address and Healthgraph's access token in the Bike.ino file.
You might want to change the number of meters per reed switch activation too.

The wiring diagram will be provided later.

### How to get access token

First, once you have your Runkeeper's account got to:

http://runkeeper.com/partner/applications/registerForm

Fill in the "Application Name", "Description", "Organization", select "Activity Tracking" and "Estimated Date of Publication".

You'll get a "Client ID" and a "Client Secret".

Open in your browser: https://runkeeper.com/apps/authorize?client_id=XXX&response_type=code&redirect_uri=http%3A%2F%2Flocalhost%3A3001%2Fback

Replace XXX with your "Client ID". Authorize your application and it'll try to redirect you to an invalid page but you'll just need the authorization code you'll find in the url bar.

Now, in a shell:

    curl https://runkeeper.com/apps/token \
    -d "grant_type=authorization_code" \
    -d "code=THE_CODE_YOU_JUST_GOT" \
    -d "client_id=XXX" \
    -d "client_secret=XXX" \
    -d "redirect_uri=http%3A%2F%2Flocalhost%3A3001%2Fback"

Please note that the 'redirect_uri' needs to the be exact same one as before.

It'll then give you the access token you can use in your Arduino's code.





