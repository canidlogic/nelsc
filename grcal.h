#ifndef GRCAL_H_INCLUDED
#define GRCAL_H_INCLUDED

/*
 * grcal.h
 * 
 * Provides functions for working with Gregorian calendar dates.  This
 * only provides the core Gregorian functions of moving between counts
 * of days and year-month-day dates.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * The minimum valid Gregorian day offset.
 * 
 * This corresponds to 1582-10-15, when the calendar was first put into
 * effect.  Dates prior to that used a different (but related) Julian
 * calendar system.
 */
#define GRCAL_DAY_MIN 139750

/*
 * The maximum valid Gregorian day offset.
 * 
 * This corresponds to 9999-12-31, after which the four-digit year
 * format will overflow.
 */
#define GRCAL_DAY_MAX 3214073

/*
 * Convert a Gregorian day offset into the year, month, and day of
 * month.
 * 
 * The month and day of month are one-indexed, so the first month is one
 * and the first day of the month is one.
 * 
 * The provided Gregorian day offset must be in range GRCAL_DAY_MIN to
 * GRCAL_DAY_MAX (inclusive of boundaries).
 * 
 * The Gregorian day offset system is defined such that day zero
 * corresponds to 1200-03-01 in the proleptic Gregorian calendar.  Note
 * that 1200-03-01 is out of the range of allowable dates, since it
 * occurs centuries before the adoption of the Gregorian calendar.  This
 * date was chosen as day zero because it is easier for date
 * calculations that way.
 * 
 * Parameters:
 * 
 *   offs - the Gregorian day offset to convert
 * 
 *   pYear - pointer to the variable to receive the Gregorian year, or
 *   NULL
 * 
 *   pMonth - pointer to the variable to receive the Gregorian month, or
 *   NULL
 * 
 *   pDayOfMonth - pointer to the variable to receive the Gregorian day
 *   of the month, or NULL
 * 
 * Faults:
 * 
 *   - If offs is out of range
 */
void grcal_offsetToDate(
		int32_t offs,
		int32_t *pYear,
		int32_t *pMonth,
		int32_t *pDayOfMonth);

/*
 * Convert a Gregorian date into a Gregorian day offset.
 * 
 * The month and day are one-indexed, so the first month is one and the
 * first day of the month is one.
 * 
 * The Gregorian day offset will be written to *pOffs if the function is
 * successful.  If NULL, the value is not written.  This can be useful
 * for merely checking whether a year-month-day combination is valid.
 * 
 * If the provided year-month-day combination is not valid in the
 * Gregorian calendar, the function will fail and false will be
 * returned.
 * 
 * Parameters:
 * 
 *   pOffs - pointer to the variable to receive the converted Gregorian
 *   day offset, or NULL
 * 
 *   year - the Gregorian year
 * 
 *   month - the month of the year
 * 
 *   dayofmonth - the day of the month
 * 
 * Return:
 * 
 *   true if successful, false if provided year-month-day combination is
 *   not valid
 */
bool grcal_dateToOffset(
		int32_t *pOffs,
		int32_t year,
		int32_t month,
		int32_t dayofmonth);

/*
 * Print a formatted Gregorian date in YYYY-MM-DD format to the given
 * file in ASCII format.
 * 
 * The m and d arguments are one-indexed, so the first month of the year
 * is one and the first day of the month is one.
 * 
 * The provided year, month, day combination must be valid within the
 * Gregorian calendar.  A fault occurs if an invalid date is specified.
 * 
 * Parameters:
 * 
 *   pFile - the file to write to
 * 
 *   y - the year
 * 
 *   m - the month of the year
 * 
 *   d - the day of the month
 * 
 * Faults:
 * 
 *   - If pFile is NULL
 * 
 *   - If y, m, and d are not a valid Gregorian combination of year,
 *     month, and day
 * 
 *   - If writing the characters to the file fails
 */
void grcal_printDate(
		FILE *pFile,
		int32_t y,
		int32_t m,
		int32_t d);

/*
 * Parse a formatted Gregorian date in YYYY-MM-DD format in a given
 * ASCII string.
 * 
 * The return value will be a Gregorian day offset, or -1 if the parsing
 * failed.  If the parsing succeeds and ppTrail is not NULL, *ppTrail
 * will be set to point to the character immediately following the date
 * that was parsed.
 * 
 * The YYYY field must be exactly four decimal digits.  MM and DD can be
 * either exactly one or exactly two decimal digits.  Note that if an
 * ASCII decimal digit immediately follows the Gregorian date, it will
 * be scanned as part of the Gregorian date, either causing an incorrect
 * result or causing the parsing to fail.
 * 
 * This function can be safely used with null-terminated ASCII strings
 * without worrying about it reading past the terminating NULL.  This
 * function does not skip leading whitespace, so it will fail if str
 * points to a whitespace character.
 * 
 * The function will fail if the provided year-month-day combination is
 * not valid in the Gregorian calendar system, or if it is out of range
 * of the Gregorian day offset (GRCAL_DAY_MIN to GRCAL_DAY_MAX).
 * 
 * Parameters:
 * 
 *   str - the string to parse
 * 
 *   ppTrail - pointer to the pointer to be set to the character
 *   following the date on successful return, or NULL
 * 
 * Return:
 * 
 *   the Gregorian day offset, or -1 if parsing failed or str is NULL
 * 
 * Undefined behavior:
 * 
 *   - If the provided string is not null-terminated
 */
int32_t grcal_scanDate(const char *str, const char **ppTrail);

#endif
