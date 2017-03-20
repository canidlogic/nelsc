/*
 * grcal.c
 * 
 * Implementation of grcal.h
 * 
 * See the header for further information.
 */

#include "grcal.h"
#include <stdlib.h>

/*
 * The number of months in a year.
 */
#define MONTH_COUNT 12

/*
 * The number of months that March-based years are offset from standard
 * Gregorian years.
 */
#define MONTH_OFFSET 2

/*
 * The number of days in a long, 31-day month.
 */
#define LONG_MONTH_LENGTH 31

/*
 * The number of days in a short, 30-day month.
 */
#define SHORT_MONTH_LENGTH 30

/*
 * The number of days in a month with a leap day in leap year.
 */
#define LEAP_MONTH_LENGTH 29

/*
 * The number of days in a month with a leap day in a non-leap year.
 */
#define NONLEAP_MONTH_LENGTH 28

/*
 * The number of days in an aligned quad century (400 years).
 */
#define QC_DAYS 146097

/*
 * The number of days in an aligned century.
 */
#define C_DAYS 36524

/*
 * The number of days in an aligned quad year (4 years).
 */
#define Q_DAYS 1461

/*
 * The number of days in a year, not including the leap day at the end
 * of a quad year.
 */
#define Y_DAYS 365

/*
 * The number of days in a leap year.
 */
#define Y_LEAP_DAYS 366

/*
 * The number of centuries in a quad century.
 */
#define QC_C_COUNT 4

/*
 * The number of quad years in a century.
 */
#define C_Q_COUNT 25

/*
 * The number of years in a quad year.
 */
#define Q_Y_COUNT 4

/*
 * The number of years in a quad century.
 */
#define QC_YEARS 400

/*
 * The number of years in a century.
 */
#define C_YEARS 100

/*
 * The number of years in a quad year.
 */
#define Q_YEARS 4

/*
 * The year in which day offset zero happened.
 */
#define BASE_YEAR 1200

/*
 * The last year supported in the Gregorian calendar.
 */
#define MAX_YEAR 9999

/*
 * The number of characters in a standard-format YYYY-MM-DD string,
 * assuming that the MM and DD field are zero-padded to two characters.
 */
#define STANDARD_DATE_LENGTH 10

/*
 * The number of digits in a Gregorian formatted year field.
 */
#define YEAR_FIELD_LENGTH 4

/*
 * The maximum number of digits in a Gregorian formatted day or month
 * field.
 */
#define DAYMONTH_FIELD_MAXLENGTH 2

/*
 * The character to use as a field separator in Gregorian dates.
 */
#define DATE_SEPARATOR '-'

/*
 * The pattern of March-based month lengths, expressed as a string.
 * 
 * This string has twelve characters, corresponding to the twelve
 * months.  However, the first character corresponds to the *third*
 * month, which causes the variable-length second month to be the last
 * month in the pattern.
 * 
 * Pattern digits that are "+" indicate 31-day months, pattern digits
 * that are "-" indicate 30-day months, and pattern digits that are "*"
 * indicate variable-length months.
 */
static const char *m_pattern = "+-+-++-+-++*";

/*
 * Function prototypes
 */
static bool isLeapYear(int32_t y);
static int32_t monthLength(int32_t i);
static int32_t parseDecimal(char c);
static int32_t parseYear(const char *str);
static int32_t parseDayMonth(const char *str, const char **ppTrail);

/*
 * Determine whether the given (January-based) year is a leap year
 * according to Gregorian calendar rules.
 * 
 * Parameters:
 * 
 *   y - the year to check
 * 
 * Return:
 * 
 *   true if leap year, false if not
 * 
 * Faults:
 * 
 *   - If y is less than one
 */
static bool isLeapYear(int32_t y) {
	bool leapyear = false;
	
	/* Check parameter */
	if (y < 1) {
		abort();
	}
	
	/* Years divisible by 400 are leap years */
	if (y % 400 == 0) {
		leapyear = true;
	}
	
	/* Years divisible by 4 but not by 100 are leap years */
	if ((y % 4 == 0) && (y % 100 != 0)) {
		leapyear = true;
	}
	
	/* Return result */
	return leapyear;
}

