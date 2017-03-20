/*
 * nelsc_cycle.c
 * 
 * Implementation of nelsc_cycle.h
 * 
 * See the header for further information.
 */

#include "nelsc_cycle.h"
#include <stdlib.h>

/*
 * Number of days in a short month of four weeks.
 */
#define DAYS_PER_SHORT_MONTH 28

/*
 * Number of days in a long month of five weeks.
 */
#define DAYS_PER_LONG_MONTH 35

/*
 * Number of months in a short year of twelve months.
 */
#define MONTHS_PER_SHORT_YEAR 12

/*
 * Number of months in a long year of thirteen months.
 */
#define MONTHS_PER_LONG_YEAR 13

/*
 * The number of days in a 32-month pattern.
 */
#define DAYS_PER_MONTH_PATTERN 945

/*
 * Number of months in a 32-month pattern.
 */
#define MONTH_PATTERN_LENGTH 32

/*
 * The number of months in an 11-year span.
 */
#define MONTHS_PER_YEAR_SPAN 136

/*
 * The number of months in a 231-year pattern of 11-year spans.
 */
#define MONTHS_PER_YEAR_PATTERN 2857

/*
 * Number of years in an 11-year span.
 */
#define YEAR_SPAN_LENGTH 11

/*
 * Number of years in a 231-year pattern.
 */
#define YEAR_PATTERN_LENGTH 231

/*
 * The offset in days from the first day of the year zero until absolute
 * day zero.
 */
#define ABSOLUTE_DAY_OFFSET 308

/*
 * The offset in months from the first month of the year zero until
 * absolute month zero.
 */
#define ABSOLUTE_MONTH_OFFSET 10

/*
 * When a negative day offset is encountered when converting up to
 * months, boost by this many days to make it non-negative and still
 * aligned with the 32-month pattern.
 */
#define DAY_UP_BOOST 35910

/*
 * If a negative day offset was boosted before converting to an absolute
 * month offset, sink the resulting month offset by this many months to
 * get the correct month offset.
 */
#define MONTH_UP_SINK 1216

/*
 * Month offsets are *always* boosted by this many months when
 * converting up to years.  This also eliminates negative offsets.  This
 * set the start of the first 231-year pattern 121 years before year
 * zero, putting year zero just about in the middle of a 231-year
 * pattern.
 */
#define MONTH_UP_BOOST 1496

/*
 * Year offsets are *always* sunk by this many years after converting
 * from (boosted) months up to years.
 */
#define YEAR_UP_SINK 121

/*
 * Years offsets are *always* boosted by this many years before
 * converting down to months, which also eliminates negative years.
 */
#define YEAR_DOWN_BOOST YEAR_UP_SINK

/*
 * Months offsets are *always* sunk by this many months after converting
 * down from (boosted) years.
 */
#define MONTH_DOWN_SINK MONTH_UP_BOOST

/*
 * When a negative month is encountered when converting down to days,
 * boost by this many months to make it non-negative and still aligned
 * with the 32-month pattern.
 */
#define MONTH_DOWN_BOOST MONTH_UP_SINK

/*
 * If a negative month was boosted before converting to days, sink the
 * resulting day offset by this many days to get the correct day.
 */
#define DAY_DOWN_SINK DAY_UP_BOOST

/*
 * The 32-month pattern.  This is a null-terminated string of 32 "S" and
 * "L" characters, where "S" refers to short months and "L" refers to
 * long months.
 */
static const char *m_month_pattern =
	"SSLS" "SSSLS"
	"SSLS" "SSSLS"
	"SSLS" "SSSLS"
	       "SSSLS";

/*
 * The 11-year span pattern.  This is a null-terminated string of 11 "S"
 * and "L" characters, where "S" refers to short years and "L" refers to
 * long years.
 * 
 * However, the eleventh year is long instead of short at the end of
 * every 231-year pattern.
 */
