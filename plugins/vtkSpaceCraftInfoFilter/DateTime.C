#include "DateTime.h"

////////////////////////////////////////////////////////////////////////

/// DateTime default constructor
/**
 *  Set DateTime to the Modified Julian Day epoch: Wednesday, november
 *  17, 1858 (mjd = 0)
 *
 */
DateTime::DateTime(void)
{
  year = 1858;
  month = 11;
  day = 17;
  hours = 0;
  minutes = 0;
  seconds = 0.0;
  
  updateMJD();

  // Make sure we computed the correct modified julian date.  
  // (Epoch should be MJD=0.0.)
  assert (*this ==  DateTime(0.0) );
}

////////////////////////////////////////////////////////////////////////

/// DateTime YMDHMS constructor
/**
 *  Set DateTime to YEAR-MONTH-DAY @ HOURS-MINUTES-SECONDS and compute
 *  the modified julian date.  
 *  @param YEAR - integer Year to compute MJD (positive values only)
 *  @param MONTH - integer Month of year to compute MJD (positive values only)
 *  @param DAY - integer Day of month to compute MJD (positive values only)
 *  @param HOURS - integer Hours of day to compute MJD (positive values only)
 *  @param MINUTES - integer Minutes of day to compute MJD (positive values only)
 *  @param SECONDS - floating-point (double) Seconds of day to compute MJD (positive values only)
 *  @see DateTime(const double &MJD)
 *  @see DateTime(void)
 */
DateTime::DateTime(const size_t &YEAR, const size_t &MONTH, const size_t &DAY, 
		   const size_t &HOURS, const size_t &MINUTES, const double &SECONDS)
{
  year = YEAR;
  month = MONTH;
  day = DAY;
  hours = HOURS;
  minutes = MINUTES;
  seconds = SECONDS;  

  updateMJD();
}

////////////////////////////////////////////////////////////////////////

/// DateTime Modified Julian Date constructor
/**
 *  Set DateTime to the modified julian date & compute YEAR-MONTH-DAY
 *  @ HOURS-MINUTES-SECONDS
 * @param MJD - Modified Julian Date to use to compute YYYY-MM-DD HH:MM:SS.  Negative mjd steps backwards from the MJD epoch 1858-11-17T00-00-00Z
 * @see DateTime(const size_t &YEAR, const size_t &MONTH, const size_t &DAY, const size_t &HOURS, const size_t &MINUTES, const double &SECONDS)
 * @see DateTime(void)
 */
DateTime::DateTime(const double &MJD)
{
  fractionOfDay = modf(MJD, &mjd);

  updateYMDHMS();
}

////////////////////////////////////////////////////////////////////////

DateTime::DateTime(const double &MJD, const double & FRACTIONOFDAY)
{
  // MJD might have decimal portion
  fractionOfDay = modf(MJD, &mjd);
  // Now update from the input fraction of day.
  incrementFractionOfDay(FRACTIONOFDAY);

  updateYMDHMS();
}

////////////////////////////////////////////////////////////////////////

void DateTime::incrementMJD(const double &delta_MJD)
{
  double nDays, deltaFracOfDay;

  deltaFracOfDay = modf(delta_MJD, &nDays);

  mjd += nDays;
  
  incrementFractionOfDay(deltaFracOfDay);

  updateYMDHMS(); 
}

////////////////////////////////////////////////////////////////////////

/// private member function to properly handle day boundaries when updating fractional component of a day.
void DateTime::incrementFractionOfDay(const double &delta_frac)
{
  fractionOfDay += delta_frac;

  // make sure we don't cross a day boundary
  if (fractionOfDay >= 1.0){
    double elapsedDays = 0.0;
    fractionOfDay = modf(fractionOfDay, &elapsedDays);
    mjd += elapsedDays;
  }

  // Phew. That sure was a frack of a day!
}

////////////////////////////////////////////////////////////////////////

/// Compute the day of year using month & day
/**
 *  i.e. January 31 = 31st day; February 32 = 32nd day...
 * @return the number of days since January 1
 */
size_t DateTime::dayOfYear(void) const
{
  int i;  // loop counter
  size_t doy; // day of year

  // is it a leap year?
  bool isLeapYear = false;
  if (  ( (year%4) == 0 ) && ( (year%100) != 0 )  )
    isLeapYear = true;
  if ( (year%400) == 400 )
    isLeapYear = true;

  // note: false=0 and true=1 

  doy = day;

  for (i = month-1; i > 0; i--)
    doy += DAYS_PER_MONTH[isLeapYear][i];

  return doy;
}

////////////////////////////////////////////////////////////////////////

/// Compute the seconds elapsed in a day using hours, minutes & seconds of the day
/**
 * @return the seconds elapsed in the current day
 */
