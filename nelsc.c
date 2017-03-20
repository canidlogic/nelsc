/*
 * nelsc.c
 * 
 * Main program module for the NELSC application.  This contains the
 * "main" method.
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base24.h"
#include "grcal.h"
#include "nelsc_cycle.h"
#include "nelsc_format.h"

/*
 * The day offset from the first day of the month that full moon week
 * begins in a short month.
 */
#define FULLMOON_SHORT_BEGIN 14

/*
 * The day offset from the first day of the month that full moon week
 * ends in a short month.
 */
#define FULLMOON_SHORT_END 20

/*
 * The day offset from the first day of the month that full moon week
 * begins in a long month.
 */
#define FULLMOON_LONG_BEGIN 21

/*
 * The day offset from the first day of the month that full moon week
 * ends in a long month.
 */
#define FULLMOON_LONG_END 27

/*
 * The Gregorian month that the March equinox always happens in.
 */
#define EQUINOX_MONTH 3

/*
 * The Gregorian day of month that the March equinox usually happens on
 * or within a day of.
 */
#define EQUINOX_DAY 20

/* Local function prototypes */
static const char *getCustom(int argc, char *argv[], int i);
static int getCustomCount(int argc);
static bool stringToLong(const char *str, long *pLong);
static bool pairToLong(const char *str, long *pLong);
static bool dateToOffset(const char *str, int32_t *pOffset);

static void printDayInformation(int32_t day);
static void fullMoons(int32_t mfirst, int32_t mlast);

static int sub_help(void);
static int sub_to24pair(int argc, char *argv[]);
static int sub_from24pair(int argc, char *argv[]);
static int sub_to24digit(int argc, char *argv[]);
static int sub_from24digit(int argc, char *argv[]);
static int sub_day(int argc, char *argv[]);
static int sub_month(int argc, char *argv[]);
static int sub_date(int argc, char *argv[]);
static int sub_fullmoon(int argc, char *argv[]);
static int sub_newyear(int argc, char *argv[]);

/*
 * Get the custom program argument with index i.
 * 
 * The first custom program argument is index zero.  If the requested
 * custom program argument index exceeds the number of custom program
 * arguments, then an empty string is returned.
 * 
 * The first custom program argument is the second element of argv --
 * the first element is assumed to be the module name.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 *   i - the custom program argument to return, with zero corresponding
 *   to the first custom argument
 * 
 * Return:
 * 
 *   the null-terminated custom program argument, or an empty string if
 *   the requested argument does not exist
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 *   - If i is negative
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static const char *getCustom(int argc, char *argv[], int i) {
	
	int x = 0;
	char *result = NULL;
	
	/* Check arguments */
	if ((argc < 0) || (argv == NULL) || (i < 0)) {
		abort();
	}
	
	for(x = 0; x < argc; x++) {
		if (argv[x] == NULL) {
			abort();
		}
	}
	
	/* Determine whether i is in range */
	if (i < getCustomCount(argc)) {
		/* i is in range -- read the custom argument */
		result = argv[i + 1];
	} else {
		/* i is out of range -- return empty string */
		result = "";
	}
	
	/* Return result */
	return result;
}

/*
 * Get the total number of custom program arguments, given the argc
 * value that was passed to main.
 * 
 * The argc value is one greater than the custom program argument count,
 * because the first argument is the module name.
 * 
 * If argc is zero, zero is returned.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments
 * 
 * Return:
 * 
 *   the number of custom program arguments
 * 
 * Faults:
 * 
 *   - If argc is negative
 */
static int getCustomCount(int argc) {
	int result = 0;
	
	if (argc > 0) {
		result = argc - 1;
	} else if (argc == 0) {
		result = 0;
	} else {
		abort();
	}
	
	return result;
}

/*
 * Convert the given null-terminated string representing a signed
 * integer in ASCII decimal into a long value.
 * 
 * If successful, the converted long value is stored to *pLong and true
 * is returned.  If the string could not be parsed, *pLong is unmodified
 * and false is returned.
 * 
 * Parameters:
 * 
 *   str - pointer to the string to convert
 * 
 *   pLong - pointer to the variable to receive the converted long value
 *   on success
 * 
 * Return:
 * 
 *   true if successful, false if parsing error
 * 
 * Faults:
 * 
 *   - If str is NULL
 * 
 *   - If pLong is NULL
 * 
 * Undefined behavior:
 * 
 *   - If str is not null-terminated
 */
