/* Includes ------------------------------------------------------------------ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
// #include <avr/delay.h>
#include <stdint.h>

#include "clock.h"                                  /* set main clock based on platformio.ini f_cpu value */
#include "rtc.h"                                    /* rtc for second timekeeping and wake from sleep */
#include "usart.h"                                  /* print device info on startup */

/* Define -------------------------------------------------------------------- */
#define LED_PORT    PORTB
#define LED_PIN     PIN5_bm

/* Macro --------------------------------------------------------------------- */

/* Typedef ------------------------------------------------------------------- */

/* Globals ------------------------------------------------------------------- */
volatile uint16_t seconds = 0;                      /* track seconds using RTC */

/* Function Prototypes ------------------------------------------------------- */

/* Main ---------------------------------------------------------------------- */
int main(void) {
    clock_init();                                   /* Set main clock frequency to f_cpu */
    rtc_init();                                     /* 1sec and 0.5 second tick, wake from sleep and track seconds */

    LED_PORT.OUTCLR = LED_PIN;
    LED_PORT.DIRSET = LED_PIN;
    LED_PORT.OUTSET = LED_PIN;                      /* set LED pin high, LED is off */

    Serial.begin(9600);                             /* init uart */

    /* print info to uart */
    _delay_ms(2000);
    Serial.println();
    Serial.println(DEVICE_NAME);
    Serial.print("FW: ");
    Serial.println(FW_VERSION);
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);

    /* print serial number */
    Serial.print("SN: ");
    const char *snPt = (char *) &SIGROW.SERNUM0;
    for (int8_t i=0; i<10; i++) {
        Serial.print(snPt[i], HEX);
    }
    Serial.println();

    while(1) {
        sleep_enable();
        sei();                                      /* enable global interrupts */
        sleep_cpu();                                /* sleep during idle */
        sleep_disable();                            /* wake from sleep here */

        LED_PORT.OUTTGL = LED_PIN;
    }
}

/* RTC interrupt service routine */
ISR(RTC_CNT_vect) {
    if (RTC.INTFLAGS & RTC_OVF_bm) {
        seconds++;                                  /* increment seconds count */
        RTC.INTFLAGS |= RTC_OVF_bm;                 /* clear interrupt flag */
    }

    if (RTC.INTFLAGS & RTC_CMP_bm) {
        RTC.INTFLAGS |= RTC_CMP_bm;                 /* clear interrupt flag */
    }
}