double DateTime::secOfDay(void) const
{
  double sod; // seconds of day

  sod  = (seconds
	  + (double) minutes * 60.0
	  + (double) hours * 3600.0); // 60*60 = 3600

  return (sod);
}

////////////////////////////////////////////////////////////////////////

/// Update modified julian date using the current Y-M-D H:M:S
/**
 *  Compute the Modified Julian Date using the current YYYY-MM-DD
 *  HH:MM:SS.  
 *
 *  This code assumes that the changeover from the Julian calendar to
 *  the Gregorian calendar occurred in October of 1582. Specifically,
 *  for dates on or before 4 October 1582, the Julian calendar is
 *  used; for dates on or after 15 October 1582, the Gregorian
 *  calendar is used.
 *
 *  For more information on Y-M-D H:M:S to Modified Julian Date, see:
 *   http://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
 *   http://en.wikipedia.org/wiki/Julian_day
 *   Numerical Recipes (3rd Edition)
 *   http://aa.usno.navy.mil/data/docs/JulianDate.php
 */
void DateTime::updateMJD(void)
{
  // If YMDHMS has any goofy values (ie. negative seconds), verify/correct it:
  setValidYMDHMS();

  long alpha;
  long julianDate;
  long y = year;
  long m;

  if (y < 0)
    y++;
  if (month > 2) 
    m = month+1;
  else{
    // think of January & February as the 13th & 14th month of the
    // previous year.
    y--;
    m = month+13;
  }
  julianDate = long(floor(365.25*y) + floor(30.6001*m) + day + 1720995);
  // Change Gregorian Calendar?
  // http://aa.usno.navy.mil/data/docs/JulianDate.php

  // Gregorian Calendar (Oct. 15, 1582)
  const long GREG = 15+31*(10+12*1582);

  if (day+31*(month+12*year) >= GREG){
    alpha = long(0.01*y);
    julianDate += 2-alpha+long(0.25*alpha);
  }

  // Convert to modified Julian date:
  mjd = julianDate - epochJulianDate;

  // add the fractional part of a day
  fractionOfDay = 0.0;
  incrementFractionOfDay( secOfDay() / 86400.0 ); // 86400 = 24*60*60 = # of seconds in a day.
}

////////////////////////////////////////////////////////////////////////

/// Update the Y-M-D H:M:S using the current modified julian date
/**
 *  Compute the YYYY-MM-DD HH:MM:SS using the specified Modified Julian Date
 *
 *  This code assumes that the changeover from the Julian calendar to
 *  the Gregorian calendar occurred in October of 1582. Specifically,
 *  for dates on or before 4 October 1582, the Julian calendar is
 *  used; for dates on or after 15 October 1582, the Gregorian
 *  calendar is used.
 *
 *  For more information on Y-M-D H:M:S to Modified Julian Date, see:
 *   http://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
 *   http://en.wikipedia.org/wiki/Julian_day
 *   Numerical Recipes (3rd Edition)
 *   http://aa.usno.navy.mil/data/docs/JulianDate.php
 */
void DateTime::updateYMDHMS(void)
{
  const long GREG=2299161; // JD for October 15, 1582 @ 12:00:00:00 UT
  long alpha, A, B, C, D, E;
  long julian = (long) mjd + (long) epochJulianDate; // Convert to standard Julian Date  
  
  if (julian >= GREG) {
    // Gregorian Calendar correction
    alpha = long((double(julian-1867216)-0.25)/36524.25);
    A = julian+1+alpha-long(0.25*alpha);
  }
  else if (julian < 0){
    // We want a positive day number: add # of julien centuries
    // (365.25*100).  correct this at the end.
    A = julian + 36525*(1-julian/36525);
  }
  else{
    A = julian;    
  }
  B = A+1524;
  C = long(6680.0 + (double(B-2439870)-122.1)/365.25);
  //C = long(B - (122.1) / 365.25);
  D = long(365.25*C);
  E = long((B-D)/30.6001);

  day = B-D-long(30.6001*E);

  // We are thinking of January & February as the 13th & 14th month of
  // the previous year
  month = E-1;

  // Make sure month is set right:
  if (month > 12)
    month -= 12;

  year = (month > 2) ? (C - 4716) : (C - 4715);
  if (year <=0)
    year--;
  // We wanted a positive day number... subtract # of julien centuries
  // (365.25*100)
  if (julian < 0)
    year -= 100*(1-julian/36525);

  // use fraction of day to find out how many hours/minutes/seconds in day
  double nHours = fractionOfDay * 24.0;
  hours = (size_t) floor(nHours+EPSILON);

  double nMinutes = (nHours - hours) * 60.0;
  minutes = (size_t) floor(nMinutes+EPSILON);

  double nSeconds = (double) (nMinutes-minutes) * 60.0;
  // round to the nearest tenth of a second
  seconds = floor(nSeconds+1e-1);
}

