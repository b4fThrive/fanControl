# fanControl

## Introduction

fanControll is a fan controller application for linux systems. It controls fan speeds automatically with the values the user has configured.

Configuration consists to associated sensors to a fan and set there working temperture and stablish an offset temperature if an ambient sensor is supplied in the configuration.

**Note: Currently only tested on an iMac 7,1 with Debian 10.
Only supported hwmon fans and sensors and hddtemp to read disk temperatures**

## Installation from source

fanControl installs with SUID root privileges for fan-control group. It is needed to control fan speeds on the device. Also if you want to use hddtemp, it must have SUID privileges. The installation process described bellow automatically creates the group fan-control and adds the builder user to the group, then set SUID privileges to the hddtemp executable.

Download or clone source files:

`wget https://github.com/b4fThrive/fanControl/archive/master.zip && tar -xvf master.zip && cd master`

or

`git clone https://github.com/b4fThrive/fanControl.git && cd fanControl`

fanControl uses CMake. To install fanControl go to the downloaded folder and run:

`cmake -S. -Bbuild && cmake --build ./build && sudo cmake --install ./build`

or

`cmake -S. -Bbuild && cd build && make && sudo make install`

To use fanControl with some other user, it must be in the group fan-control. You can run:

`sudo adduser <user> fan-control`
