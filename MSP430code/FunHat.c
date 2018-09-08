/*
 * Allison Z. Durkan, Hannah A. Hebert, Elizabeth R. Kenny
 * "Fun Hat"
 * EC450 Final Project
 * Spring 2016
 * ********************
 * ********************
 * FunHat.c
 *
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
 *
 *
 */

#include <msp430g2553.h>
#include <stdint.h>

#define RXD BIT1
#define TXD BIT2
#define GREEN BIT6

#define SMCLKRATE 8000000
#define BAUDRATE 9600

#define BRDIV16 ((16*SMCLKRATE)/BAUDRATE)
#define BRDIV (BRDIV16/16)
#define BRMOD ((BRDIV16-(16*BRDIV)+1)/16)
#define BRDIVHI (BRDIV/256)
#define BRDIVLO (BRDIV-BRDIVHI*256)

#define LEDs  BIT6
#define PERIOD  2000
#define minHalfPeriod 100
#define maxHalfPeriod 1900

unsigned int halfPeriod = minHalfPeriod;

unsigned char Rx_Data = 0;                      // Byte received via UART
volatile unsigned char fade_flag = 0;   //if using PWM
unsigned int count = 0;                         //Used for the flashing LEDs (case L)

// bitmask for speaker for sound output
#define SPEAKER  0x01       // speaker (TA1) at P2.0
// LOW KEY OBNOXIOUS: Amazing Grace
const int amazingNotes[] = { 1911, 1432, 1136, 1432, 1136, 1276, 1432, 1703,
                1911, 1, 1911, 1432, 1136, 1432, 1136, 1276, 956, 1136, 956, 1136, 956,
                1136, 1432, 1911, 1703, 1432, 1, 1432, 1703, 1911, 1, 1911, 1432, 1136,
                1432, 1136, 1276, 1432
                                };
const int amazingDurs[] = { 300, 600, 200, 100, 600, 300, 600, 300, 600, 1, 300,
                600, 200, 100, 600, 300, 900, 300, 200, 100, 200, 100, 600, 300, 400,
                200, 1, 200, 100, 600, 1, 300, 600, 200, 100, 600, 300, 900
                                };
// HIGH KEY OBNOXIOUS: Joy to the World at 3x speed
const int joyNotes[] = { 1911, 1012, 1136, 1276, 1432, 1517, 1703, 1911, 1,
                1276, 1136, 1, 1136, 1012, 1, 1012, 956, 1, 956, 1, 956, 1012, 1136,
                1276, 1, 1276, 1432, 1517, 956, 1, 956, 1012, 1136, 1276, 1, 1276, 1432,
                1517, 1, 1517, 1, 1517, 1, 1517, 1, 1517, 1, 1517, 1432, 1276, 1432,
                1517, 1703, 1, 1703, 1, 1703, 1, 1703, 1517, 1432, 1517, 1703, 1517,
                956, 1136, 1276, 1432, 1517, 1432, 1517, 1703, 1911 };
const int joyDurs[] = { 150, 125, 50, 250, 75, 150, 150, 250, 1, 75, 250, 1, 75,
                250, 1, 75, 250, 1, 75, 1, 75, 75, 75, 75, 1, 125, 50, 75, 75, 1, 75,
                75, 75, 75, 1, 125, 50, 75, 1, 75, 1, 75, 1, 75, 1, 75, 1, 50, 50, 250,
                50, 50, 75, 1, 75, 1, 75, 1, 50, 50, 250, 50, 50, 75, 150, 75, 125, 50,
                75, 75, 150, 150, 300
                                };