/*
 * Return the length of the month corresponding to the provided
 * March-based month index.
 * 
 * The value i has zero for the third month of the Gregorian year,
 * matching the indexing system used for m_pattern.
 * 
 * The return value is either the number of days the month always has,
 * or zero if the month has variable length.
 * 
 * Parameters:
 * 
 *   i - the zero-indexed, March-based month offset
 * 
 * Return:
 * 
 *   the number of days in this month, or zero if the month is variable
 *   length
 * 
 * Faults:
 * 
 *   - If i is out of range
 * 
 *   - If m_pattern includes a character besides '+', '-', or '*'
 * 
 * Undefined behavior:
 * 
 *   - If m_pattern does not have exactly twelve characters before the
 *     terminating null
 */
static int32_t monthLength(int32_t i) {
	
	char c = 0;
	int32_t result = 0;
	
	/* Check parameter */
	if ((i < 0) || (i >= MONTH_COUNT)) {
		abort();
	}
	
	/* Get the requested pattern character */
	c = m_pattern[i];
	
	/* Interpret the pattern character */
	if (c == '+') {
		result = LONG_MONTH_LENGTH;
	
	} else if (c == '-') {
		result = SHORT_MONTH_LENGTH;
		
	} else if (c == '*') {
		result = 0;
		
	} else {
		abort();
	}
	
	/* Return result */
	return result;
}

/*
 * Parse the given ASCII character as an ASCII decimal character.
 * 
 * Parameters:
 * 
 *   c - the character to parse
 * 
 * Return:
 * 
 *   the character's decimal value (0-9), or -1 if the character is not
 *   an ASCII decimal character
 */
static int32_t parseDecimal(char c) {
	int32_t result = 0;
	
	if ((c >= '0') && (c <= '9')) {
		result = (int32_t) (c - '0');
	} else {
		result = -1;
	}
	
	return result;
}

/*
 * Parse the characters at the provided ASCII string as a Gregorian
 * year.
 * 
 * This function will immediately fail and stop processing if it
 * encounters a terminating null character, so it may be used with a
 * null-terminated string without worrying about reading past the end.
 * 
 * There must be four ASCII decimal characters at the indicated string.
 * This function does not skip over leading whitespace, so it must point
 * right at the beginning of the year field.  The function fails if
 * there are not four ASCII decimal characters.
 * 
 * Note that the function ignores anything beyond the four characters,
 * so even if there is another decimal character immediately following,
 * it will be ignored.
 * 
 * This function will not check that the year is valid.
 * 
 * Parameters:
 * 
 *   str - pointer to the ASCII characters to parse
 * 
 * Return:
 * 
 *   the parsed (but not validated) year, or -1 if parsing failed or str
 *   is NULL
 * 
 * Undefined behavior:
 * 
 *   - If the provided string is not null-terminated
 */
static int32_t parseYear(const char *str) {
	
	bool result = true;
	int32_t x = 0;
	int32_t y = 0;
	int32_t d = 0;
	
	/* Fail if str is NULL */
	if (str == NULL) {
		result = false;
	}
	
	/* Fail if any of the first four characters are a terminating
	 * null */
	if (result) {
		for(x = 0; x < YEAR_FIELD_LENGTH; x++) {
			if (str[x] == 0) {
				result = false;
				break;
			}
		}
	}
	
	/* Build the value up digit by digit */
	if (result) {
		for(x = 0; x < YEAR_FIELD_LENGTH; x++) {
			/* Parse current digit */
			d = parseDecimal(str[x]);
			
			/* Fail if not a valid decimal digit */
			if (d == -1) {
				result = false;
				break;
			}
			
			/* Add digit into computed result */
			y = (y * 10) + d;
		}
	}
	
	/* If failure, set parsed year to -1 */
	if (!result) {
		y = -1;
	}
	
	/* Return result */
	return y;
}

