#ifndef UTIL_HPP__
#define UTIL_HPP__

#include <iostream>
#include <iomanip>
#include <sstream>
#include <typeinfo>
#include <stdint.h>

#define ERROR(INFO) {							\
    string file( __FILE__ );						\
    string func( __func__ );						\
    cerr << " - DEBUG ";						\
    cerr << left  << std::setw(30) << file.substr(0,30)			\
	 << ":" << std::setw(15) << func.substr(0,15) << ":";		\
    cerr << std::setw(5) << right << __LINE__ << ":";			\
    cerr << right << std::setw(5)  << mesgCount++  << " X";		\
    cerr << right << std::setw(3)  << std::setfill('0')			\
	 << -99 << std::setfill(' ');					\
    cerr << " | " << INFO << "\n" << flush;				\
    exit(-1);								\
  }

enum identify_data_type {
  identify_unknown_t = 0,
  identify_byte_t    = 100,
  identify_char_t    = 300,
  identify_string_t  = 333,
  identify_short_t   = 400,
  identify_int_t     = 500,
  identify_long_t    = 600,
  identify_float_t   = 700,
  identify_double_t  = 800,
};

template<class T> identify_data_type identify(const T& v) {
  const type_info& v_type = typeid(T);

  if      ( v_type == typeid(unsigned char) ) { return identify_byte_t;   }
  else if ( v_type == typeid(signed char) )   { return identify_char_t;   }
  else if ( v_type == typeid(char*) )         { return identify_string_t; }
  else if ( v_type == typeid(const char*) )   { return identify_string_t; }
  else if ( v_type == typeid(char[]) )        { return identify_string_t; }
  else if ( v_type == typeid(string) )        { return identify_string_t; }
  else if ( v_type == typeid(const string) )  { return identify_string_t; }

  else if ( v_type == typeid(int16_t)     )   { return identify_short_t;  }
  else if ( v_type == typeid(int32_t)     )   { return identify_int_t;    }
  else if ( v_type == typeid(int64_t)     )   { return identify_long_t;   }
  
  else if ( v_type == typeid(float)       )   { return identify_float_t;  }
  else if ( v_type == typeid(double)      )   { return identify_double_t; }
  
  else { 
    cerr << "Unknown type for identify" << endl;
    return identify_unknown_t; 
  }
}

static int identifySize(const identify_data_type& t) {
  switch(t) {
  case identify_byte_t:   return 1;
  case identify_char_t:   return 1;
  case identify_short_t:  return 2;
  case identify_int_t:    return 4;
  case identify_long_t:   return 8;
  case identify_float_t:  return 4;
  case identify_double_t: return 8;
  default:
    return 0;
  }
}

template <class T>
inline std::string toString (const T& t, const int width=0, const char fill=' ', const int precision=5 ) {
  std::stringstream ss;
  ss << std::scientific << std::setw(width) << std::setfill(fill) << std::setprecision(precision) << t;
  return ss.str();
}

template <class T>
inline std::string toStringSci (const T& t) {
  return toString(t,16,' ',7);
}

#ifndef NOPP
template<class PppArray> void displayInfo( const PppArray& data ){
  cout << "nDims:" 
       << data.numberOfDimensions() << "\n"
       << data.getLocalArray().numberOfDimensions() << "\n";
  cout << "LocalBase:"
       << data.getLocalBase(0) << ","
       << data.getLocalBase(1) << ","
       << data.getLocalBase(2) << ","
       << data.getLocalBase(3) << "\n";
  cout << "LocalStride:"
       << data.getLocalStride(0) << ","
       << data.getLocalStride(1) << ","
       << data.getLocalStride(2) << ","
       << data.getLocalStride(3) << "\n";
  cout << "LocalBound:"
       << data.getLocalBound(0) << ","
       << data.getLocalBound(1) << ","
       << data.getLocalBound(2) << ","
       << data.getLocalBound(3) << "\n";
  cout << "LocalLength:"
       << data.getLocalLength(0) << ","
       << data.getLocalLength(1) << ","
       << data.getLocalLength(2) << ","
       << data.getLocalLength(3) << "\n";
  cout << "LocalRawBase:"
       << data.getLocalRawBase(0) << ","
       << data.getLocalRawBase(1) << ","
       << data.getLocalRawBase(2) << ","
       << data.getLocalRawBase(3) << "\n";
  cout << "LocalRawStride:"
       << data.getLocalRawStride(0) << ","
       << data.getLocalRawStride(1) << ","
       << data.getLocalRawStride(2) << ","
       << data.getLocalRawStride(3) << "\n";
  cout << "LocalRawBound:"
       << data.getLocalRawBound(0) << ","
       << data.getLocalRawBound(1) << ","
       << data.getLocalRawBound(2) << ","
       << data.getLocalRawBound(3) << "\n";
  cout << "GhostCells:"
       << data.getGhostBoundaryWidth(0) << ","
       << data.getGhostBoundaryWidth(1) << ","
       << data.getGhostBoundaryWidth(2) << ","
       << data.getGhostBoundaryWidth(3) << "\n";
  cout << "InternalGhost:"
       << data.getInternalGhostCellWidth(0) << ","
       << data.getInternalGhostCellWidth(1) << ","
       << data.getInternalGhostCellWidth(2) << ","
       << data.getInternalGhostCellWidth(3) << "\n";
  cout << endl;
  cout.flush();
}
#endif//NOPP

#endif //UTIL_HPP__