static bool stringToLong(const char *str, long *pLong) {
	
	bool result = true;
	long i = 0;
	char *endptr = NULL;
	
	/* Check parameters */
	if ((str == NULL) || (pLong == NULL)) {
		abort();
	}
	
	/* Attempt to convert the string as a base-10 decimal */
	errno = 0;
	i = strtol(str, &endptr, 10);
	
	/* Check for parsing error */
	if (errno != 0) {
		result = false;
	}
	
	/* If endptr equals original string, it means parameter was empty,
	 * so fail */
	if (result) {
		if (endptr == str) {
			result = false;
		}
	}
	
	/* Make sure the unconverted part of the argument is either empty or
	 * consists only of whitespace */
	if (result) {
		while(*endptr != 0) {
			if (!isspace(*endptr)) {
				result = false;
				break;
			}
			endptr++;
		}
	}
	
	/* If successful, write the result */
	if (result) {
		*pLong = i;
	}
	
	/* Return the status */
	return result;
}

/*
 * Convert the given null-terminated string representing a signed
 * base-24 pair in ASCII into a long value.
 * 
 * If successful, the converted long value is stored to *pLong and true
 * is returned.  If the string could not be parsed, *pLong is unmodified
 * and false is returned.
 * 
 * Parameters:
 * 
 *   str - pointer to the string to convert
 * 
 *   pLong - pointer to the variable to receive the converted long value
 *   on success
 * 
 * Return:
 * 
 *   true if successful, false if parsing error
 * 
 * Faults:
 * 
 *   - If str is NULL
 * 
 *   - If pLong is NULL
 * 
 * Undefined behavior:
 * 
 *   - If str is not null-terminated
 */
static bool pairToLong(const char *str, long *pLong) {
	
	bool result = true;
	int32_t i = 0;
	const char *pc = NULL;
	
	/* Check parameters */
	if ((str == NULL) || (pLong == NULL)) {
		abort();
	}
	
	/* Find the first non-whitespace character, failing if there is no
	 * such thing */
	pc = str;
	while (*pc != 0) {
		if (!isspace(*pc)) {
			break;
		}
		pc++;
	}
	if (*pc == 0) {
		result = false;
	}
	
	/* Attempt to parse a base-24 pair */
	if (result) {
		result = base24_pairToInt(pc, &i);
	}
	
	/* Verify that anything after a valid base-24 pair is whitespace */
	if (result) {
		pc = pc + 2;	/* Skip the base-24 pair that was just parsed */
		while (*pc != 0) {
			if (!isspace(*pc)) {
				result = false;
				break;
			}
			pc++;
		}
	}
	
	/* If successful, write the result */
	if (result) {
		*pLong = (long) i;
	}
	
	/* Return the status */
	return result;
}

/*
 * Convert the given null-terminated string representing a calendar date
 * in ASCII into a NELSC absolute day offset.
 * 
 * This supports both NELSC and Gregorian dates (YYYY-MM-DD format).
 * 
 * If successful, the converted offset is stored to *pOffset and true is
 * returned.  If the string could not be parsed, *pOffset is unmodified
 * and false is returned.
 * 
 * Parameters:
 * 
 *   str - pointer to the string to convert
 * 
 *   pOffset - pointer to the variable to receive the converted day
 *   offset on success
 * 
 * Return:
 * 
 *   true if successful, false if parsing error
 * 
 * Faults:
 * 
 *   - If str is NULL
 * 
 *   - If pOffset is NULL
 * 
 * Undefined behavior:
 * 
 *   - If str is not null-terminated
 */
