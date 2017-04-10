# NELSC Application

The Nome Epoch Lunisolar Calendar (NELSC) application, developed by
Canidlogic (http://www.canidlogic.com)

This is the official implementation of the NELSC calendar algorithms.
This README file describes the basic rules of the calendar system and
how to use this software implementation.  However, discussing the
justification and rationale behind the NELSC calendar is out of scope
for this README.

## 1. Calendar definition

This section provides the basic rules and conventions of the NELSC
calendar.

### 1.1 Base-24 numbers

Some elements of the NELSC date notation make use of a base-24 counting
system that is unique to NELSC.  The digits of this counting system are
as follows:

> 0 1 2 3 4 5 6 7 8 9 A B C D E F G M P R T V X Y

By convention, uppercase letters are used for the alphabetic digits in
base-24.

Base-24 numbers may be in "signed" or "unsigned" style.  In NELSC,
unsigned style is only used with single-digit base-24 numbers.  In this
case, the numeric value of the number equals the numeric value of the
digit.  Hence, "2" has an unsigned value of 2, and "Y" has an unsigned
value of 23.  The "to24digit" and "from24digit" subprograms of the NELSC
application allow conversions between decimal values and single-digit
unsigned base-24 numbers.

Signed base-24 style in NELSC is only used with two-digit base-24
numbers.  In this case, the numeric value of the number normally equals
the unsigned value of the first digit multiplied by 24 (decimal), added
to the unsigned value of the second digit, just as one would expect in a
base-24 counting system.

However, if the first digit is "T", "V", "X" or "Y", then the numeric
value encodes a negative number.  To determine the negative value, first
compute the unsigned value by adding together the unsigned value of the
first digit multiplied by 24 with the unsigned value of the second
digit, as described earlier.  Then, subtract this unsigned value by 576
(decimal) to yield the negative value.

As a result of this, "YY" is -1, "YX" is -2, "YV" is -3, and so forth,
down to "T0", which is -96 (decimal).  "RY", meanwhile, is the greatest
positive value, which is 479 (decimal).

The "to24pair" and "from24pair" subprograms of the NELSC application are
able to convert between signed-style base-24 pairs and their
corresponding signed decimal values.

### 1.2 Absolute day offsets

The most basic date functionality provided by NELSC is the _absolute day
offset,_ which merely counts the days that have passed since a fixed
reference event.  The fixed reference event happened on the Gregorian
date of February 2, 1925.  Hence, absolute day offset zero corresponds
to February 2, 1925, absolute day offset positive one corresponds to
February 3, 1925, absolute day offset negative two corresponds to
January 31, 1925, and so forth.

Absolute day offsets are expressed as signed decimal numbers.  The
base-24 conventions described in the previous section are not used for
absolute day offsets.  The full range of NELSC absolute day offsets are
from -35,364 (April 7, 1828) to 175,020 (April 11, 2404), inclusive of
boundaries.

The NELSC application subprogram "day" provides useful information about
a particular absolute day offset, including its mapping to the other
date systems of NELSC and its Gregorian calendar equivalent.

Furthermore, the "month" and "date" subprograms of the NELSC application
will include the absolute day offset in the reports that they return.

### 1.3 Absolute month offsets

NELSC organizes days into months, which can be uniquely referenced with
an _absolute month offset._  NELSC months always have either exactly 28
days (short months) or exactly 35 days (long months).  There is no
simple mapping between NELSC months and Gregorian calendar months.

Absolute month offsets are expressed as signed decimal numbers, just
like absolute day offsets.  Absolute month offset zero corresponds to
the month that includes absolute day offset zero, absolute month offset
negative one corresponds to the month before this, absolute month offset
positive one corresponds to the month after absolute month zero, and so
forth.

The full range of NELSC absolute month offsets are from -1,197 to 5,926.
The first day of month -1,197 is absolute day offset -35,364, and the
last day of month 5,926 is 175,020, so the range of days covered by
absolute days and absolute months is exactly the same.

Long and short months fall into a fixed pattern.  The pattern repeats
every 32 months, and is aligned such that a 32-month pattern begins 10
months before absolute month zero.  (In other words, absolute month zero
is the eleventh month in a repeating 32-month pattern.)

The 32-month pattern of long and short months is as follows, where "S"
refers to a short month and "L" refers to a long month:

> SSLS SSSLS SSLS SSSLS SSLS SSSLS SSSLS

The "month" command of the NELSC application provides information about
a particular absolute month offset.  Furthermore, the "day" and "date"
subprograms include the absolute month offset in the reports that they
return.

### 1.4 Full moon week

Months in NELSC are designed such that the full moon (almost) always
happens in a specific time frame of the calendar month.

To make use of this feature of NELSC, consider months to be organized
into 7-day weeks.  28-day short months thus include exactly four weeks,
while 35-day long months thus include exactly five weeks.

_Full moon week_ is always the second-to-last week of the calendar month
(the third week in short months and the fourth week in long months).
The full moon _almost_ always happens during full moon week.  However,
the patterns used by NELSC are not sophisticated enough to track the
full moon with high accuracy, so sometimes the actual full moon drifts a
bit out of bounds from full moon week.

The NELSC application provides the "fullmoon" subprogram to print out
the Gregorian dates of the full moon weeks for a given range of absolute
month offsets.  This can be compared to an astronomical table of full
moons to study how accurate the NELSC full moon predictions are.

### 1.5 Years

NELSC months are organized into years.  A year has either exactly twelve
months (short years) or exactly thirteen months (long years).

The pattern of long and short years is organized into 21 instances of an
11-year span (for a total of 231 years for the full pattern).  The last
instance of the 11-year span is slightly different from the others
within each 231-year pattern.  The regular 11-year span pattern of long
and short years is:

> SL SL SSL SSL S

The last 11-year span of each 231-year pattern changes the final year in
the pattern to a long year:

> SL SL SSL SSL L

NELSC years are written using a signed, two-digit base-24 number, using
the conventions described in an earlier section.  The month indicated by
absolute month offset zero is the eleventh month of the year zero.  The
valid range of years is T0 (-96 decimal) up to RY (479 decimal),
inclusive of boundaries.  The first valid day offset and the first valid
month offset occur at the start of T0, and the last valid day offset and
the last valid month offset occur at the end of RY, so the valid ranges
of NELSC days, months, and years are exactly the same.

The pattern of short and long years is aligned such that a 231-year
pattern begins 121 years before the year zero.  Note that this year is
actually out of range of the NELSC calendar -- the out-of-range years
are treated as if they existed for the purpose of determining which
years are long and which are short.

### 1.6 Dates

A complete NELSC date is written in the following format:

> 3T:C4-7

In this example, "3T" is the NELSC year, "C" is the unsigned base-24
digit indicating the twelfth month of the year, "4" means the fourth
week of this month, and "7" means the seventh day of this week.  Hence,
the full date means the seventh day of the fourth week of the twelfth
month of the NELSC year 3T.  Note that the month, week, and day numbers
start with one as the first element, not zero.

The "date" subprogram of the NELSC application accepts either a complete
NELSC date or a Gregorian date in the YYYY-MM-DD format, and reports
various information about the date.

### 1.7 Equinoxes

The NELSC year is defined such that the March equinox (which begins the
spring season in the northern hemisphere) always happens before the end
of the first month of the year.

For all valid NELSC years, the March equinox _always_ happens either in
the first month of the year, or the last month of the prior year, or in
the second-to-last month of the prior year.  The first day of the year
in NELSC may occur anywhere between March 4 and May 3 relative to the
Gregorian calendar.

The "newyear" subprogram of the NELSC application reports for each NELSC
year the Gregorian date of the first day of the year, along with which
month the equinox happens in -- 0 means first month of year, -1 means
last month of prior year, -2 means second-to-last month of prior year.
Note that the NELSC application currently approximates the March equinox
as always happening on March 20 in the Gregorian year, which is
sometimes a day or so off from the actual date of the equinox.

## 2. Program documentation

The NELSC application is a set of C language source and header files
that has no dependencies apart from the standard library.  Building the
application might be as simple as:

> gcc -o nelsc *.c

Running the program without any arguments prints out a list of all the
subprograms that are supported.  To run a particular subprogram, name
the subprogram as the first argument and then any further parameters as
additional arguments after that.  For example, to report information
about the NELSC date 3V:14-1, issue the following command:

> ./nelsc date 3V:14-1

## 3. Contact information

The NELSC calendar system was designed by Noah Johnson:

Name         | Email                       | GitHub
------------ | --------------------------- | --------------------------
Noah Johnson | noah.johnson@canidlogic.com | https://github.com/nomejoh

The NELSC application was written by Noah Johnson for Canidlogic (a
service of Multimedia Data Technology, Inc.):

Organization | Website                   | GitHub
------------ | ------------------------- | -----------------------------
Canidlogic   | http://www.canidlogic.com | https://github.com/canidlogic

This software is provided under an MIT license, the details of which can
be read in the LICENSE file.