/*
 * Parse the characters at the provided ASCII string as a Gregorian
 * month or day field.
 * 
 * First, the function will count how many ASCII decimal digits are at
 * the string pointed to by str.  The counting function will stop if it
 * encounters a terminating null, so this is safe to use with
 * null-terminated strings without worrying about reading past the end
 * of the string.
 * 
 * The count of ASCII decimal digits must be one or two.  The function
 * fails if it is zero or greater than two.  This function does not skip
 * over leading whitespace, so it will fail if str points at whitespace.
 * 
 * If the function succeeds, the integer value of the field is returned
 * and *ppTrail is set to point at the first character immediately after
 * the field that was just read, if ppTrail is not NULL.  If the
 * function fails, -1 is returned.
 * 
 * This function will not check that the month or day is valid.
 * 
 * Parameters:
 * 
 *   str - pointer to the ASCII characters to parse
 * 
 *   ppTrail - pointer to the pointer to set to the character after the
 *   field upon successful completion, or NULL
 * 
 * Return:
 * 
 *   the parsed (but not validated) month or day, or -1 if parsing
 *   failed or str is NULL
 * 
 * Undefined behavior:
 * 
 *   - If the provided string is not null-terminated
 */
static int32_t parseDayMonth(const char *str, const char **ppTrail) {
	
	bool result = true;
	int32_t digit_count = 0;
	const char *pc = NULL;
	int32_t x = 0;
	int32_t val = 0;
	int32_t d = 0;
	
	/* Fail if str is NULL */
	if (str == NULL) {
		result = false;
	}
	
	/* Count the number of decimal digits, failing immediately at a
	 * non-decimal digit (to prevent overrunning the terminating null),
	 * and failing if going beyond DAYMONTH_FIELD_MAXLENGTH */
	if (result) {
		pc = str;
		while(parseDecimal(*pc) != -1) {
			digit_count++;
			if (digit_count > DAYMONTH_FIELD_MAXLENGTH) {
				result = false;
				break;
			}
			pc++;
		}
	}
	
	/* Fail if digit count is not one or two */
	if (result) {
		if ((digit_count < 1) ||
			(digit_count > DAYMONTH_FIELD_MAXLENGTH)) {
			result = false;
		}
	}
	
	/* Build the integer value digit by digit */
	if (result) {
		val = 0;
		for(x = 0; x < digit_count; x++) {
			/* Parse current digit */
			d = parseDecimal(str[x]);
			
			/* Fail if not a decimal */
			if (d == -1) {
				result = false;
				break;
			}
			
			/* Add digit into value */
			val = (val * 10) + d;
		}
	}
	
	/* If succeeded, write trailing value if requested; if failed, set
	 * value to -1 */
	if (result) {
		if (ppTrail != NULL) {
			*ppTrail = (str + digit_count);
		}
	} else {
		val = -1;
	}
	
	/* Return result */
	return val;
}

/*
 * grcal_offsetToDate function.
 */