// MEDIUM KEY OBNOXIOUS: Mamma Mia by ABBA
const int mammaNotes[] = {
                1911,                                                                      // 1
                1432, 1517, 1432, 1276, 1136, 1432,            // 2
                1, 1432, 1517, 1432, 1517, 1703,                   // 3
                1,                                                                 // 4
                1911,                                                                      // 5
                1432, 1517, 1432, 1276, 1136, 1432,            // 6
                1, 1432, 1517, 1432, 1517, 1703,               // 7
                1,                                                                 // 8
                1911,                                                                      // 9
                1703, 1432, 1136, 1136, 1,                                 // 10
                1136, 1276, 1136, 1276, 1432, 1911,            // 11
                1703, 1432, 1136, 1,                                               // 12
                1136, 1276, 803, 1136, 1432, 1276, 1136,       // 13
                1276, 1432, 1,                                                             // 14
                1136, 1, 1136, 1276, 1432, 1276, 1136,         // 15
                1911, 1,                                                   // 16
                1073, 1136,                                                        // 17
                1276, 1911, 1, 1911, 1, 1911, 1, 1911, 1, 1911, 1, 1911, 1, 1911, 1,      // 18
                1073, 1136, 1276,                                          // 19
                1911, 1, 1911, 1, 1911, 1, 1911, 1, 1911, 1, 1911, 1, 1911, 1,            // 20
                1136, 1432, 1276, 1432, 1,                     // 21
                1136, 1432, 1276, 1432, 1, 1432,                   // 22
                1, 1432, 1276, 1136, 1276, 1432, 1,            // 23
                1276, 1432, 1, 1073,                                       // 24
                1, 1073, 1, 1073, 1, 1073, 1136, 1432, 1,      // 25
                1276, 1432, 1276, 1432, 1, 1432,               // 26
                1, 1432, 1276, 1136, 1276, 1432, 1,            // 27
                1276, 1432, 1, 1073,   1,                      // 28
                1073, 1, 1073, 1, 1073, 1136, 1432, 1, 956, 1, 956, 1, 956, 1, 956, 1276,  // 30
                1136, 1073, 1,                                                                  // 31
                1136, 1, 1136, 1, 1136, 1, 1136, 1517,          // 32
                1432, 1276, 1,                                                                  // 33
                1276, 1432, 1, 1073, 1,                                         // 34
                1073, 1, 1073, 1, 1073, 1136, 1432, 1 };                // 35
//
const int mammaDurs[] = { 320,                          // 1
                320, 160, 160, 160, 320, 160,           // 2
                1, 160, 160, 160, 320, 320,             // 3
                1,                                      // 4
                320,                                            // 5
                320, 160, 160, 160, 320, 160,           // 6
                1, 160, 160, 160, 320, 320,             // 7
                1,                                                      // 8
                160,                                                            // 9
                160, 160, 160, 640, 1,                                  // 10
                160, 160, 160, 160, 320, 160,           // 11
                160, 160, 640, 1,                                       // 12
                160, 160, 160, 160, 160, 480, 320,      // 13
                320, 320, 1,                                                    // 14
                320, 1, 160, 160, 160, 480, 320,        // 15
                640, 1,                                                         // 16
                320, 320,                                               // 17
                160, 160, 1, 160, 1, 160, 1, 160, 1, 160, 1, 160, 1, 160, 1,    // 18
                320, 320, 160,                                           // 19
                160, 1, 160, 1, 160, 1, 160, 1, 160, 1, 160, 1, 480, 1,         // 20
                160, 160, 160, 160, 1,                          // 21
                160, 160, 160, 160, 1, 160, 1, 160, 160, 160, 160, 160, 1,      // 23
                320, 320, 1, 160,                       // 24
                1, 160, 1, 160, 1, 160, 320, 320, 1,    // 25
                160, 160, 160, 160, 1, 160,             // 26
                1, 160, 160, 160, 160, 160, 1,          // 27
                320, 320, 1, 160, 1,                    // 28
                160, 1, 160, 1, 160, 320, 320, 1, 320, 1, 160, 1, 160, 1, 480, 320,     // 30
                320, 320, 1,                                        // 31
                320, 1, 160, 1, 160, 1, 480, 320,       // 32
                320, 320, 1,                                        // 33
                320, 320, 1, 160, 1,                    // 34
                160, 1, 160, 1, 160, 320, 320, 1                // 35
                                };