static bool dateToOffset(const char *str, int32_t *pOffset) {
	
	bool result = true;
	bool gregorian = false;
	int32_t d = 0;
	const char *pc = NULL;
	
	/* Check parameters */
	if ((str == NULL) || (pOffset == NULL)) {
		abort();
	}
	
	/* Find the first non-whitespace character, failing if there is no
	 * such thing */
	pc = str;
	while (*pc != 0) {
		if (!isspace(*pc)) {
			break;
		}
		pc++;
	}
	if (*pc == 0) {
		result = false;
	}
	
	/* Attempt to parse a calendar date, trying NELSC first and then
	 * falling back to Gregorian */
	if (result) {
		result = nelsc_format_scanDate(pc, &d);
		if (!result) {
			gregorian = true;
			d = grcal_scanDate(pc, &pc);
			if (d != -1) {
				result = true;
			} else {
				result = false;
			}
		}
	}
	
	/* Skip over the date if NELSC; if Gregorian, this was already done
	 * during parsing stage */
	if (result) {
		if (!gregorian) {
			pc = pc + NELSC_FORMAT_DATE_LENGTH;
		}
	}
	
	/* If Gregorian, convert offset to NELSC absolute day, and check
	 * range is in NELSC */
	if (result) {
		if (gregorian) {
			d -= NELSC_CYCLE_GROFFS;
			if ((d < NELSC_CYCLE_DAYMIN) || (d > NELSC_CYCLE_DAYMAX)) {
				result = false;
			}
		}
	}
	
	/* Verify that anything after a valid date is whitespace */
	if (result) {
		while (*pc != 0) {
			if (!isspace(*pc)) {
				result = false;
				break;
			}
			pc++;
		}
	}
	
	/* If successful, write the result */
	if (result) {
		*pOffset = d;
	}
	
	/* Return the status */
	return result;
}

/*
 * Print information about the day indicated by the provided NELSC
 * absolute day offset.
 * 
 * Parameters:
 * 
 *   day - the NELSC absolute day offset
 * 
 * Faults:
 * 
 *   - If day is out of range
 */
static void printDayInformation(int32_t day) {
	
	int32_t year = 0;
	int32_t month_of_year = 0;
	int32_t absolute_month = 0;
	int32_t day_of_month = 0;
	
	int32_t gr_year = 0;
	int32_t gr_month = 0;
	int32_t gr_day = 0;
	
	/* Check parameter */
	if ((day < NELSC_CYCLE_DAYMIN) || (day > NELSC_CYCLE_DAYMAX)) {
		abort();
	}
	
	/* Print information */
	printf("Day offset:      %ld\n", (long) day);
	
	absolute_month = nelsc_cycle_dayToMonth(day, &day_of_month);
	year = nelsc_cycle_monthToYear(absolute_month, &month_of_year);
	
	grcal_offsetToDate(
		(day + NELSC_CYCLE_GROFFS),
		&gr_year,
		&gr_month,
		&gr_day);
	
	printf("Absolute month:  %ld\n", (long) absolute_month);
	printf("NELSC date:      ");
	nelsc_format_printDate(stdout,
		year, month_of_year, day_of_month);
	printf("\n");
	printf("Month length:    ");
	if (nelsc_cycle_isLongMonth(absolute_month)) {
		printf("long\n");
	} else {
		printf("short\n");
	}
	printf("Year length:     ");
	if (nelsc_cycle_isLongYear(year)) {
		printf("long\n");
	} else {
		printf("short\n");
	}
	
	printf("Gregorian date:  ");
	grcal_printDate(stdout, gr_year, gr_month, gr_day);
	printf("\n");
}

/*
 * Print the NELSC full moon weeks from NELSC absolute month mfirst up
 * to and including NELSC absolute month mlast.
 * 
 * mfirst and mlast must both be in range NELSC_CYCLE_MONMIN up to
 * NELSC_CYCLE_MONMAX (inclusive of boundaries).  mlast must be greater
 * than or equal to mfirst.
 * 
 * Parameters:
 * 
 *   mfirst - the first month in the range
 * 
 *   mlast - the last month in the range
 * 
 * Faults:
 * 
 *   - If mfirst or mlast is out of range
 * 
 *   - If mfirst is greater than mlast
 */
