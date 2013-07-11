#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <algorithm>
#include <cassert>
#include <math.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#include "DateTime_const.h"

/// Date and Time managment
/**
 *  Class to handle Date and Time in two formats:
 *
 *   1.   YYYY/MM/DD HH:MM:SS
 *
 *   2.   Modified Julian Date
 *
 * The Julian day or Julian day number (JDN) is the integer number of
 * days that have elapsed since the initial epoch defined as noon
 * Universal Time (UT) Monday, January 1, 4713 BC in the proleptic
 * Julian calendar.  That noon-to-noon day is counted as Julian day 0.
 *
 * The Modified Julian Day (MJD) is the number of days (with decimal
 * fraction of the day) that have elapsed since midnight at the
 * beginning of Wednesday November 17, 1858.  In terms of the Julian
 * day:
 *
 *  \f[
 *     MJD = JD - 2400000.5
 *  \f]
 *
 * @author Peter Schmitt
 * @version January 3, 2008
 */
class DateTime
{
public:
  // Constructors:    
  DateTime(void);
  DateTime(const size_t &YEAR, const size_t &MONTH, const size_t &DAY, 
	   const size_t &HOURS, const size_t &MINUTES, const double &SECONDS);
  DateTime(const double & MJD);
  DateTime(const double & MJD, const double &fractionOfDay);

  // Set Member Data:
  /// Set modified julian date and update YYYY-MM-DD @ HH:MM:SS
  void setMJD(const double &MJD) { fractionOfDay = modf(MJD, &mjd); updateYMDHMS(); }
  /// Set year and update modified julian date
  void setYear(const size_t &YEAR) { year = YEAR; updateMJD(); }
  /// Set month and update modified julian date
  void setMonth(const size_t &MONTH) { month = MONTH; updateMJD(); }
  /// Set day and update modified julian date
  void setDay(const size_t &DAY) { day = DAY; updateMJD(); }
  /// Set hours and update modified julian date
  void setHours(const size_t &HOURS) { hours = HOURS; updateMJD(); }
  /// Set minutes and update modified julian date
  void setMinutes(const size_t &MINUTES) { minutes = MINUTES; updateMJD(); }
  /// Set seconds and update modified julian date
  void setSeconds(const double &SECONDS) { seconds = SECONDS; updateMJD(); }

  // Modify Member Data:
  /// Increment the current MJD by delta_MJD and update YYYY-MM-DD @ HH:MM:SS
  void incrementMJD(const double &delta_MJD);
  /// Increment the current year by delta_YEAR and update modified julian date
  void incrementYear(const size_t &delta_YEAR) { year += delta_YEAR; updateMJD(); }
  /// Increment the current month by delta_MONTH and update modified julian date
  void incrementMonth(const size_t &delta_MONTH) { month += delta_MONTH; updateMJD(); }
  /// Increment the current day by delta_DAY and update modified julian date
  void incrementDay(const size_t &delta_DAY) { day += delta_DAY; updateMJD(); }
  /// Increment the current hours by delta_HOURS and update modified julian date
  void incrementHours(const size_t &delta_HOURS) { hours += delta_HOURS; updateMJD(); }
  /// Increment the current minutes by delta_MINUTES and update modified julian date
  void incrementMinutes(const size_t &delta_MINUTES) { minutes += delta_MINUTES; updateMJD(); }
  /// Increment the current seconds by delta_SECONDS and update modified julian date
  void incrementSeconds(const double &delta_SECONDS) { seconds += delta_SECONDS; updateMJD(); }

  // Access Member Data:
  /// Get the current modified julian date
  double getMJD(void) const { return (mjd+fractionOfDay); }
  // get percentage of day elapsed
  double getFractionOfDay(void) const { return fractionOfDay; }
  /// Get the current year
  size_t getYear(void) const { return year; }
  /// Get the current month
  size_t getMonth(void) const { return month; }
  /// Get the current day
  size_t getDay(void) const { return day; }
  /// Get the current hours
  size_t getYears(void) const { return year; }
  /// Get the current month
  size_t getMonths(void) const { return month; }
  /// Get the current day
  size_t getDays(void) const { return day; }
  /// Get the current hours
  size_t getHour(void) const { return hours; }
  /// Get the current minutes
  size_t getMinute(void) const { return minutes; }
  /// Get the current seconds
  double getSecond(void) const { return seconds; }  
  /// Get the current hours
  size_t getHours(void) const { return hours; }
  /// Get the current minutes
  size_t getMinutes(void) const { return minutes; }
  /// Get the current seconds
  double getSeconds(void) const { return seconds; }  

  /// Get the number of days elapsed since Epoch date
  double getDaySinceEpoch(void) const { return mjd; }
  /// Get the number of hours elapsed since Epoch date
  double getHoursSinceEpoch(void) const { return 24*this->getMJD(); }
  /// Get the number of minutes elapsed since Epoch date
  double getMinutesSinceEpoch(void) const { return 1440*this->getMJD(); } // 1440 = 24*60
  /// Get the number of seconds elapsed since Epoch date
  double getSecondsSinceEpoch(void) const { return 86400*this->getMJD(); } // 86400 = 24*60*60

  /// Get the day of the year
  size_t getDayOfYear(void) const { return dayOfYear(); }

  // Output:
  friend std::ostream& operator << (std::ostream& output, const DateTime& time);
  std::string getDateTimeString(void) const;
  std::string getISO8601DateTimeString(void) const;

  // Addition & Subtraction
  void operator += (const DateTime & date);
  void operator -= (const DateTime & date);
 
private:  
  /// Increment fraction of day & update MJD if necessary.
  void incrementFractionOfDay(const double &delta_frac);

  size_t dayOfYear(void) const;
  double secOfDay(void) const;

  void updateMJD(void);
  void updateYMDHMS(void);
  
  void verifyYMDHMS(void);

  /// Modified Julian Date
  double mjd; 
  double fractionOfDay;

  /// Year corresponding to mjd
  size_t year;
  /// Month of yearcorresponding to mjd
  size_t month;
  /// Day of month correspdonging to mjd
  size_t day;
  /// Hours of day corresponding to mjd
  size_t hours;
  /// Minutes of hour corresponding to mjd
  size_t minutes;
  /// Seconds of minute corresponding to mjd
  double seconds;
};

// Addition & Subtraction
//
// Overloaded as nonmember functions because they should not change member data.
DateTime operator + (const DateTime &date1, const DateTime &date2);
DateTime operator - (const DateTime &date1, const DateTime &date2);

// DateTime comparison
//
// overloaded as nonmember functions to compare both const DateTime
// objects (Compiler should automatically promote non-const to const
// for both arguments).
bool operator  < (const DateTime &d1, const DateTime &d2);
bool operator <= (const DateTime &d1, const DateTime &d2);
bool operator  > (const DateTime &d1, const DateTime &d2);
bool operator >= (const DateTime &d1, const DateTime &d2);
bool operator == (const DateTime &d1, const DateTime &d2);

#endif
