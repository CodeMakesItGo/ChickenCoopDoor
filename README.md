# ChickenCoopDoor
Chick Coop Automated Smart Door

This project uses an Arduino Nano.

For this project we built a chicken coop door that is operated by a smart phone app. 
This allows you to monitor your chicken coop door remotely, open and close it remotely, set a schedule for operation, and operate it using Amazon Alexa.

YouTube build here: https://www.youtube.com/watch?v=RmLc_RDQBD4

# Debug steps if you are having trouble getting this to work:

Test #1:
First thing is to test the relay polarity switcher. To verify, if you touch 5V to the IN1 and IN2 at the same time, the motor should start moving out. 
If it starts moving in, then your polarity is reversed.
If it doesn't move at all. Then there is a bad connection on this part.
 
If this works then we can focus just on the Arduino side.
 
Test #2:
Programming, ensure that D5 is connected to an LED and that it is blinking at 1Hz. If not, check the polarity of the LED to ensure it is connected correctly. If it is, then its possible that the Arduino was not programmed correctly or not getting power.
 
Test #3:
Manually trip the Arduino, jump or short D2 and GND together for at least 2 seconds. This should trigger the relay we tested in test #1 to switch polarity and the actuator should move.
If this does not work, test to ensure that D4 output toggles from high to low or low to high when shorting the D2 to GND. You can also attach the LED to D4 to visually see if D4 is switching states.
 
Test #4:
If tests 1-3 work, then we are down to the garage door opener. Ensure the toggle lines from this device are connected to D2 and GND on the Arduino and you hear a click when you activate the garage door. You can also test with an ohm meter or digital volt meter (DVM). The ohm reading should drop to 0 indicating a short, for 2 seconds after being triggered.
 
If all of this works individually, it should as a system.
 
Once it is working, to test the entire system follow these steps.
1. Open the door and make sure the sensor is not in contact with the magnet.
2. Unplug or power off the Arduino.
3. Power the Arduino back on, the door should start to close for 2 seconds then start to open back up.
4. Again with the door open
5. Unplug or power off the Arduino.
6. Connect the garage door sensor to the magnet
7. Power the Arduino back on, the door should close all the way
 
The idea is that the door will stay shut if you briefly lose power at night. And stay open during the day in the same scenario.


# All parts used in this project:

120V WiFi Smart Garage Door Controller
https://amzn.to/3ikQDJ0

5V USB WiFi Smart Garage Door Controller
https://amzn.to/3mnXkhl

Arduino Nano V3.0 
https://amzn.to/2KoNETr

DC 12v 5Amp Power Supply
https://amzn.to/3ikQrcK

DC 12v 2 channel relay module
https://amzn.to/39HqHDn

DC 5V 2 channel relay module
https://amzn.to/3j30sNQ

8" Linear Actuator (225lbs)
https://amzn.to/2LGo9xw

DC 3v - 40v Voltage Regulator
https://amzn.to/35Rzf9J

Project Box
https://amzn.to/2LYWMhU

Cable Glands
https://amzn.to/3qtqSZR

18 AWG Power Cable
https://amzn.to/35Oqn4N