static void fullMoons(int32_t mfirst, int32_t mlast) {
	
	int32_t m = 0;
	int32_t lyear = -1;
	
	int32_t m_begin = 0;
	int32_t fmw_begin = 0;
	int32_t fmw_end = 0;
	
	int32_t b_year = 0;
	int32_t b_month = 0;
	int32_t b_day = 0;
	
	int32_t e_year = 0;
	int32_t e_month = 0;
	int32_t e_day = 0;

	/* Check parameters */
	if ((mfirst < NELSC_CYCLE_MONMIN) ||
		(mfirst > NELSC_CYCLE_MONMAX) ||
		(mlast < NELSC_CYCLE_MONMIN) ||
		(mlast > NELSC_CYCLE_MONMAX) ||
		(mfirst > mlast)) {
		abort();
	}

	/* Print the full moon week for each month in range */
	for(m = mfirst; m <= mlast; m++) {
		/* Start with the first day of the month */
		m_begin = nelsc_cycle_monthToDay(m);

		/* Compute full moon week boundaries depending on whether this
		 * is a long or short month */
		if (nelsc_cycle_isLongMonth(m)) {
			fmw_begin = m_begin + FULLMOON_LONG_BEGIN;
			fmw_end   = m_begin + FULLMOON_LONG_END;
		} else {
			fmw_begin = m_begin + FULLMOON_SHORT_BEGIN;
			fmw_end   = m_begin + FULLMOON_SHORT_END;
		}

		/* Convert NELSC absolute day offsets to Gregorian offsets */
		fmw_begin += NELSC_CYCLE_GROFFS;
		fmw_end   += NELSC_CYCLE_GROFFS;

		/* Convert begin and end days to Gregorian dates */
		grcal_offsetToDate(fmw_begin, &b_year, &b_month, &b_day);
		grcal_offsetToDate(fmw_end,   &e_year, &e_month, &e_day);

		/* If lyear is not -1 (indicating first time through the loop),
		 * prefix a blank line if the year of the begin date is
		 * different from the last begin date's year */
		if ((lyear != -1) && (lyear != b_year)) {
			printf("\n");
		}
		
		/* Store the begin year of the current full moon in lyear */
		lyear = b_year;
		
		/* Write the dates */
		grcal_printDate(stdout, b_year, b_month, b_day);
		printf(" - ");
		grcal_printDate(stdout, e_year, e_month, e_day);
		printf("\n");
	}
}

/*
 * Subprogram to display a brief helpscreen.
 * 
 * Return:
 * 
 *   always EXIT_SUCCESS
 */
static int sub_help(void) {
	printf(

"nelsc command summary:\n"
"\n"
"  help - show this helpscreen.\n"
"\n"
"  to24pair [i] - convert signed decimal integer i into a base-24\n"
"  pair in signed style.\n"
"\n"
"  from24pair [p] - convert base-24 pair i in signed style into a\n"
"  signed decimal integer.  p must have exactly two base-24 digits.\n"
"\n"
"  to24digit [i] - convert integer i into an unsigned base-24 digit.\n"
"  i must be in range 0-23.\n"
"\n"
"  from24digit [d] - convert base-24 digit d into a decimal integer.\n"
"  d must contain only one base-24 digit.\n"
"\n"
"  day [d] - provide information about the day indicated by NELSC\n"
"  absolute day offset d.\n"
"\n"
"  month [m] - provide information about the first day of the month\n"
"  indicated by NELSC absolute month offset m.\n"
"\n"
"  date [d] - provide information about a particular calendar date.\n"
"  The parameter d must be a NELSC date in 3T:C4-7 format, or a\n"
"  Gregorian date in YYYY-MM-DD format.\n"
"\n"
"  fullmoon [m1] [m2] - return the Gregorian dates of the full moon\n"
"  weeks in NELSC from NELSC absolute month offset m1 up to m2.  The\n"
"  full moon does not always actually happen in the full moon week.\n"
"\n"
"  newyear - create a chart of all NELSC years and the Gregorian date\n"
"  of the first day of the year for each year, along with minimum and\n"
"  maximum Gregorian month and day for the first day of the year, and\n"
"  for each year the offset from the first month that March 20\n"
"  (an approximation of the equinox) happens.\n"
"\n"

	);

	return EXIT_SUCCESS;
}

