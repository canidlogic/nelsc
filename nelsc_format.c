/*
 * nelsc_format.c
 * 
 * Implementation of nelsc_format.h
 * 
 * See the header for further information.
 */

#include "nelsc_format.h"
#include <stdlib.h>

/*
 * The number of days in a week.
 */
#define DAYS_PER_WEEK 7

/*
 * Number of months in a short year of twelve months.
 */
#define MONTHS_PER_SHORT_YEAR 12

/*
 * Number of months in a long year of thirteen months.
 */
#define MONTHS_PER_LONG_YEAR 13

/*
 * Number of days in a short month of four weeks.
 */
#define DAYS_PER_SHORT_MONTH 28

/*
 * Number of days in a long month of five weeks.
 */
#define DAYS_PER_LONG_MONTH 35

/*
 * Number of weeks in a short month of four weeks.
 */
#define WEEKS_PER_SHORT_MONTH 4

/*
 * Number of weeks in a long month of five weeks.
 */
#define WEEKS_PER_LONG_MONTH 5

/*
 * The number of characters in a formatted day of month.
 */
#define MONTHDAY_CHARS 3

/*
 * The character offset within a NELSC date of the separator that keeps
 * the year and month apart from each other.
 */
#define DATESEP_YEAR_OFFS 2

/*
 * The character offset within a NELSC date of the separator that keeps
 * the week and day apart from each other.
 */
#define DATESEP_WEEK_OFFS 5

/*
 * The character that separates the year from the month within a NELSC
 * date.
 */
#define DATESEP_YEAR ':'

/*
 * The character that separates the week from the day within a NELSC
 * date.
 */
#define DATESEP_WEEK '-'

/*
 * The character offset of the year field within a NELSC date.
 */
#define DATEFIELD_YEAR 0

/*
 * The character offset of the month field within a NELSC date.
 */
#define DATEFIELD_MONTH 3

/*
 * The character offset of the week field within a NELSC date.
 */
#define DATEFIELD_WEEK 4

/*
 * The character offset of the day field within a NELSC date.
 */
#define DATEFIELD_DAY 6

/*
 * nelsc_format_printDate function.
 */
void nelsc_format_printDate(
		FILE *pFile,
		int32_t y,
		int32_t m,
		int32_t d) {
	
	int week_digit = 0;
	int day_digit = 0;
	
	int32_t abs_month = 0;
	
	/* Check parameters */
	if ((pFile == NULL) ||
		(y < NELSC_CYCLE_YEARMIN) || (y > NELSC_CYCLE_YEARMAX) ||
		(m < 0) || (d < 0)) {
		abort();
	}
	
	/* Check upper bound of month, depending on whether year is long or
	 * short */
	if (nelsc_cycle_isLongYear(y)) {
		if (m >= MONTHS_PER_LONG_YEAR) {
			abort();
		}
	} else {
		if (m >= MONTHS_PER_SHORT_YEAR) {
			abort();
		}
	}
	
	/* Determine the absolute month */
	abs_month = nelsc_cycle_yearToMonth(y);
	abs_month += m;
	
	/* Check upper bound of day offset, depending on whether month is
	 * long or short */
	if (nelsc_cycle_isLongMonth(abs_month)) {
		if (d >= DAYS_PER_LONG_MONTH) {
			abort();
		}
	} else {
		if (d >= DAYS_PER_SHORT_MONTH) {
			abort();
		}
	}
	
	/* Print the year */
	base24_printPair(pFile, y);
	
	/* Print the month separator and the month */
	if (fprintf(pFile, ":%c", (int) base24_intToDigit(m + 1)) != 2) {
		abort();
	}
	
	/* Split day offset in week and day */
	week_digit = (int) (d / DAYS_PER_WEEK);
	day_digit = (int) (d % DAYS_PER_WEEK);
	
	/* Increment both digits so that they are one-based */
	week_digit++;
	day_digit++;
	
	/* Print the three characters */
	if (fprintf(pFile, "%d-%d", week_digit, day_digit)
					!= MONTHDAY_CHARS) {
		abort();
	}
}

