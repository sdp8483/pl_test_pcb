/* Microcontroller Power Loss Detection
 *  Based on code and circuit from: https://www.youtube.com/watch?v=i2YsKacRZOs&t=392s
 *  
 *  Sam Perry
 *  June 2024 */

/* Includes ------------------------------------------------------------------ */
#include <Arduino.h>
#include <stdint.h>

#define _TASK_SCHEDULING_OPTIONS
#include "TaskScheduler.h"

#include "EEPROMBitSentinelWearLeveling.h"          
                                    /* eeprom wear leveling storage of interesting data to print on startup */

/* Define -------------------------------------------------------------------- */
#define RESET_EEPROM        false   /* reset eeprom, disable after 1 run */
#if (RESET_EEPROM)
#include <EEPROM.h>
#endif

#define EEPROM_COPIES               80      /* number of copies of data on EEPROM for wear leveling, make sure it will fit in EEPROM */

#define POWER_LOSS_DETECTION_RATE   10      /* rate in ms to check for power loss */
#define POWER_SENSE_PIN             A3      /* analog pin to sense for power loss */
#define POWER_SENSE_CAP_VALUE       920     /* analog value to charge sense cap to */
#define POWER_LOSS_ANALOG_VALUE     1000    /* analog read value of this indicates power loss */

/* Macro --------------------------------------------------------------------- */

/* Typedef ------------------------------------------------------------------- */
#pragma pack(1)
struct Record {                                     /* eeprom data storage structure */
    uint32_t hour_meter;                            /* total hours of on time */
    uint8_t minutes_meter;                          /* minutes of total hours on time */
    uint8_t seconds_meter;                          /* seconds of total hours on time */
    uint32_t power_cycles;                          /* total power cycles */
};

#pragma pack()

/* Globals ------------------------------------------------------------------- */
EEMEM uint8_t eepromBlock[sizeof(Record) * EEPROM_COPIES];  /* wear leveling eeprom storage block, 
                                                             * EEPROM_COPIES indicates how many storage blocks there are to level between.
                                                             * atmega328 has 1024Bytes of EEPROM */
EEPROMBitSentinelWearLeveling<Record, sizeof(eepromBlock), eepromBlock> eeprom;
Record record;                          /* eeprom data storage variable */

/* Function Prototypes ------------------------------------------------------- */
void print_info(void);          /* serial print fw info and eeprom data */
void (* resetFunc) (void) = 0;  /* software reset function for return from loss of external power */
void seconds_update(void);      /* increment seconds and set minute and hour meter */
void power_detection(void);     /* check for power loss, top up sensor capacitor */

/* Task Scheduler ------------------------------------------------------------ */
Scheduler ts;
Task seconds_task(1000, TASK_FOREVER, &seconds_update);
Task power_task(POWER_LOSS_DETECTION_RATE, TASK_FOREVER, &power_detection);

/* Setup --------------------------------------------------------------------- */
void setup() {
    #if (RESET_EEPROM)
    for (int i=0; i<EEPROM.length(); i++) {
        EEPROM.write(i, 0);
    }
    #endif

    /* read EEPROM data */
    if (eeprom.load(record)) {  /* eeprom data is valid */
        record.power_cycles++;  /* increment power cycles */
    } else {                    /* eeprom data is empty, setup init values */
        record.hour_meter = 0;
        record.minutes_meter = 0;
        record.seconds_meter = 0;
        record.power_cycles = 0;
    }

    /* setup serial for debug and info dump */
    Serial.begin(SERIAL_BAUD);
    print_info();

    ts.addTask(seconds_task);
    ts.addTask(power_task);

    seconds_task.enable();
    power_task.enable();
}

/* Main Loop ----------------------------------------------------------------- */
void loop() {
    ts.execute();               /* run task scheduler */

}

/* serial print fw info and eeprom data */
void print_info(void) {
    /* print info to uart */
    Serial.println();
    Serial.println(F(DEVICE_NAME));
    Serial.print(F("FW: "));
    Serial.println(F(FW_VERSION));
    Serial.print(F(__DATE__));
    Serial.print(F(" "));
    Serial.println(F(__TIME__));
    Serial.println();

    Serial.print(F("Power Cycles: "));
    Serial.println(record.power_cycles);
    Serial.print(F("Runtime: "));
    Serial.print(record.hour_meter);
    Serial.print(F(":"));
    if (record.minutes_meter < 10) {
        Serial.print(F("0"));
    }
    Serial.print(record.minutes_meter);
    Serial.print(F(":"));
    if (record.seconds_meter < 10) {
        Serial.print(F("0"));
    }
    Serial.println(record.seconds_meter);
    Serial.println();
}


/* update hour meter variables */
void seconds_update(void) {
    /* increment seconds count */
    record.seconds_meter++;

    /* increment minutes count if seconds are at 60 */
    if (record.seconds_meter == 60) {
        record.seconds_meter = 0;
        record.minutes_meter++;
    }

    /* increment hour count if minutes are at 60 */
    if (record.minutes_meter == 60) {
        record.minutes_meter = 0;
        record.hour_meter++;
    }
}

void power_detection(void) {
    pinMode(POWER_SENSE_PIN, INPUT);    /* set so that analog read can happen */
    uint16_t val = analogRead(POWER_SENSE_PIN);

    // Serial.println(val);

    /* charge sense cap to about 90% of max analog value */
    if (val < POWER_SENSE_CAP_VALUE) {
        pinMode(POWER_SENSE_PIN, OUTPUT);
        digitalWrite(POWER_SENSE_PIN, HIGH);
        return; /* let cap charge for at least 10ms */
    }

    /* detect if power has been lost
     *    aref is derived from 5V power rail
     *    in power loss this 5V rail will drop below charge on sense cap resulting in aread of > initial charge value */
    if (val > POWER_LOSS_ANALOG_VALUE) {
        eeprom.save(record);
        digitalWrite(LED_BUILTIN, HIGH);
    }
}