// global variables for sound output
volatile unsigned noteCount = 0;		// keep track of place in song
volatile unsigned int tonePeriod = 0;	// keep track of time relative to currrent tone
volatile unsigned int state = 0;		// current state of music player
volatile unsigned int prevState = 0;	// previous state of music player for comparison when pausing

void InitializeUART(void) {
        P1SEL |= RXD + TXD;  // P1.1 = RXD, P1.2 = TXD
        P1SEL2 |= RXD + TXD;  // P1.1 = RXD, P1.2 =
        UCA0CTL1 |= UCSSEL_2;
        UCA0BR1 = 0;
        UCA0BR0 = 104;
        UCA0MCTL = 2 * BRMOD;
        UCA0CTL1 &= ~UCSWRST;
        IE2 |= UCA0RXIE;
}
// set up sound system run by TA1
void init_timerA1(void)
 {
        // set up Timer A as frequency generator
        TA1CTL |= TACLR;        // reset clock
        TA1CTL = (TASSEL_2 +    // clock source = SMCLK
                        ID_0 +        // clock divider = 1
                        MC_2);        // UP mode
        P2SEL |= SPEAKER;       // Speaker on P2.0
        P2DIR |= SPEAKER;
}

int main(void) {
        // clocks
        WDTCTL = WDTPW + WDTHOLD;    // Stop WDT

        BCSCTL1 = CALBC1_1MHZ;  // 1Mhz calibration for clock
        DCOCTL = CALDCO_1MHZ;

        P1DIR |= GREEN;
        P1OUT &= ~(BIT6);

        InitializeUART();
        init_timerA1();

        __bis_SR_register(LPM0_bits + GIE);  // Enter LPM0, interrupts enabled

        while (1) {
                switch (Rx_Data) {
                case 0x4C:                                       // 'L' Command
                        fade_flag = 0;
                        state = 1;                       // play "Amazing Grace"
                        TA0CCTL0 &= ~CCIE;   // Disable Timer0 interrupts
                        P1SEL &= ~BIT6;      // P1.6 selected as GPIO
                        P1OUT |= BIT6;           // 1.6 set on
                        TA1CCTL0 |= OUTMOD_4;// set TA1CCR0 OUTMOD_4
                        TA1CCTL0 |= CCIE;    // compare mode, output 0, interrupt CCR0 enabled

                        break;

                case 0x4F:                                              // 'O' Command
                        fade_flag = 0;
                        state = 0;                      // sound off
                        TA1CCTL0 &= ~OUTMOD_4;  // toggle TA1CCR0 OUTMOD_4 to off, stop freq output
                        TA0CCTL0 &= ~CCIE;      // Disable Timer0 interrupts
                        TA1CCTL0 &= ~CCIE;      // Disable Timer1 interrupts
                        P1SEL &= ~BIT6;         // P1.6 selected as GPIO
                        P1OUT &= ~(BIT6);               // P1.6 set off
                        break;

                case 0x48:                      // 'H' Command
                        TA0CCTL0 &= ~CCIE;      // Disable Timer0 interrupts
                        TA1CCTL0 &= ~CCIE;              // Disable Timer1 interrupts
                        fade_flag = 1;                  //enabled PWM
                        state = 3;                              //play "Joy to the world"
                        TA0CCR0 |= halfPeriod;  //initialize half period
                        TA1CCTL0 |= OUTMOD_4;   // set TA1CCR0 OUTMOD_4
                        TA0CCTL1 = OUTMOD_7;    // TA0CCR1 reset/set
                        P1DIR |= LEDs;                  //
                        P1OUT = LEDs;                   // set LEDs as output
                        TA0CCTL0 |= CCIE;       // Enable Timer0 interrupts
                        TA1CCTL0 |= CCIE;               // Enable Timer1 interrupts
                        TA0CTL |= TASSEL_2 + MC_2;  // ACLK, Up Mode
                        break;

                case 0x4D:  // 'M'
                        TA0CCTL0 &= ~CCIE;              // Disable Timer0 interrupts
                        TA1CCTL0 &= ~CCIE;      // Disable Timer1 interrupts
                        state = 2;                              //play "Mamma Mia"
                        fade_flag = 0;                  // blink leds
                        TA0CCR0 = 10000;                // Timer counts to this value, which initiates ISR 128
                        TA1CCTL0 |= OUTMOD_4;   // set TA1CCR0
                        TA0CCTL1 |= OUTMOD_7;   // TA0CCR1 reset/set
                        TA0CCR1 = 0;                    // TA0CCR1 PWM initial duty cycle
                        TA0CCR0 &= ~CCIFG;      // clear interrupt flag
                        TA0CCTL0 |= CCIE;               // Enable Timer0 interrupts
                        TA1CCTL0 |= CCIE;               // Enable Timer1 interrupts
                        TA0CTL |= TASSEL_2 + MC_1; // SMCLK, contmode
                        break;

                default:
                        break;
                }
                // reset global vars to restart songs with state change
                if (state != prevState) {
                        tonePeriod = 0;                             // reset period
                        noteCount = 0;                // reset note index
                }
                __bis_SR_register(LPM0_bits); // Enter LPM0, interrupts enabled
        }
}