static const char *m_year_span =
	"SL"  "SL"
	"SSL" "SSL" "S";

/* Function prototypes */
static bool charToBool(char c);

/*
 * Given a character from a pattern string that is either "S" or "L"
 * (case sensitive), return true if it is "L" or false if it is "S".  If
 * it is any other character (including null), a fault occurs.
 * 
 * Parameters:
 * 
 *   c - the character to convert
 * 
 * Return:
 * 
 *   true if long, false if short
 * 
 * Faults:
 * 
 *   - If c is neither "S" nor "L"
 */
static bool charToBool(char c) {
	bool result = false;
	
	if (c == 'L') {
		result = true;
	} else if (c == 'S') {
		result = false;
	} else {
		abort();
	}
	
	return result;
}

/*
 * nelsc_cycle_dayToMonth function.
 */
int32_t nelsc_cycle_dayToMonth(int32_t d, int32_t *pOffset) {
	
	bool boosted = false;
	int32_t month_count = 0;
	const char *pc = NULL;
	bool mlong = false;
	
	/* Check parameter */
	if ((d < NELSC_CYCLE_DAYMIN) || (d > NELSC_CYCLE_DAYMAX)) {
		abort();
	}
	
	/* Undo absolute offset so that day offset is relative to the first
	 * day of the year zero */
	d += ABSOLUTE_DAY_OFFSET;
	
	/* If day offset is negative, boost it to non-negative and set flag
	 * so we remember to sink the result later */
	if (d < 0) {
		d += DAY_UP_BOOST;
		boosted = true;
	}

	/* Count all complete 32-month patterns */
	month_count = (d / DAYS_PER_MONTH_PATTERN) * MONTH_PATTERN_LENGTH;
	d = d % DAYS_PER_MONTH_PATTERN;

	/* While there are still days remaining, go through the month
	 * pattern until we've reached the month the day offset is within */
	pc = m_month_pattern;
	while (d > 0) {
		/* Determine whether the current month in the pattern is long or
		 * short */
		mlong = charToBool(*pc);

		/* If day offset is within current month, then break out of the
		 * loop */
		if (( mlong && (d < DAYS_PER_LONG_MONTH )) ||
			(!mlong && (d < DAYS_PER_SHORT_MONTH))) {
			break;
		}
		
		/* Otherwise, increase month count, decrease day offset, and
		 * move to next month in the pattern */
		month_count++;
		
		if (mlong) {
			d -= DAYS_PER_LONG_MONTH;
		} else {
			d -= DAYS_PER_SHORT_MONTH;
		}
		
		pc++;
	}
	
	/* If we boosted a negative day offset earlier, sink the month
	 * count */
	if (boosted) {
		month_count -= MONTH_UP_SINK;
	}

	/* Apply the absolute month offset */
	month_count -= ABSOLUTE_MONTH_OFFSET;
	
	/* Return the remainder day offset, if it was requested */
	if (pOffset != NULL) {
		*pOffset = d;
	}
	
	/* Return the absolute month offset */
	return month_count;
}

/*
 * nelsc_cycle_monthToDay function.
 */
int32_t nelsc_cycle_monthToDay(int32_t m) {
	
	bool boosted = false;
	int32_t day_count = 0;
	int32_t i = 0;
	bool mlong = false;
	
	/* Check parameter */
	if ((m < NELSC_CYCLE_MONMIN) || (m > NELSC_CYCLE_MONMAX)) {
		abort();
	}
	
	/* Undo absolute offset so that month offset is relative to the
	 * first month of the year zero */
	m += ABSOLUTE_MONTH_OFFSET;
	
	/* If month offset is negative, boost it to non-negative and set
	 * flag so we remember to sink the result later */
	if (m < 0) {
		m += MONTH_DOWN_BOOST;
		boosted = true;
	}

	/* Count all complete 32-month patterns */
	day_count = (m / MONTH_PATTERN_LENGTH) * DAYS_PER_MONTH_PATTERN;
	m = m % MONTH_PATTERN_LENGTH;

	/* Use the month pattern to count up how many days there are in the
	 * remaining months */
	for(i = 0; i < m; i++) {
		/* Determine whether the current month in the pattern is long or
		 * short */
		mlong = charToBool(m_month_pattern[i]);

		/* Increase the day count appropriately */
		if (mlong) {
			day_count += DAYS_PER_LONG_MONTH;
		} else {
			day_count += DAYS_PER_SHORT_MONTH;
		}
	}
	
	/* If we boosted a negative month offset earlier, sink the day
	 * count */
	if (boosted) {
		day_count -= DAY_DOWN_SINK;
	}

	/* Apply the absolute day offset */
	day_count -= ABSOLUTE_DAY_OFFSET;
	
	/* Return the absolute day offset */
	return day_count;
}

