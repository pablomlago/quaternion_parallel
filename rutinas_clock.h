#ifndef __FUNC_LIB

#define __FUNC_LIB

/* Initialize the cycle counter */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

void start_counter();
double get_counter();
double mhz();

#endif