////////////////////////////////////////////////////////////////////////

/// Verify the Y-M-D H:M:S is valid.
/** Verify that the Year/Month/Day/Hours/Minutes/Seconds is valid
 *
 * For example:
 *
 * 1.    120 seconds should be 2 mintes
 *
 * 2.    What is February 29?  Leap year?
 *
 * 3.    December 32, 2007 is really January 1, 2008
 */
void DateTime::setValidYMDHMS(const long &year, const long &month, const long &day, const long &hours, const long &minutes, const double &seconds)
{
  long year_i = year;
  long month_i = month;
  long day_i = day;
  long hours_i = hours;
  long minutes_i = minutes;
  double seconds_d = seconds;
  
  /***** Check seconds *****/
  while (seconds_d >= 60.0){
    seconds_d -= 60.0;
    minutes_i++;
  }
  while (seconds_d < 0){
    seconds_d += 60.0;
    minutes_i--;
  }
  
  /***** Check minutes *****/
  while (minutes_i >= 60){
    minutes_i -= 60;
    hours_i++;
  }
  while (minutes_i < 0){
    minutes_i += 60;
    hours_i--;
  }

  /***** Check hours *****/
  while (hours_i >= 24){
    hours_i -= 24;
    day_i++;
  }
  while (hours_i < 0){
    hours_i += 24;
    day_i--;
  }

  /***** Check days *****/
  /* Is it a leap year?
   *
   * The Gregorian calendar, the current standard calendar in most of
   * the world, adds a 29th day to February in 97 years out of every
   * 400, a closer approximation than once every four years. This is
   * implemented by making every year divisible by 4 a leap year
   * unless that year is divisible by 100. If it is divisible by 100
   * it can only be a leap year if that year is also divisible by 400.
   * Source: http://en.wikipedia.org/wiki/Leap_year
   */
  bool isLeapYear = false;
  if (  ( (year_i%4) == 0 ) && ( (year_i%100) != 0 )  )
    isLeapYear = true;
  if ( (year_i%400) == 400 )
    isLeapYear = true;

  // note: false=0 and true=1 
  while (day_i >  DAYS_PER_MONTH[isLeapYear][month_i%12]){
    day_i = day_i - DAYS_PER_MONTH[isLeapYear][month_i%12];
    month_i++;
  }
  
  // We can only have positive numbers of days!
  while (day_i < 0){
    month_i--;

    if (month_i <= 0){
      month_i += 12;
      year_i--;
      if (  ( (year_i%4) == 0 ) && ( (year_i%100) != 0 )  )
	isLeapYear = true;
      if ( (year_i%400) == 400 )
	isLeapYear = true;
    }

    day_i += DAYS_PER_MONTH[isLeapYear][month_i%12];
  }


  /***** check months *****/
  while (month_i > 12){
    month_i -= 12;
    year_i++;
  }

  /*  What does it mean to decrement the date by 1 month?  Is -1
   *  month == 28, 29, 30 or 31 days?  
   */
  assert (month_i > 0);

  this->year = size_t(year_i);
  this->month = size_t(month_i);
  this->day = size_t(day_i);
  this->hours = size_t(hours_i);
  this->minutes = size_t(minutes_i); 
  this->seconds = seconds_d;
}

////////////////////////////////////////////////////////////////////////

/// Get formatted DateTime string in the format YYYY-MM-DDTHH-MM-SSZ
/**
 *  Returns a string of the form YYYY-MM-DDTHH-MM-SSZ.  This is useful
 *  for UTIO filenames, like we have used with CMIT.
 */
std::string DateTime::getDateTimeString(void) const
{
  std::string YYYY, MM, DD, HH, MMin, SS;
  std::stringstream out;

  out << year << "-";

  if (month < 10)
    out << "0" << month << "-";
  else
    out << month << "-"; 

  if (day < 10)
    out << "0" << day << " ";
  else
    out << day << " ";

  if (hours < 10)
    out << "0" << hours << ":";
  else
    out << hours << ":";

  if (minutes < 10)
    out << "0" << minutes << ":";
  else
    out << minutes << ":";

  if (seconds < 10.0)
    out << "0" << (size_t) seconds ;
  else
    out << (size_t) seconds ;

  return out.str();
}


////////////////////////////////////////////////////////////////////////

/// Get formatted DateTime string in the format YYYYMMDDTHHMMSSZ
/**
 *  Returns a string of the form YYYY-MM-DDTHH-MM-SSZ.  This is useful
 *  for UTIO filenames, like we have used with CMIT.
 */