void grcal_offsetToDate(
		int32_t offs,
		int32_t *pYear,
		int32_t *pMonth,
		int32_t *pDayOfMonth) {
	
	int32_t qc = 0;
	int32_t c = 0;
	int32_t q = 0;
	int32_t y = 0;
	int32_t d = 0;
	
	int32_t year = 0;
	int32_t month = 0;
	int32_t day = 0;
	
	int32_t ml = 0;
	
	/* Check parameter */
	if ((offs < GRCAL_DAY_MIN) || (offs > GRCAL_DAY_MAX)) {
		abort();
	}
	
	/* Compute number of quad centuries, centuries, quad years, and
	 * years, and adjust offs for remainder of days */
	qc   = offs / QC_DAYS;
	offs = offs % QC_DAYS;
	
	c    = offs / C_DAYS;
	offs = offs % C_DAYS;
	
	q    = offs / Q_DAYS;
	offs = offs % Q_DAYS;
	
	y    = offs / Y_DAYS;
	offs = offs % Y_DAYS;
	
	d    = offs;
	
	/* Special case:  c might be four, but only if the day offset refers
	 * to the leap day that is added at the end of each quad century; in
	 * this special case, adjust c, q, y, and d so that they refer to
	 * the leap day at the end of the quad century */
	if (c == QC_C_COUNT) {
		c = QC_C_COUNT - 1;
		q = C_Q_COUNT - 1;
		y = Q_Y_COUNT - 1;
		d = Y_LEAP_DAYS - 1;
	}
	
	/* Special case:  y might be four, but only if the day offset refers
	 * to the leap day that is added at the end of a quad year; in this
	 * special case, adjust y and d so that they refer to the leap day
	 * at the end of a quad year */
	if (y == Q_Y_COUNT) {
		y = Q_Y_COUNT - 1;
		d = Y_LEAP_DAYS - 1;
	}
	
	/* Compute the (March-based) year */
	year = 	(qc * QC_YEARS) + (c * C_YEARS) +
			(q  * Q_YEARS)  +  y            + BASE_YEAR;
	
	/* Compute the (March-based) month offset */
	month = 0;
	while (d > 0) {
		/* Get the length in days of the current month offset */
		ml = monthLength(month);
		
		/* If this is a variable-length month or this month contains the
		 * remaining day offset, break */
		if ((ml == 0) || (d < ml)) {
			break;
		}
		
		/* Otherwise, increase the month count and decrease the
		 * remaining days */
		month++;
		d -= ml;
	}
	
	/* Compute the (final, one-based) day of the month */
	day = d + 1;
	
	/* Apply the month offset to the month to get the (zero-based, mod
	 * 12) standard month offset */
	month += MONTH_OFFSET;
	
	/* If adjusted month went past the end of the year, subtract twelve
	 * from the month and add one to the year to get the (zero-based)
	 * standard month offset */
	if (month >= MONTH_COUNT) {
		month -= MONTH_COUNT;
		year++;
	}
	
	/* Add one to the month offset to get the standard, one-based month
	 * number */
	month++;
	
	/* Return any computed results that were requested */
	if (pYear != NULL) {
		*pYear = year;
	}
	
	if (pMonth != NULL) {
		*pMonth = month;
	}
	
	if (pDayOfMonth != NULL) {
		*pDayOfMonth = day;
	}
}

/*
 * grcal_dateToOffset function.
 */
