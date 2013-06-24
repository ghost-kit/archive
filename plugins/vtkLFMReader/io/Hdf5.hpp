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

/// Compare with bool Hdf5::errorCheck(...)
#define ERRORCHECK(STATUS) errorCheck(STATUS, __FILE__, __LINE__, __FUNCTION__)
/// Compare with void Hdf5::pushError(...)
#define PUSHERROR(E) pushError(E, __FILE__, __LINE__, __FUNCTION__)

/**
 * \todo { Some Hdf5 operations require closing hid_t objects.  These
 * are not always obvious.  For example:
 *  - H5Dget_type(...)  
 *  - H5Aget_type(...)
 * Modify code so we close access where required.  See Hdf5
 * documentation for more info. }
 */
class Hdf5 : public Io {

 public:

  Hdf5(int superDomainSize);
  ~Hdf5();

  bool isEnabled() const;

  bool openRead(const std::string &filename);
  bool openWrite(const std::string &filename);

  array_info_t getArrayInfo(const std::string& variableName, 
			    const std::string& group) const;

  bool readVariable( const std::string& variableName, 
		     const std::string& group,
		     const array_info_t& info,
		     void* data ) const;

  bool readAttribute( const std::string& attributeName,
		      void* data,
		      int& dataLength, 
		      const identify_data_type& dataType,
		      const std::string& group) const;
  
  bool writeVariable( const std::string& variableName, 
		      const std::string& group,
		      const array_info_t& info,
		      const void* data );

  bool writeAttribute( const std::string& attributeName,
		       const void* data,
		       const int& dataLength ,
		       const identify_data_type& dataType,
		       const std::string& group);

  void getBcastArrayInfo( const std::string& group,
			  array_info_t& info ) const;
  
  void getLocalArrayInfo( const std::string& group,
			  array_info_t& info ) const;
  
  void putArrayInfo( const std::string& group,
		     const array_info_t& info );


  bool verifyShape( const std::string& variableName,
		    const std::string& group,
		    const array_info_t& info ) const;
  

  const std::list<std::string> getVariableNames() const;
  const std::list<std::string> getAttributeNames() const;

  bool close();

 protected:
  
#ifdef HAS_HDF5

  std::map<std::string,hid_t> h5groups;

  template<class T> hsize_t* hsize_convert(const T* v, const int &s, hsize_t *n, 
					   const int &max=MAX_ARRAY_DIMENSION) const {
    for (int i=0; i<max; i++) n[i]=(i<=s?v[i]:0);
    return n;
  }

  hid_t identifyH5Type( const identify_data_type& dataType, const std::string& v) const {
    switch (dataType) {
    case identify_byte_t:   return H5T_NATIVE_UCHAR;
    case identify_char_t:   return H5T_NATIVE_CHAR;
    case identify_string_t: return H5T_C_S1;
    case identify_short_t:  return H5T_NATIVE_SHORT;
    case identify_int_t:    return H5T_NATIVE_INT;
    case identify_long_t:   return H5T_NATIVE_LONG;
    case identify_float_t:  return H5T_NATIVE_FLOAT;
    case identify_double_t: return H5T_NATIVE_DOUBLE;
    case identify_unknown_t:
    default:                
      //cerr << "Unknown type for identifyH5Type with variable " << v << endl;
      //raise(SIGABRT);
      return -1;
    }
  }

  /**
   * Find a list of native data types here:
   *   http://www.hdfgroup.org/HDF5/doc/RM/RM_H5T.html#Datatype-GetNativeType
   */
  identify_data_type H5identifyType( const hid_t h5type, const std::string& v ) const {

    if (H5Tequal(h5type, H5T_C_S1))   return identify_string_t;

    hid_t native_type = H5Tget_native_type(h5type, H5T_DIR_ASCEND);

    if (H5Tequal(native_type,H5T_NATIVE_UCHAR))  return identify_byte_t;
    if (H5Tequal(native_type,H5T_NATIVE_SHORT))  return identify_short_t;
    if (H5Tequal(native_type,H5T_NATIVE_INT))    return identify_int_t;
    if (H5Tequal(native_type,H5T_NATIVE_LONG))   return identify_long_t;
    if (H5Tequal(native_type,H5T_NATIVE_FLOAT))  return identify_float_t;
    if (H5Tequal(native_type,H5T_NATIVE_DOUBLE)) return identify_double_t;
    //cerr << "Unknown type for H5identifyType with variable " << v << endl;
    //raise(SIGABRT);
    return identify_unknown_t;
  }

  /**
   * \brief Check for Hdf5 errors.  If a problem is found, push error message(s) to errorStack.
   *
   * \note Use ERRORCHECK preprocessor macro to help set Hdf5::errorCheck arguments!
   * 
   * \param status hdf5 error status flag (should be < 0 denotes error)
   * \param file Name of source file containing the error
   * \param line Line number where error occured
   * \param func Name of function which error occured within.
   *
   * \return true if an error was found.
   */
  bool errorCheck(const int &status, const char *file, const int &line, const char *func) const;

  hid_t createGroup(const std::string &groupName);

  virtual bool open(const std::string &filename, const hid_t &accessMode );
  hid_t fileId, classId, majorErrorId, minorErrorId;

  void pushError(const std::string &e, const char *file, const int &line, const char *func) const;

#else
  int fileId;
#endif

};

#endif
