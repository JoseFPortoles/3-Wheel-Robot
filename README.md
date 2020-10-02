# 3-Wheeled robot
These files are part of an old project for developing a 3-wheel robot. The traction/steering scheme consisted of two fixed wheels on the back and a directional wheel at the front. The front wheel was a BLDC-motor wheel and it was steered with a DC-motor thanks to a rotational mounture.

The electronic control unit consisted of an Arduino card that worked both as a controller for the motor controllers and a sensor reader. The Arduino was connected via USB to a small travel router running OpenWRT. The router provided WiFi connectivity as well as the possibility to control a number of USB peripherals via.

For orientation a L3G4200D would be built into the robot's ECU. The gyro was connected to the arduino card via I2C. PerricoApp is a simple app written in Kivy that would run in a PC with a serial port mapped to the Arduino card via wifi through the router. In this configuration, the app would read the gyro, integrate the readings to obtain the angle in the horixontal plane, and show it on the screen both as a numeric reading and as a schematic of the robot turning in real time.

Ultimately this was intended to be part of a wider robot control app that would run from a mobile device.

See a video of a working example at https://youtu.be/f6jrQS4w1dc
