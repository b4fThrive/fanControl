# fanControl

## Introduction

fanControll is a fan controller application for linux systems. It controls fan speeds automatically with the values the user has configured.

Configuration consists to associated sensors to a fan and set there working temperture and stablish an offset temperature if an ambient sensor is supplied in the configuration.

**Note: Currently only tested on an iMac 7,1 with Debian 10.
Only supported hwmon fans and sensors and hddtemp to read disk temperatures**

## Installation

To use fanControl the user must be in the group fan-control. The installation process automatically creates the group and adds the user builder to the group.

fanControl uses CMake. To install fanControl go to the downloaded folder and run:

`cmake -S. -Bbuild && cmake --build ./build && sudo cmake --install ./build`

or

`cmake -S. -Bbuild && cd build && make && sudo make install`