//*************** UART_RX INTERRUPT ******************
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
        Rx_Data = UCA0RXBUF; // Assign received character byte to Rx_Data
        __bic_SR_register_on_exit(LPM0_bits); // turn on CPU
}

//*************** TIMER A0 INTERRUPT ******************
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
        if (fade_flag == 1) //if want to use PMW
        {
                P1OUT ^= LEDs;
                if (P1OUT & LEDs)
                 {
                        halfPeriod++;           //increases halfperiod until it reaches the max
                        if (halfPeriod > maxHalfPeriod)
                        {
                                halfPeriod = minHalfPeriod; //resets halfperiod once reaches max
                        }
                        TA0CCR0 += halfPeriod; //increase value of TA0CCR0
                }

                else // decrease value of TA0CCR0
                {
                        TA0CCR0 += PERIOD - halfPeriod;
                }
        }

        else if (fade_flag == 0)//if want to blink LEDS
        {
                count++;
                if (count == 10) {
                        P1OUT ^= BIT6;     // P1.6 Toggle (blinking)
                        count = 0;
                }
        }
}

//*************** TIMER A1 INTERRUPT ******************
// used to output frequencies to P2.0 for sound
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0(void) {
                // switch sound output depending on state
        switch (state) {
        // high key obnoxious song
        case 1:
                TA1CCR0 += amazingNotes[noteCount];  // assign freq to TA1CCR0
                tonePeriod++;                                          // adjust period
                if (tonePeriod >= (amazingDurs[noteCount])) {
                        tonePeriod = 0;                    // reset to stay at minimum half-period
                        noteCount++;                             // set up next note
                }
                if (noteCount == (sizeof(amazingNotes) / sizeof(int))) {
                        noteCount = 0;                           // restart song for infinite obnoxious
                }
                break;
        // medium key obnoxious song
        case 2:
                TA1CCR0 += mammaNotes[noteCount];
                tonePeriod++;
                if (tonePeriod >= (mammaDurs[noteCount])) {
                        tonePeriod = 0;
                        noteCount++;
                }
                if (noteCount == (sizeof(mammaNotes) / sizeof(int))) {
                        noteCount = 0;
                }
                break;
        // high key obnoxious song
        case 3:
                TA1CCR0 += joyNotes[noteCount];
                tonePeriod++;
                if (tonePeriod >= (joyDurs[noteCount])) {
                        tonePeriod = 0;
                        noteCount++;
                }
                if (noteCount == (sizeof(joyNotes) / sizeof(int))) {
                        noteCount = 0;
                }
                break;

        }
}