/*
 * nelsc_cycle_monthToYear function.
 */
int32_t nelsc_cycle_monthToYear(int32_t m, int32_t *pOffset) {
	
	bool force13 = false;
	int32_t year_count = 0;
	const char *pc = NULL;
	bool mlong = false;
	
	/* Check parameter */
	if ((m < NELSC_CYCLE_MONMIN) || (m > NELSC_CYCLE_MONMAX)) {
		abort();
	}
	
	/* Undo absolute offset so that month offset is relative to the
	 * first month of the year zero */
	m += ABSOLUTE_MONTH_OFFSET;
	
	/* Boost the month offset (always) */
	m += MONTH_UP_BOOST;
	
	/* Count all complete 231-year patterns */
	year_count = (m / MONTHS_PER_YEAR_PATTERN) * YEAR_PATTERN_LENGTH;
	m = m % MONTHS_PER_YEAR_PATTERN;

	/* Special case:  if this is the very last month in a 231-year
	 * pattern, then increase year_count by 230, set month remainder to
	 * zero, and set a flag to force the month offset within the year to
	 * 12 at the end; this accounts for the last month of the 11-year
	 * span being long at the end of 231-year patterns */
	if (m == MONTHS_PER_YEAR_PATTERN - 1) {
		year_count += (YEAR_PATTERN_LENGTH - 1);
		m = 0;
		force13 = true;
	}
	
	/* Count all complete 11-year spans */
	year_count += (m / MONTHS_PER_YEAR_SPAN) * YEAR_SPAN_LENGTH;
	m = m % MONTHS_PER_YEAR_SPAN;
	
	/* While there are still months remaining, go through the year span
	 * pattern until we've reached the year the month offset is
	 * within */
	pc = m_year_span;
	while (m > 0) {
		/* Determine whether the current year in the pattern is long or
		 * short */
		mlong = charToBool(*pc);

		/* If month offset is within current year, then break out of the
		 * loop */
		if (( mlong && (m < MONTHS_PER_LONG_YEAR )) ||
			(!mlong && (m < MONTHS_PER_SHORT_YEAR))) {
			break;
		}
		
		/* Otherwise, increase year count, decrease month offset, and
		 * move to next year in the pattern */
		year_count++;
		
		if (mlong) {
			m -= MONTHS_PER_LONG_YEAR;
		} else {
			m -= MONTHS_PER_SHORT_YEAR;
		}
		
		pc++;
	}
	
	/* If the special force13 flag was set earlier, force the remainder
	 * of months to 12 here */
	if (force13) {
		m = MONTHS_PER_LONG_YEAR - 1;
	}
	
	/* Sink the year count (always) */
	year_count -= YEAR_UP_SINK;

	/* Return the remainder month offset, if it was requested */
	if (pOffset != NULL) {
		*pOffset = m;
	}
	
	/* Return the year */
	return year_count;
}

/*
 * nelsc_cycle_yeatToMonth function.
 */