/*
 * nelsc_format_scanDate function.
 */
bool nelsc_format_scanDate(const char *str, int32_t *pOffset) {
	
	bool result = true;
	int32_t x = 0;
	
	int32_t s_year = 0;
	int32_t s_month = 0;
	int32_t s_week = 0;
	int32_t s_day = 0;
	
	int32_t abs_month = 0;
	int32_t offs = 0;
	
	/* Fail if str is NULL */
	if (str == NULL) {
		result = false;
	}
	
	/* Fail if a null termination character occurs within the first
	 * NELSC_FORMAT_DATE_LENGTH characters of str */
	if (result) {
		for(x = 0; x < NELSC_FORMAT_DATE_LENGTH; x++) {
			if (str[x] == 0) {
				result = false;
				break;
			}
		}
	}
	
	/* Fail if the separator characters are not in the proper
	 * positions */
	if (result) {
		if ((str[DATESEP_YEAR_OFFS] != DATESEP_YEAR) ||
			(str[DATESEP_WEEK_OFFS] != DATESEP_WEEK)) {
			result = false;
		}
	}
	
	/* Read each of the individual fields of the date */
	if (result) {
		if (!base24_pairToInt(&(str[DATEFIELD_YEAR]), &s_year)) {
			result = false;
		}
		
		s_month = base24_digitToInt(str[DATEFIELD_MONTH]);
		s_week  = base24_digitToInt(str[DATEFIELD_WEEK ]);
		s_day   = base24_digitToInt(str[DATEFIELD_DAY  ]);
		
		if ((s_month == -1) || (s_week == -1) || (s_day == -1)) {
			result = false;
		}
	}
	
	/* Convert the year into an absolute month offset of the first month
	 * of the year; the year should already be in range because every
	 * valid base-24 pair is also a valid year */
	if (result) {
		abs_month = nelsc_cycle_yearToMonth(s_year);
	}
	
	/* Verify that the month field is in range, taking into account the
	 * length of the year, convert it to a zero-based offset, and add it
	 * to the absolute month offset of the first month of the year to
	 * yield the absolute month offset of the date */
	if (result) {
		if (nelsc_cycle_isLongYear(s_year)) {
			/* Long year */
			if ((s_month < 1) || (s_month > MONTHS_PER_LONG_YEAR)) {
				result = false;
			}
		} else {
			/* Short year */
			if ((s_month < 1) || (s_month > MONTHS_PER_SHORT_YEAR)) {
				result = false;
			}
		}
	}
	
	if (result) {
		abs_month += (s_month - 1);
	}
	
	/* Set the day offset to the first day of the month */
	if (result) {
		offs = nelsc_cycle_monthToDay(abs_month);
	}
	
	/* Verify that the week field is in range, taking into account the
	 * length of the month, convert it to a zero-based offset, and add
	 * the corresponding number of days to the day offset */
	if (result) {
		if (nelsc_cycle_isLongMonth(abs_month)) {
			/* Long month */
			if ((s_week < 1) || (s_week > WEEKS_PER_LONG_MONTH)) {
				result = false;
			}
		} else {
			/* Short month */
			if ((s_week < 1) || (s_week > WEEKS_PER_SHORT_MONTH)) {
				result = false;
			}
		}
	}
	
	if (result) {
		offs += ((s_week - 1) * DAYS_PER_WEEK);
	}
	
	/* Verify that the day field is in range, convert it to a zero-based
	 * offset, and add to the day offset to yield the NELSC absolute day
	 * offset of the date */
	if (result) {
		if ((s_day < 1) || (s_day > DAYS_PER_WEEK)) {
			result = false;
		}
	}
	
	if (result) {
		offs += (s_day - 1);
	}
	
	/* If a pointer to a return field was provided, store the result */
	if (result) {
		if (pOffset != NULL) {
			*pOffset = offs;
		}
	}
	
	/* Return status */
	return result;
}
