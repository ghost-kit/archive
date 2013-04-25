#ifndef HDF5_HPP__
#define HDF5_HPP__

#define IO_BASE_DEFINITION__
#include "Io.hpp"

#ifdef HAS_HDF5
#include <hdf5.h>
#endif

#include <map>
#include <csignal>

#ifdef HAS_HDF5
static const hid_t DATA_TYPE = H5T_NATIVE_FLOAT;
#endif

#define ERRORCHECK(STATUS) errorCheck(__FILE__, __LINE__, __FUNCTION__, #STATUS, STATUS)
#define PUSHERROR(E) pushError(E, __FILE__, __LINE__, __FUNCTION__)

class Hdf5 : public Io {

 public:

  Hdf5(int superDomainSize);
  ~Hdf5();

  bool enabled();

  bool openRead(const string &filename);
  bool openWrite(const string &filename);

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
  
#ifdef HAS_HDF5

  map<string,hid_t> h5groups;

  template<class T> hsize_t* hsize_convert(const T* v, const int &s, hsize_t *n, 
					   const int &max=MAX_ARRAY_DIMENSION) const {
    for (int i=0; i<max; i++) n[i]=(i<=s?v[i]:0);
    return n;
  }

  hid_t identifyH5Type( const identify_data_type& dataType, const string& v) const {
    switch (dataType) {
    case identify_byte_t:   return H5T_NATIVE_UCHAR; 
    case identify_char_t:   return H5T_C_S1;	     
    case identify_string_t: return H5T_C_S1;	     
    case identify_short_t:  return H5T_NATIVE_SHORT; 
    case identify_int_t:    return H5T_NATIVE_INT;   
    case identify_long_t:   return H5T_NATIVE_LONG;  
    case identify_float_t:  return H5T_NATIVE_FLOAT; 
    case identify_double_t: return H5T_NATIVE_DOUBLE;
    case identify_unknown_t:
    default:                
      cerr << "Unknown type for identifyH5Type with variable " << v << endl;      
      raise(SIGABRT);
      return -1;
    }
  }

  identify_data_type H5identifyType( const hid_t h5type, const string& v ) const {
    if (H5Tequal(h5type,H5T_NATIVE_UCHAR))  return identify_byte_t;
    if (H5Tequal(h5type,H5T_C_S1))	    return identify_char_t;
    if (H5Tequal(h5type,H5T_NATIVE_SHORT))  return identify_short_t;
    if (H5Tequal(h5type,H5T_NATIVE_INT))    return identify_int_t;
    if (H5Tequal(h5type,H5T_NATIVE_LONG))   return identify_long_t;
    if (H5Tequal(h5type,H5T_NATIVE_FLOAT))  return identify_float_t;
    if (H5Tequal(h5type,H5T_NATIVE_DOUBLE)) return identify_double_t;
    cerr << "Unknown type for H5identifyType with variable " << v << endl;
    raise(SIGABRT);
    return identify_unknown_t;
  }

  void errorCheck(const char const *file, const int &lineNumber, const char const *func, const char const *line, const int &status);

  hid_t createGroup(const string &groupName);

  virtual bool open(const string &filename, const hid_t &accessMode );
  hid_t fileId, classId, majorErrorId, minorErrorId;

  void pushError(const string &e, const char const *file, const int &lineNumber, const char const *func);

#else
  int fileId;
#endif

};

#endif
