#ifndef HDF4_HPP__
#define HDF4_HPP__

#define IO_BASE_DEFINITION__
#include "Io.hpp"

#ifdef HAS_HDF4
#include <mfhdf.h>
#endif

#include <map>
#include <csignal>

#define ERRORCHECK(STATUS) errorCheck(STATUS, __FILE__, __LINE__, __FUNCTION__)

class Hdf4 : public Io {

 public:

  Hdf4(int superDomainSize);
  ~Hdf4();

  bool isEnabled();

  bool openRead(const string& filename);
  bool openWrite(const string& filename);

  bool readVariable( const string& variable, 
		     const string& group,
		     const array_info_t& info,
		     void* data );

  int readAttribute( const string& variable,
		     const string& group,
		     const identify_data_type& dataType,
		     void* data,
		     const int& len=1 );
  
  void writeVariable( const string& variable, 
		      const string& group,
		      const array_info_t& info,
		      const void* data );

  void writeAttribute( const string& variable,
		       const string& group,
		       const identify_data_type& dataType,
		       const void* data,
		       const int& len=1 );

  void getBcastArrayInfo( const string& group,
			  array_info_t& info );
  
  void getLocalArrayInfo( const string& group,
			  array_info_t& info );
  
  void putArrayInfo( const string& group,
		     const array_info_t& info );


  bool verifyShape( const string& variable,
		    const string& group,
		    const array_info_t& info );
  
  const list<string> getVarNames();

  void close();

 protected:

#ifdef HAS_HDF4

  map<string,int32> h4groups;
  
  template<class T> int32* int32_convert(const T* v, const int& s, int32* n, 
					 const int& max=MAX_ARRAY_DIMENSION) const {
    for (int i=0; i<max; i++) n[i]=(i<s?v[i]:0);
    return n;
  }

  int32 identifyH4Type( const identify_data_type& dataType, const string& v ) const {
    switch (dataType) {
    case identify_byte_t:   return DFNT_UCHAR8;
    case identify_char_t:   return DFNT_CHAR8;
    case identify_string_t: return DFNT_CHAR8;
    case identify_short_t:  return DFNT_INT16;
    case identify_int_t:    return DFNT_INT32;
    case identify_long_t:   return DFNT_INT64;
    case identify_float_t:  return DFNT_FLOAT32;
    case identify_double_t: return DFNT_FLOAT64;
    case identify_unknown_t:
    default:                
      cerr << "Unknown type for identifyH4Type with variable " << v << endl;
      raise(SIGABRT);
      return -1;
    }
  }

  identify_data_type H4identifyType( const int32 h4type, const string& v ) const {
    switch (h4type) {
    case DFNT_UCHAR8:   return identify_byte_t;
    case DFNT_CHAR8:    return identify_char_t;
    case DFNT_INT16:    return identify_short_t;
    case DFNT_INT32:    return identify_int_t;
    case DFNT_INT64:    return identify_long_t;
    case DFNT_FLOAT32:  return identify_float_t;
    case DFNT_FLOAT64:  return identify_double_t;
    default:                
      cerr << "Unknown type for H4identifyType with variable " << v << endl;
      raise(SIGABRT);
      return identify_unknown_t;
    }
  }

  int32 openGroup(const string& groupName);

  int32 createGroup(const string& groupName);

  void errorCheck(const int& status, const char* file, const int& line, const char* func);  
  
  bool open(const string& filename, const int32& accessMode);

  int32 sdId;

#else
  int sdId;
#endif //HAS_HDF4

};

#endif //HDF4_HPP__
