#ifndef INC_CLOCK_H
#define INC_CLOCK_H

/* Includes ------------------------------------------------------------------ */
#include <avr/io.h>
#include <stdint.h>

/* Defines ------------------------------------------------------------------- */

/* Macros -------------------------------------------------------------------- */

/* typedef ------------------------------------------------------------------- */

/* Globals ------------------------------------------------------------------- */

/* External Globals ---------------------------------------------------------- */

/* Function Prototypes ------------------------------------------------------- */
void clock_init(void);  /* Set main clock frequency to f_cpu set in platformio.ini */

/* Class --------------------------------------------------------------------- */

#endif