/*
 * Subprogram to convert a signed decimal integer into a base-24 pair.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments or the argument is out of range,
 * an error message is displayed to the user and EXIT_FAILURE is
 * returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_to24pair(int argc, char *argv[]) {
	
	const char *arg_decimal = NULL;
	long argi = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the decimal
	 * argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr,
			"to24pair expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_decimal = getCustom(argc, argv, 1);
	}
	
	/* Convert the argument to a long integer */
	if (result != EXIT_FAILURE) {
		if (!stringToLong(arg_decimal, &argi)) {
			fprintf(stderr,
				"Could not parse argument as decimal integer!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Check the range of the argument */
	if (result != EXIT_FAILURE) {
		if ((argi < BASE24_PAIR_MIN) || (argi > BASE24_PAIR_MAX)) {
			fprintf(stderr,
				"Argument must be in range -96 to 479!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Perform the conversion */
	if (result != EXIT_FAILURE) {
		printf("Decimal value:  %ld\n", argi);
		printf("Base-24 pair:   ");
		base24_printPair(stdout, (int32_t) argi);
		printf("\n");
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to convert a base-24 pair in signed style to a signed
 * decimal integer.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_from24pair(int argc, char *argv[]) {
	
	const char *arg_pair = NULL;
	long val = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the base-24
	 * pair argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr,
			"from24pair expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_pair = getCustom(argc, argv, 1);
	}
	
	/* Perform the conversion */
	if (result != EXIT_FAILURE) {
		if (!pairToLong(arg_pair, &val)) {
			fprintf(stderr,
				"Could not parse as a base-24 pair!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Report results */
	if (result != EXIT_FAILURE) {
		printf("Base-24 pair:   ");
		base24_printPair(stdout, (int32_t) val);
		printf("\n");
		printf("Decimal value:  %ld\n", val);
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to convert a decimal integer into an unsigned base-24
 * digit.
 * 
 * The provided integer must be in the range 0 to 23 (inclusive).
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments or the argument is out of range,
 * an error message is displayed to the user and EXIT_FAILURE is
 * returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_to24digit(int argc, char *argv[]) {
	
	const char *arg_decimal = NULL;
	long argi = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the decimal
	 * argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr,
			"to24digit expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_decimal = getCustom(argc, argv, 1);
	}
	
	/* Convert the argument to a long integer */
	if (result != EXIT_FAILURE) {
		if (!stringToLong(arg_decimal, &argi)) {
			fprintf(stderr,
				"Could not parse argument as decimal integer!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Check range */
	if (result != EXIT_FAILURE) {
		if ((argi < 0) || (argi > BASE24_DIGIT_MAX)) {
			fprintf(stderr,
				"Argument must be in range 0 to 23!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Perform the conversion */
	if (result != EXIT_FAILURE) {
		printf("Decimal value:  %ld\n", argi);
		printf("Base-24 digit:  %c\n",
			base24_intToDigit((int32_t) argi));
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to convert an unsigned base-24 digit to a decimal integer.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_from24digit(int argc, char *argv[]) {
	
	const char *arg_digit = NULL;
	const char *pc = NULL;
	char digit = 0;
	int val = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the base-24
	 * digit argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr,
			"from24digit expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_digit = getCustom(argc, argv, 1);
	}
	
	/* Scan through arg_digit and make sure there is exactly one
	 * non-whitespace character; put this character into digit */
	if (result != EXIT_FAILURE) {
		pc = arg_digit;
		while(*pc != 0) {
			/* Check if current character is whitespace */
			if (!isspace(*pc)) {
				/* Not whitespace -- fail if we already have the digit
				 * character */
				if (digit != 0) {
					fprintf(stderr,
						"Provide no more than one base-24 digit!\n");
					result = EXIT_FAILURE;
					break;
				}
				
				/* Otherwise, record the non-whitespace character */
				digit = *pc;
			}
			pc++;
		}
		
		/* Fail if we didn't encounter any non-whitespace characters */
		if (result != EXIT_FAILURE) {
			if (digit == 0) {
				fprintf(stderr,
					"Provide a base-24 digit!\n");
				result = EXIT_FAILURE;
			}
		}
	}
	
	/* Convert to decimal value */
	if (result != EXIT_FAILURE) {
		val = (int) base24_digitToInt(digit);
		if (val == -1) {
			fprintf(stderr,
				"Could not parse as base-24 digit!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Report results */
	if (result != EXIT_FAILURE) {
		printf("Base-24 digit:  %c\n", digit);
		printf("Decimal value:  %d\n", val);
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to provide information a particular NELSC absolute day
 * offset.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_day(int argc, char *argv[]) {
	
	const char *arg_decimal = NULL;
	long day = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the decimal
	 * argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr,
			"day expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_decimal = getCustom(argc, argv, 1);
	}
	
	/* Convert the argument to a long integer */
	if (result != EXIT_FAILURE) {
		if (!stringToLong(arg_decimal, &day)) {
			fprintf(stderr,
				"Could not parse argument as decimal integer!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Check the range of the argument */
	if (result != EXIT_FAILURE) {
		if ((day < NELSC_CYCLE_DAYMIN) || (day > NELSC_CYCLE_DAYMAX)) {
			fprintf(stderr,
				"Argument must be in range %d to %d!\n",
				NELSC_CYCLE_DAYMIN,
				NELSC_CYCLE_DAYMAX);
			result = EXIT_FAILURE;
		}
	}

	/* Print information */
	if (result != EXIT_FAILURE) {
		printDayInformation((int32_t) day);
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to provide information a particular NELSC absolute month
 * offset.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_month(int argc, char *argv[]) {
	
	const char *arg_decimal = NULL;
	long month = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the decimal
	 * argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr,
			"month expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_decimal = getCustom(argc, argv, 1);
	}
	
	/* Convert the argument to a long integer */
	if (result != EXIT_FAILURE) {
		if (!stringToLong(arg_decimal, &month)) {
			fprintf(stderr,
				"Could not parse argument as decimal integer!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Check the range of the argument */
	if (result != EXIT_FAILURE) {
		if ((month < NELSC_CYCLE_MONMIN) ||
				(month > NELSC_CYCLE_MONMAX)) {
			fprintf(stderr,
				"Argument must be in range %d to %d!\n",
				NELSC_CYCLE_MONMIN,
				NELSC_CYCLE_MONMAX);
			result = EXIT_FAILURE;
		}
	}
	
	/* Print information */
	if (result != EXIT_FAILURE) {
		printDayInformation(nelsc_cycle_monthToDay((int32_t) month));
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to provide information a particular calendar date (either
 * NELSC or Gregorian).
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_date(int argc, char *argv[]) {
	
	const char *arg_date = NULL;
	int32_t offs = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the date
	 * argument */
	if (getCustomCount(argc) != 2) {
		fprintf(stderr, 
			"date expects exactly one additional argument!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_date = getCustom(argc, argv, 1);
	}
	
	/* Perform the conversion */
	if (result != EXIT_FAILURE) {
		if (!dateToOffset(arg_date, &offs)) {
			fprintf(stderr,
				"Could not parse as a valid calendar date!\n"
				"(Note: Gregorian dates must be in range 1828-04-07 to "
				"2404-04-11.)\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Print information */
	if (result != EXIT_FAILURE) {
		printDayInformation(offs);
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to provide the Gregorian dates of the NELSC full moon
 * weeks for a given range of NELSC months.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_fullmoon(int argc, char *argv[]) {
	
	const char *arg_decimal_1 = NULL;
	const char *arg_decimal_2 = NULL;
	long month_1 = 0;
	long month_2 = 0;
	int result = EXIT_SUCCESS;
	
	/* Verify the total number of custom parameters and get the decimal
	 * arguments */
	if (getCustomCount(argc) != 3) {
		fprintf(stderr,
			"fullmoon expects exactly two additional arguments!\n");
		result = EXIT_FAILURE;
	}
	
	if (result != EXIT_FAILURE) {
		arg_decimal_1 = getCustom(argc, argv, 1);
		arg_decimal_2 = getCustom(argc, argv, 2);
	}
	
	/* Convert the arguments to long integers */
	if (result != EXIT_FAILURE) {
		if (!stringToLong(arg_decimal_1, &month_1)) {
			fprintf(stderr,
				"Could not parse first argument as decimal integer!\n");
			result = EXIT_FAILURE;
		}
	}
	
	if (result != EXIT_FAILURE) {
		if (!stringToLong(arg_decimal_2, &month_2)) {
			fprintf(stderr,
			"Could not parse second argument as decimal integer!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Check the range of the arguments */
	if (result != EXIT_FAILURE) {
		if ((month_1 < NELSC_CYCLE_MONMIN) ||
				(month_1 > NELSC_CYCLE_MONMAX) ||
				(month_2 < NELSC_CYCLE_MONMIN) ||
				(month_2 > NELSC_CYCLE_MONMAX)) {
			fprintf(stderr,
				"Arguments must be in range %d to %d!\n",
				NELSC_CYCLE_MONMIN,
				NELSC_CYCLE_MONMAX);
			result = EXIT_FAILURE;
		}
	}
	
	/* Check that second argument is not less than first argument */
	if (result != EXIT_FAILURE) {
		if (month_2 < month_1) {
			fprintf(stderr,
				"Second argument must not be less than first!\n");
			result = EXIT_FAILURE;
		}
	}
	
	/* Call through to the computation procedure */
	if (result != EXIT_FAILURE) {
		fullMoons((int32_t) month_1, (int32_t) month_2);
	}
	
	/* Return result */
	return result;
}

/*
 * Subprogram to provide the Gregorian dates of the first day of each
 * NELSC year, along with minimum and maximum Gregorian month and day of
 * the first day of the NELSC year, and for each year the year drift
 * relative to March 20.
 * 
 * If the improper number of custom arguments is specified or the
 * subprogram can't parse the arguments, an error message is displayed
 * to the user and EXIT_FAILURE is returned.
 * 
 * The first custom parameter is *not* checked -- that was assumed to
 * have been interpreted by the main procedure to select this
 * subprogram.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
static int sub_newyear(int argc, char *argv[]) {
	
	int result = EXIT_SUCCESS;
	int32_t y = 0;
	int32_t d = 0;
	int32_t abs_month = 0;
	int32_t abs_equinox = 0;
	int32_t year_drift = 0;
	int32_t min_drift = 0;
	int32_t max_drift = 0;
	int32_t gr_year = 0;
	int32_t gr_month = 0;
	int32_t gr_day = 0;
	int32_t gr_equinox = 0;
	int32_t earliest_month = -1;
	int32_t earliest_day = -1;
	int32_t latest_month = -1;
	int32_t latest_day = -1;
	
	/* Verify the total number of custom parameters and get the decimal
	 * arguments */
	if (getCustomCount(argc) != 1) {
		fprintf(stderr,
			"newyear expects no additional arguments!\n");
		result = EXIT_FAILURE;
	}
	
	/* Generate the report */
	if (result != EXIT_FAILURE) {
		/* Go through each year */
		for(y = NELSC_CYCLE_YEARMIN; y <= NELSC_CYCLE_YEARMAX; y++) {
			
			/* If this row is not the first row and its row index is a
			 * multiple of four, prefix a blank line */
			if ((y != NELSC_CYCLE_YEARMIN) &&
				((y - NELSC_CYCLE_YEARMIN) % 4 == 0)) {
				printf("\n");
			}
			
			/* Begin by printing the year */
			base24_printPair(stdout, y);
			
			/* Convert the year into a day offset and an absolute
			 * month */
			abs_month = nelsc_cycle_yearToMonth(y);
			d = nelsc_cycle_monthToDay(abs_month);

			/* Apply Gregorian offset to convert it to Gregorian */
			d += NELSC_CYCLE_GROFFS;
			
			/* Split into Gregorian year-month-day */
			grcal_offsetToDate(d, &gr_year, &gr_month, &gr_day);

			/* Figure out the equinox offset that year */
			grcal_dateToOffset(
				&gr_equinox, gr_year, EQUINOX_MONTH, EQUINOX_DAY);

			/* Get the NELSC absolute month of the equinox -- except in
			 * the first year, use one less than the least month, since
			 * the NELSC calendar doesn't go that far back */
			if (y > NELSC_CYCLE_YEARMIN) {
				abs_equinox = nelsc_cycle_dayToMonth(
								gr_equinox - NELSC_CYCLE_GROFFS,
								NULL);
			} else {
				abs_equinox = abs_month - 1;
			}

			/* Compute the year drift */
			year_drift = abs_equinox - abs_month;

			/* If this is first year, use value to initialize minimum
			 * and maximum statistics; else, update the statistics
			 * appropriately */
			if (y == NELSC_CYCLE_YEARMIN) {
				min_drift = year_drift;
				max_drift = year_drift;
			} else {
				if (year_drift < min_drift) {
					min_drift = year_drift;
				}
				if (year_drift > max_drift) {
					max_drift = year_drift;
				}
			}
			
			/* Print the rest of the line */
			printf("  ");
			grcal_printDate(stdout, gr_year, gr_month, gr_day);
			printf("  equinox month offset %2d\n", (int) year_drift);
			
			/* Update the earliest and latest first day statistics */
			if (earliest_month == -1) {
				earliest_month = gr_month;
				earliest_day = gr_day;
			}
			
			if ((earliest_month > gr_month) ||
				((earliest_month == gr_month) &&
					(earliest_day > gr_day))) {
				earliest_month = gr_month;
				earliest_day = gr_day;
			}
			
			if (latest_month == -1) {
				latest_month = gr_month;
				latest_day = gr_day;
			}
			
			if ((latest_month < gr_month) ||
				((latest_month == gr_month) &&
					(latest_day < gr_day))) {
				latest_month = gr_month;
				latest_day = gr_day;
			}
		}
		
		/* Print the range of new year times and year drifts */
		printf("\n");
		printf("Range of first day of year:  %02ld-%02ld - "
				"%02ld-%02ld\n",
					(long) earliest_month,
					(long) earliest_day,
					(long) latest_month,
					(long) latest_day);
		printf("Range of equinox offsets:    [%d, %d]\n",
					(int) min_drift,
					(int) max_drift);
	}
	
	/* Return result */
	return result;
}

/*
 * The program entrypoint.
 * 
 * This checks the first custom argument and calls through to one of the
 * subprogram procedures.  If there are no custom arguments, it calls
 * through to the sub_help() program.
 * 
 * Parameters:
 * 
 *   argc - the number of program arguments in argv
 * 
 *   argv - an array of null-terminated strings representing the program
 *   arguments, with argc elements; the first element is name of the
 *   program module, while the second element is the first custom
 *   argument (if it exists)
 * 
 * Return:
 * 
 *   EXIT_SUCCESS if successful, EXIT_FAILURE if unsuccessful; passing
 *   through no custom arguments to invoke sub_help() results in success
 * 
 * Faults:
 * 
 *   - If argc is negative
 * 
 *   - If argv is NULL
 * 
 *   - If any element of argv is NULL
 * 
 * Undefined behavior:
 * 
 *   - If argv has fewer elements than are indicated with argc
 * 
 *   - If any string indicated by argv is not null-terminated
 */
int main(int argc, char *argv[]) {
	int retval = 0;
	const char *spname = NULL;
	
	/* Get the first custom argument, which selects the subprogram; an
	 * empty string is returned if there are no custom arguments */
	spname = getCustom(argc, argv, 0);
	
	/* Call through to the appropriate subprogram procedure */
	if ((strcmp(spname, "") == 0) || (strcmp(spname, "help") == 0)) {
		retval = sub_help();
	
	} else if (strcmp(spname, "to24pair") == 0) {
		retval = sub_to24pair(argc, argv);
	
	} else if (strcmp(spname, "from24pair") == 0) {
		retval = sub_from24pair(argc, argv);
	
	} else if (strcmp(spname, "to24digit") == 0) {
		retval = sub_to24digit(argc, argv);
		
	} else if (strcmp(spname, "from24digit") == 0) {
		retval = sub_from24digit(argc, argv);
		
	} else if (strcmp(spname, "day") == 0) {
		retval = sub_day(argc, argv);
		
	} else if (strcmp(spname, "month") == 0) {
		retval = sub_month(argc, argv);
		
	} else if (strcmp(spname, "date") == 0) {
		retval = sub_date(argc, argv);
		
	} else if (strcmp(spname, "fullmoon") == 0) {
		retval = sub_fullmoon(argc, argv);
		
	} else if (strcmp(spname, "newyear") == 0) {
		retval = sub_newyear(argc, argv);
		
	} else {
		/* Unrecognized subprogram argument */
		fprintf(stderr,
			"Unrecognized command.  Use \"help\" for help.\n");
		retval = EXIT_FAILURE;
	}
	
	/* Return the result */
	return retval;
}
