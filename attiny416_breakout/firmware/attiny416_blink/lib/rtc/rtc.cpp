#include "rtc.h"

void rtc_init(void) {
    /* wait for all registers to be sychronized */
    while (RTC.STATUS > 0);

    RTC.CLKSEL |= RTC_CLKSEL_INT32K_gc;

    RTC.INTCTRL |= RTC_OVF_bm;          /* interrupt when RTC.CNT equals RTC.PER and wraps around to zero */
    RTC.INTCTRL |= RTC_CMP_bm;          /* interrupt when RTC.CNT equals RTC.CMP */

    while (RTC.STATUS & RTC_PERBUSY_bm);
    RTC.PER = ONE_SECOND_COUNTS;        /* one second */

    while (RTC.STATUS & RTC_CMPBUSY_bm);
    RTC.CMP = HALF_SECOND_COUNTS;       /* half second */

    RTC.CTRLA |= (RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm);

}