# rpi2_fan_controller
A PI-controller that keeps your Raspberry Pi 2 cool by driving a fan with hardware PWM
Author: André Picker

This program can drive a fan by utilizing hardware PWM to cool your Raspberry Pi.
It is intended to be used on the Raspberry Pi 2. It was not tested on other models yet.
The wiring pi library is needed to compile the program. See http://wiringpi.com/ for further information.
You can install wiring pi by executing the following line: sudo apt install wiringpi

How to install:
After cloning the repository to a directory on your Raspberry Pi, type 'make' to compile
It should finish without warnings or errors if wiring pi is installed properly

How to use:
Change the settings in fan.conf to how you need them. Tuning the proportional and integral controller might take some time later.
Execute with 'sudo ./fan <temperature>' substitute <temperatue> with the temperature in °C the Raspberry Pi should stay at
It is recommended to use something like screen to run the software in the background.

How to connect your fan:
For fans with seperate PWM inputs (usually four wires):
Using a PC fan with a seperate PWM input has not been tested yet.

For fans without seperate PWM inputs (two or three wires):
DO NOT USE THE GPIO PIN TO DRIVE THE FAN DIRECTLY!
Doing this will damage your Pi!
Instead use the GPIO Pin to drive a transitor that can handle your fans maximum current. Then use the transistor to drive the fan
Additionally a electrolytic capacitor connected in parallel to your fan in recommended.

Have fun!