std::string DateTime::getISO8601DateTimeString(void) const
{
  std::string YYYY, MM, DD, HH, MMin, SS;
  std::stringstream out;

  out << year;

  if (month < 10)
    out << "0" << month;
  else
    out << month;

  if (day < 10)
      out << "0" << day << "T";
  else
      out << day << "T";

  if (hours < 10)
    out << "0" << hours;
  else
    out << hours;

  if (minutes < 10)
    out << "0" << minutes;
  else
    out << minutes;

  if (seconds < 10.0)
      out << "0" << (size_t) seconds << "Z";
  else
      out << (size_t) seconds  << "Z";

  return out.str();
}

////////////////////////////////////////////////////////////////////////

/// Output DateTime object
/**
 *  Write DateTime information (Year-Month-Day @ hour:minute:second)
 *  and modified julian date to output stream.
 *
 *  @param output output stream to write DateTime object to
 *  @param time DateTime object to output
 *  @return output stream
 */
std::ostream& operator<<(std::ostream& output, const DateTime& time)
{
  output << "[YYYY-MM-DD] @ HH:MM:SS = "
	 << "[" << time.year << "-" << time.month << "-" << time.day
	 << "] @ " << time.hours << ":" << time.minutes << ":" << time.seconds
	 << std::endl;
  output << "Modified Julian Date    = " << std::setprecision(30) << time.getMJD() << std::endl;
  return output;
}

////////////////////////////////////////////////////////////////////////

/// this.mjd += date.mjd
/**
 *  Add date.mjd to the current time, update the Year-Month-Day
 *  Hours:Minutes:Seconds
 *  @param date - what to add to input.
 */
void DateTime::operator += (const DateTime & date)
{
  mjd += date.mjd;
  incrementFractionOfDay( date.fractionOfDay );
  updateYMDHMS();
}

////////////////////////////////////////////////////////////////////////

/// this.mjd -= date.mjd
/**
 *  Add date.mjd to the current time, update the Year-Month-Day
 *  Hours:Minutes:Seconds
 *  @param date - what to subtract from input.
 */
void DateTime::operator -= (const DateTime & date)
{
  mjd -= date.mjd;
  incrementFractionOfDay( -date.fractionOfDay );
  updateYMDHMS();
}

////////////////////////////////////////////////////////////////////////
// Non-member functions:
////////////////////////////////////////////////////////////////////////


/// date1 + date2
/**
 *  Add Modified Julian Dates and return DateTime object.
 *  @param date1
 *  @param date2
 *  @return new DateTime object that is the result of date1+date2
 */
DateTime operator + (const DateTime & date1, const DateTime &date2)
{
  return DateTime(date1.getMJD() + date2.getMJD());
}

////////////////////////////////////////////////////////////////////////

/// date1 - date2
/**
 *  Subtracted Modified Julian Dates and return DateTime object.
 *  @param date1
 *  @param date2
 *  @return new DateTime object that is the result of date1-date2
 */
DateTime operator - (const DateTime & date1, const DateTime &date2)
{
  return DateTime(date1.getMJD() - date2.getMJD());
}

////////////////////////////////////////////////////////////////////////


/// date1 < date2
/**
 *  Use modified julian date to determine if date1 is earlier than date2.
 *  @param date date to compare with
 *  @return true if input < date, false otherwise
 */
bool operator <(const DateTime &d1, const DateTime &d2)
{
  return (d1.getMJD() < d2.getMJD());
}

////////////////////////////////////////////////////////////////////////

/// date1 <= date2
/**
 *  Use modified julian date to determine if date1 is earlier  or equal to date2.
 *  @param date date to compare with
 *  @return true if input <= date, false otherwise
 */
bool operator <= (const DateTime &d1, const DateTime &d2)
{
  return (d1.getMJD() < d2.getMJD() + EPSILON);
//    return (d1.getMJD() < d2.getMJD());

}

////////////////////////////////////////////////////////////////////////

/// date1 > date2
/**
 *  Use modified julian date to determine if date1 is later than date2.
 *  @param date date to compare with
 *  @return true if input > date, false otherwise
 */
bool operator >(const DateTime &d1, const DateTime &d2)
{
  return (d1.getMJD() > d2.getMJD());
}

////////////////////////////////////////////////////////////////////////

/// date1 >= date2
/**
 *  Use modified julian date to determine if date1 is later than or equal to date2.
 *  @param date date to compare with
 *  @return true if input >= date, false otherwise
 */
bool operator >= (const DateTime & d1, const DateTime & d2)
{
  return (d1.getMJD() >= d2.getMJD());
}

////////////////////////////////////////////////////////////////////////

/// date1 == date2
/**
 *  Use modified julian date to determine if date1 is equal to date2.
 *  @param date date to compare with
 *  @return true if input == date, false otherwise
 */
bool operator == (const DateTime & d1, const DateTime & d2)
{
  return ( (d1.getMJD() <= d2.getMJD() + EPSILON) &&
       (d1.getMJD() >= d2.getMJD() - EPSILON) );
//  return (d1.getMJD() == d2.getMJD());
}

////////////////////////////////////////////////////////////////////////
