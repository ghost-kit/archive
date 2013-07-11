#ifndef HDF4_HPP__
#define HDF4_HPP__

#define IO_BASE_DEFINITION__
#include "Io.hpp"

#ifdef HAS_HDF4
#include <mfhdf.h>
#endif

#include <map>
#include <csignal>

/// Compare with bool Hdf4::errorCheck(...)
#define ERRORCHECK(STATUS) errorCheck(STATUS, __FILE__, __LINE__, __FUNCTION__)

class Hdf4 : public Io {

 public:

  Hdf4(int superDomainSize);
  ~Hdf4();

  bool isEnabled() const;

  bool openRead(const std::string& filename);
  bool openWrite(const std::string& filename);

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
		       const int& dataLength,
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

#ifdef HAS_HDF4

  mutable std::map<std::string,int32> h4groups;
  
  template<class T> int32* int32_convert(const T* v, const int& s, int32* n, 
					 const int& max=MAX_ARRAY_DIMENSION) const {
    for (int i=0; i<max; i++) n[i]=(i<s?v[i]:0);
    return n;
  }

  int32 identifyH4Type( const identify_data_type& dataType, const std::string& v ) const {
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
      std::cerr << "Unknown type for identifyH4Type with variable " << v << std::endl;
      raise(SIGABRT);
      return -1;
    }
  }

  identify_data_type H4identifyType( const int32 h4type, const std::string& v ) const {
    switch (h4type) {
    case DFNT_UCHAR8:   return identify_byte_t;
    case DFNT_CHAR8:    return identify_char_t;
    case DFNT_INT16:    return identify_short_t;
    case DFNT_INT32:    return identify_int_t;
    case DFNT_INT64:    return identify_long_t;
    case DFNT_FLOAT32:  return identify_float_t;
    case DFNT_FLOAT64:  return identify_double_t;
    default:                
      std::cerr << "Unknown type for H4identifyType with variable " << v << std::endl;
      raise(SIGABRT);
      return identify_unknown_t;
    }
  }

  int32 openGroup(const std::string& groupName) const;

  int32 createGroup(const std::string& groupName);

  /**
   * \brief Check for Hdf4 errors.  If a problem is found, push error message(s) to errorStack.
   *
   * \note Use ERRORCHECK preprocessor macro to help set Hdf4::errorCheck arguments!
   * 
   * \param status hdf4 error status flag (should be < 0 denotes error)
   * \param file Name of source file containing the error
   * \param line Line number where error occured
   * \param func Name of function which error occured within.
   *
   * \return true if an error was found.
   */
  bool errorCheck(const int& status, const char* file, const int& line, const char* func) const;
  
  bool open(const std::string& filename, const int32& accessMode);

  int32 sdId;

#else
  int sdId;
#endif //HAS_HDF4

private:
  /// Prevent copying
  Hdf4(const Hdf4 &copy_me);
  /// Prevent assignment
  Hdf4& operator=(const Hdf4& rhs );  
};

#endif //HDF4_HPP__
