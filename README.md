# MSP430-Fun-Hat

 * This program links the MSP430G2553 chip to the HC05 Bluetooth Module, which
 * communicates with the chip to change the device's state using UART.
 * An Android app was designed to link to the bluetooth module and send characters
 * through which the state of the program is changed.
 *
 * With each state ("High Key Obnoxious", "Medium Key Obnoxious", "Low Key Obnoxious",
 * and off), are associated different outputs of LED flashing and fading sequences,
 * as well as a song. Both outputs are accomplished using Pulse Width Modulation in
 * TimerA0 and TimerA1, respectively. A string of LEDs that was soldered together and
 * a speaker were attached to a fedora by taking the MSP430G2553 off-chip and
 * incorporating all components using a skillfully hidden breadboard inside of the hat.