int32_t nelsc_cycle_yearToMonth(int32_t y) {
	
	int32_t month_count = 0;
	int32_t i = 0;
	bool mlong = false;
	
	/* Check parameter */
	if ((y < NELSC_CYCLE_YEARMIN) || (y > NELSC_CYCLE_YEARMAX)) {
		abort();
	}
	
	/* Boost the year (always) */
	y += YEAR_DOWN_BOOST;
	
	/* Count all complete 231-year patterns */
	month_count = (y / YEAR_PATTERN_LENGTH) * MONTHS_PER_YEAR_PATTERN;
	y = y % YEAR_PATTERN_LENGTH;
	
	/* Count all complete 11-year spans */
	month_count += (y / YEAR_SPAN_LENGTH) * MONTHS_PER_YEAR_SPAN;
	y = y % YEAR_SPAN_LENGTH;
	
	/* While there are still years remaining, go through the year span
	 * pattern until we've reached our year; since we never use the last
	 * digit in the pattern here, we don't have to worry about the extra
	 * long month at the end of the pattern */
	for(i = 0; i < y; i++) {
		/* Determine whether the current year in the pattern is long or
		 * short */
		mlong = charToBool(m_year_span[i]);

		/* Increase the month count appropriately */
		if (mlong) {
			month_count += MONTHS_PER_LONG_YEAR;
		} else {
			month_count += MONTHS_PER_SHORT_YEAR;
		}
	}
	
	/* Sink the month count (always) */
	month_count -= MONTH_DOWN_SINK;

	/* Apply the absolute month offset */
	month_count -= ABSOLUTE_MONTH_OFFSET;
	
	/* Return the month */
	return month_count;
}

/*
 * nelsc_cycle_isLongMonth function.
 */
bool nelsc_cycle_isLongMonth(int32_t m) {
	
	int32_t day_begin = 0;
	int32_t day_next = 0;
	bool longmonth = false;
	
	/* Check parameter */
	if ((m < NELSC_CYCLE_MONMIN) || (m > NELSC_CYCLE_MONMAX)) {
		abort();
	}
	
	/* Determine the first day of the month */
	day_begin = nelsc_cycle_monthToDay(m);
	
	/* Determine the first day of the next month -- in the special case
	 * of the very last month of NELSC, use one greater than the NELSC
	 * greatest day offset as the start of the hypothetical next
	 * month */
	if (m < NELSC_CYCLE_MONMAX) {
		day_next = nelsc_cycle_monthToDay(m + 1);
	} else {
		day_next = NELSC_CYCLE_DAYMAX + 1;
	}
	
	/* Count the days in the current month and use that to determine
	 * whether this is a short month */
	if (day_next - day_begin > DAYS_PER_SHORT_MONTH) {
		longmonth = true;
	} else {
		longmonth = false;
	}
	
	/* Return result */
	return longmonth;
}

/*
 * nelsc_cycle_isLongYear function.
 */
bool nelsc_cycle_isLongYear(int32_t y) {
	
	int32_t month_begin = 0;
	int32_t month_next = 0;
	bool longyear = false;
	
	/* Check parameter */
	if ((y < NELSC_CYCLE_YEARMIN) || (y > NELSC_CYCLE_YEARMAX)) {
		abort();
	}
	
	/* Determine the first month of the year */
	month_begin = nelsc_cycle_yearToMonth(y);
	
	/* Determine the first month of the next year -- in the special case
	 * of the very last year or NELSC, use one greater than the NELSC
	 * greatest month offset as the start of the hypothetical next
	 * year */
	if (y < NELSC_CYCLE_YEARMAX) {
		month_next = nelsc_cycle_yearToMonth(y + 1);
	} else {
		month_next = NELSC_CYCLE_MONMAX + 1;
	}
	
	/* Count the months in the current year and use that to determine
	 * whether this is a short year */
	if (month_next - month_begin > MONTHS_PER_SHORT_YEAR) {
		longyear = true;
	} else {
		longyear = false;
	}
	
	/* Return result */
	return longyear;
}
