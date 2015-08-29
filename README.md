# Arduino-Optical
Optical communication between two Arduinos

Goal of this project is to implement a simple serial communication system using optical link and UART. Transmitted data are organized in packets of fixed but arbitrary length. For both communication nodes the two Arduino Minis and for transport medium an optical fiber are used. From user perspective it can be seen as a synchonization of variables between both Arduinos. Optical transmitter being used is DLT1111 and optical receiver is DLR1111. The Arduino's built-in UART module is being used for driving optical transmitter and for receiving data from optical receiver. Software serial interface is being used for transmitting debug info to PC via RS232.
