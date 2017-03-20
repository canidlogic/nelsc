#ifndef NELSC_FORMAT_H_INCLUDED
#define NELSC_FORMAT_H_INCLUDED

/*
 * nelsc_format.h
 * 
 * Provides functions for handling NELSC information formatted as
 * strings.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "base24.h"
#include "nelsc_cycle.h"

/*
 * The number of characters in a formatted NELSC date.
 */
#define NELSC_FORMAT_DATE_LENGTH 7

/*
 * Print a formatted NELSC date to the given file in ASCII format.
 * 
 * The formatted date is of the form "3T:C4-5", where "3T" is the year,
 * "C" is the month within the year (first month is one), "4" is the
 * week within the month (first week is one), and "5" is the day within
 * the week (first day is one).
 * 
 * The m and d arguments are zero-indexed, so the first month of the
 * year is zero and the first day of the month is zero.  The day offset
 * within the month will automatically be converted into a week/day of
 * week combination.
 * 
 * The provided year, month, day combination must be valid within NELSC.
 * If the thirteenth month of a short year is specified or a day in the
 * fifth week of a short month, a fault occurs.
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
 *   - If y, m, and d are not a valid NELSC combination of year, month,
 *     and day in month
 * 
 *   - If writing the characters to the file fails
 */
void nelsc_format_printDate(
		FILE *pFile,
		int32_t y,
		int32_t m,
		int32_t d);

/*
 * Read a formatted NELSC date from an ASCII string.
 * 
 * The date must be formatted according to the convention "3T:C4-7"
 * where "3T" is the year, "C" is the month, "4" is the week of the
 * month, and "7" is the day of the week.  The month, week of the month,
 * and day of the week are one-based, so the first month is one, the
 * first week is one, and first day of the week is one.
 * 
 * NELSC dates always have exactly NELSC_FORMAT_DATE_LENGTH characters,
 * so if a date was successfully read, that is how many characters must
 * be advanced to go beyond the characters in the date.  This function
 * will immediately stop reading the string and fail if it encounters a
 * null termination character, so this function may be safely used with
 * null-terminated strings without worrying about reading past the end
 * of the string.
 * 
 * This function will not skip over leading whitespace, so if the
 * provided pointer points to a whitespace character, this function will
 * fail.  Note that the function won't read more than
 * NELSC_FORMAT_DATE_LENGTH characters, so if there is a non-whitespace
 * character immediately after the date, this function will completely
 * ignore that.
 * 
 * If the conversion is successful, true is returned, and the NELSC
 * absolute day offset indicated by the provided date is written to
 * *pOffset, if pOffset is not NULL.  (Passing a NULL pOffset can be
 * useful for just checking whether a valid NELSC date is present.)  If
 * the conversion fails, false is returned.
 * 
 * This function will apply all the NELSC rules to verify the specific
 * date is valid.  So, for example, if the fifth week is specified for
 * a short month, this function will fail.  The function will also fail
 * if the thirteenth month is specified for a short year.
 * 
 * Parameters:
 * 
 *   str - pointer to the ASCII-formatted NELSC date to parse
 * 
 *   pOffset - pointer to the variable to receive the parsed NELSC day
 *   offset if the function succeeds, or NULL
 * 
 * Return:
 * 
 *   true if conversion is successful, false if the characters at str
 *   don't form a valid NELSC date or str is NULL
 * 
 * Undefined behavior:
 * 
 *   - If the string at *str is not null-terminated
 */
bool nelsc_format_scanDate(const char *str, int32_t *pOffset);

#endif
