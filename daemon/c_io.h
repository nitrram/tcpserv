#pragma once
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	int print_mem_stats(FILE *);
	int print_cpu_stats(FILE *);

#ifdef __cplusplus
}
#endif

/** Fills the int value array from the 1st line from '/proc/stat'.
 * The array elements are CPU statistics as follows:
 * 1) user
 * 2) nice
 * 3) system
 * 4) idle
 * 5) nowait
 * 6) irq
 * 7) softirq
 * 8) steal
 * 9) guest
 * 10) guest_nice
 * \param map Array of the CPU stats
 * \return Error code. Which is 0 if ending correctly.
 */
int get_cpu_stats(int *const map);
