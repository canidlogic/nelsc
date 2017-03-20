/*
 * base24.c
 * 
 * Implementation of base24.h
 * 
 * See the header for further information.
 */

#include "base24.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*
 * The maximum value of an *unsigned* base-24 pair.
 */
#define UPAIR24_MAX (576)

/*
 * The base-24 digits (in uppercase).
 */
static const char *m_base24 = "0123456789ABCDEFGMPRTVXY";

/*
 * base24_pairToInt function.
 */
bool base24_pairToInt(const char *str, int32_t *pResult) {
	bool result = true;
	int32_t digit_most = 0;
	int32_t digit_least = 0;
	int32_t val = 0;
	
	/* Check parameters */
	if ((str == NULL) || (pResult == NULL)) {
		abort();
	}
	
	/* Fail immediately if *str is null, so we don't read past the end
	 * of the string */
	if (*str == 0) {
		result = false;
	}
	
	/* Convert the most significant and least significant digits of the
	 * pair */
	if (result) {
		digit_most = base24_digitToInt(str[0]);
		digit_least = base24_digitToInt(str[1]);
		
		if ((digit_most == -1) || (digit_least == -1)) {
			result = false;
		}
	}
	
	/* Convert the digits into a signed value */
	if (result) {
		/* Get the unsigned value */
		val = (digit_most * 24) + digit_least;
		
		/* If greater than BASE24_PAIR_MAX, convert to negative number
		 * by subtracting by UPAIR24_MAX */
		if (val > BASE24_PAIR_MAX) {
			val = val - UPAIR24_MAX;
		}
	}
	
	/* If we succeeded, write the full result to *pResult */
	if (result) {
		*pResult = val;
	}
	
	/* Return result */
	return result;
}

/*
 * base24_printPair function.
 */
void base24_printPair(FILE *pFile, int32_t v) {
	
	int32_t digit_most = 0;
	int32_t digit_least = 0;
	
	/* Check parameters */
	if ((pFile == NULL) || (v < BASE24_PAIR_MIN) ||
			(v > BASE24_PAIR_MAX)) {
		abort();
	}
	
	/* If v is negative, convert to unsigned by adding to UPAIR24_MAX */
	if (v < 0) {
		v += UPAIR24_MAX;
	}
	
	/* Extract the two base-24 digits */
	digit_least = v % 24;
	digit_most = v / 24;
	
	/* Print the two base-24 characters, failing if not exactly two
	 * characters were output */
	if (fprintf(
				pFile,
				"%c%c",
				base24_intToDigit(digit_most),
				base24_intToDigit(digit_least)) != 2) {
		abort();
	}	
}

/*
 * base24_digitToInt function.
 */
int32_t base24_digitToInt(char c) {
	
	char *pc = NULL;
	int32_t result = 0;
	
	/* Convert the digit to uppercase */
	c = (char) toupper(c);
	
	/* Find the digit in the base-24 digit string, fail if not a base-24
	 * digit */
	pc = strchr(m_base24, c);
	if (pc == NULL) {
		result = -1;
	}
	
	/* Figure out the digit's value from its offset within the string */
	if (result != -1) {
		result = (int32_t) (pc - m_base24);
	}
	
	/* Return result */
	return result;
}

/*
 * base24_intToDigit function.
 */
char base24_intToDigit(int32_t v) {
	/* Check range */
	if ((v < 0) || (v > BASE24_DIGIT_MAX)) {
		abort();
	}
	
	/* Return digit by looking up in base-24 digit string */
	return m_base24[v];
}
