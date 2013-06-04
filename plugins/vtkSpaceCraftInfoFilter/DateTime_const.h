#ifndef DATETIME_CONST_H_
#define DATETIME_CONST_H_

#include <limits>

static double EPSILON = std::numeric_limits<double>::epsilon();
/// Days per month
/**  0   1   2   3   4   5   6   7   8   9  10  11  12
 *
 *  dec jan feb mar apr may jun jul aug sep oct nov dec
 */
static int DAYS_PER_MONTH[2][13] = {
  {31,31,28,31,30,31,30,31,31,30,31,30,31},
  {31,31,29,31,30,31,30,31,31,30,31,30,31}  
};
static double epochJulianDate = 2400001; // mjd = jd - 2400001
static size_t epochYear = 1858;
static size_t epochMonth = 11;
static size_t epochDay = 17;
static size_t epochHours = 0;
static size_t epochMinutes = 0;
static double epochSeconds = 0;

#endif /* DATETIME_CONST_H_ */