bool grcal_dateToOffset(
		int32_t *pOffs,
		int32_t year,
		int32_t month,
		int32_t dayofmonth) {
	
	bool result = true;
	int32_t month_len = 0;
	
	int32_t qc = 0;
	int32_t c = 0;
	int32_t q = 0;
	
	int32_t offs = 0;
	int32_t x = 0;
	
	/* Fail if the year is BASE_YEAR or less, or if month or dayofmonth
	 * are less than one */
	if ((year <= BASE_YEAR) || (month < 1) || (dayofmonth < 1)) {
		result = false;
	}
	
	/* Fail if the year is greater than MAX_YEAR or if month is greater
	 * than MONTH_COUNT -- the range of everything but the upper bound
	 * of dayofmonth has now been checked */
	if (result) {
		if ((year > MAX_YEAR) || (month > MONTH_COUNT)) {
			result = false;
		}
	}
	
	/* Convert to zero-based day offset within month */
	if (result) {
		dayofmonth--;
	}
	
	/* Convert to March-based year offsets by making month zero-based,
	 * then subtracting two months, and decrementing the year and adding
	 * twelve to the month if it goes below zero */
	if (result) {
		month = month - 1 - MONTH_OFFSET;
		if (month < 0) {
			year--;
			month += MONTH_COUNT;
		}
	}
	
	/* Check the day offset against the length of the month, taking leap
	 * years into account */
	if (result) {
		/* Get the length of the month */
		month_len = monthLength(month);
		
		/* If month is variable length, use leap year to determine month
		 * length -- increment year because March-based years have their
		 * last (leap) month in the next year according to the
		 * January-based years used for leap year calculations */
		if (month_len == 0) {
			if (isLeapYear(year + 1)) {
				month_len = LEAP_MONTH_LENGTH;
			} else {
				month_len = NONLEAP_MONTH_LENGTH;
			}
		}
		
		/* Verify upper bound of day offset within month */
		if (dayofmonth >= month_len) {
			result = false;
		}
	}
	
	/* We've now converted to March-based offsets and checked the range
	 * of the input parameters -- begin by decreasing the year by
	 * BASE_YEAR to make it relative to the base year */
	if (result) {
		year -= BASE_YEAR;
	}
	
	/* Get the number of quad centuries, centuries, and quad years in
	 * the year offset, leaving year as the remainder years */
	if (result) {
		qc   = year / QC_YEARS;
		year = year % QC_YEARS;
		
		c    = year / C_YEARS;
		year = year % C_YEARS;
		
		q    = year / Q_YEARS;
		year = year % Q_YEARS;
	}
	
	/* Calculate the number of days to the start of the year */
	if (result) {
		offs = (qc * QC_DAYS) + (c    * C_DAYS) + 
			   (q  * Q_DAYS ) + (year * Y_DAYS);
	}
	
	/* Get to the start of the month by adding month lengths together;
	 * the variable length month won't figure in here because that is
	 * the last month in March-based years and we are only getting to
	 * the beginning of months, not passing them */
	if (result) {
		for(x = 0; x < month; x++) {
			offs += monthLength(x);
		}
	}
	
	/* Finally, add the month-in-day offset in */
	if (result) {
		offs += dayofmonth;
	}
	
	/* Fail if offset is outside the allowable range */
	if (result) {
		if ((offs < GRCAL_DAY_MIN) || (offs > GRCAL_DAY_MAX)) {
			result = false;
		}
	}
	
	/* Write the computed offset, if it was requested */
	if (result) {
		if (pOffs != NULL) {
			*pOffs = offs;
		}
	}
	
	/* Return status */
	return result;
}

/*
 * grcal_printDate function.
 */
void grcal_printDate(
		FILE *pFile,
		int32_t y,
		int32_t m,
		int32_t d) {
	
	/* Check parameters */
	if ((pFile == NULL) || (!grcal_dateToOffset(NULL, y, m, d))) {
		abort();
	}
	
	/* Print the date */
	if (fprintf(
			pFile, "%04d-%02d-%02d",
			(int) y, (int) m, (int) d) != STANDARD_DATE_LENGTH) {
		abort();
	}
}

/*
 * grcal_scanDate function.
 */
int32_t grcal_scanDate(const char *str, const char **ppTrail) {
	
	bool result = true;
	
	int32_t year = 0;
	int32_t month = 0;
	int32_t day = 0;
	
	int32_t offs = 0;
	
	const char *pc = NULL;
	
	/* Fail if str is NULL */
	if (str == NULL) {
		result = false;
	}
	
	/* Attempt to read each field, verifying the separators between them
	 * as well */
	pc = str;

	if (result) {
		year = parseYear(pc);
		if (year != -1) {
			pc += YEAR_FIELD_LENGTH;
		} else {
			result = false;
		}
	}

	if (result) {
		if (*pc == DATE_SEPARATOR) {
			pc++;
		} else {
			result = false;
		}
	}

	if (result) {
		month = parseDayMonth(pc, &pc);
		if (month == -1) {
			result = false;
		}
	}

	if (result) {
		if (*pc == DATE_SEPARATOR) {
			pc++;
		} else {
			result = false;
		}
	}

	if (result) {
		day = parseDayMonth(pc, &pc);
		if (day == -1) {
			result = false;
		}
	}

	/* Convert to a Gregorian day offset */
	if (result) {
		result = grcal_dateToOffset(&offs, year, month, day);
	}

	/* If succeeded, write trailing pointer if requested; if failed, set
	 * result to -1 */
	if (result) {
		if (ppTrail != NULL) {
			*ppTrail = pc;
		}
	} else {
		offs = -1;
	}

	/* Return result */
	return offs;
}
