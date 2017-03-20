#ifndef NELSC_CYCLE_H_INCLUDED
#define NELSC_CYCLE_H_INCLUDED

/*
 * nelsc_cycle.h
 * 
 * Provides the basic NELSC cycle conversion functions.  NELSC has three
 * independent cycle layers -- absolute days, absolute months, and
 * years.  The functions in this module convert between days and months
 * and between months and years.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The NELSC absolute day offset of the first day in NELSC.
 */
#define NELSC_CYCLE_DAYMIN (-35364)

/*
 * The NELSC absolute day offset of the last day in NELSC.
 */
#define NELSC_CYCLE_DAYMAX (175020)

/*
 * The NELSC absolute month offset of the first month in NELSC.
 */
#define NELSC_CYCLE_MONMIN (-1197)

/*
 * The NELSC absolute month offset of the first month in NELSC.
 */
#define NELSC_CYCLE_MONMAX (5926)

/*
 * The earliest year in NELSC.
 */
#define NELSC_CYCLE_YEARMIN (-96)

/*
 * The latest year in NELSC.
 */
#define NELSC_CYCLE_YEARMAX (479)

/*
 * The offset in days since proleptic Gregorian date 1200-03-01 to get
 * from that date to NELSC absolute day offset zero in the reference
 * time zone.  Useful for converting between NELSC and Gregorian
 * calendar dates.
 */
#define NELSC_CYCLE_GROFFS (264773)

/*
 * Convert a NELSC absolute day offset into a NELSC absolute month
 * offset of the month that includes the day.
 * 
 * If pOffset is not NULL, then the day offset within the month is
 * written to that variable.  Offset zero means the first day of the
 * month.
 * 
 * The provided day offset must be in range NELSC_CYCLE_DAYMIN to
 * NELSC_CYCLE_DAYMAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   d - the NELSC absolute day offset to convert
 * 
 *   pOffset - pointer to the variable to receive the day offset within
 *   the returned month, or NULL
 * 
 * Return:
 * 
 *   the NELSC absolute month offset that the provided day occurs within
 * 
 * Faults:
 * 
 *   - If d is out of range
 */
int32_t nelsc_cycle_dayToMonth(int32_t d, int32_t *pOffset);

/*
 * Convert a NELSC absolute month offset into a NELSC absolute day
 * offset that refers to the first day of the month.
 * 
 * The provided month offset must be in range NELSC_CYCLE_MONMIN to
 * NELSC_CYCLE_MONMAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   m - the NELSC absolute month offset to convert
 * 
 * Return:
 * 
 *   the NELSC absolute day offset of the first day of this month
 * 
 * Faults:
 * 
 *   - If m is out of range
 */
int32_t nelsc_cycle_monthToDay(int32_t m);

/*
 * Convert a NELSC absolute month offset into a NELSC year that includes
 * the month.
 * 
 * If pOffset is not NULL, then the month offset within the year is
 * written to that variable.  Offset zero means the first month of the
 * year.
 * 
 * The provided month offset must be in range NELSC_CYCLE_MONMIN to
 * NELSC_CYCLE_MONMAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   M - the NELSC absolute month offset to convert
 * 
 *   pOffset - pointer to the variable to receive the month offset
 *   within the returned year, or NULL
 * 
 * Return:
 * 
 *   the NELSC year that the provided month occurs within
 * 
 * Faults:
 * 
 *   - If m is out of range
 */
int32_t nelsc_cycle_monthToYear(int32_t m, int32_t *pOffset);

/*
 * Convert a NELSC year into a NELSC absolute month offset that refers
 * to the first month of the year.
 * 
 * The provided year must be in range NELSC_CYCLE_YEARMIN to 
 * NELSC_CYCLE_YEARMAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   y - the NELSC year to convert
 * 
 * Return:
 * 
 *   the NELSC absolute month offset of the first month of this year
 * 
 * Faults:
 * 
 *   - If y is out of range
 */
int32_t nelsc_cycle_yearToMonth(int32_t y);

/*
 * Determine whether the month indicated by a provided NELSC absolute
 * month offset is a long month or a short month.
 * 
 * The provided month must be in range NELSC_CYCLE_MONMIN to
 * NELSC_CYCLE_MONMAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   m - the NELSC absolute month offset
 * 
 * Return:
 * 
 *   true if the month is long, false if short
 * 
 * Faults:
 * 
 *   - If m is out of range
 */
bool nelsc_cycle_isLongMonth(int32_t m);

/*
 * Determine whether the given NELSC year is a long year with 13 months
 * or a short year with 12 months.
 * 
 * The provided year must be in range NELSC_CYCLE_YEARMIN to
 * NELSC_CYCLE_YEARMAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   y - the NELSC year
 * 
 * Return:
 * 
 *   true if year is long, false if short
 * 
 * Faults:
 * 
 *   - If y is out of range
 */
bool nelsc_cycle_isLongYear(int32_t y);

#endif
