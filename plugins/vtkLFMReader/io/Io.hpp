#ifndef IO_HPP__
#define IO_HPP__

#ifdef BUILD_WITH_MPI
#include <mpi.h>
#else
#include "null-mpi/mpi.h"
#endif//BUILD_WITH_MPI

#ifdef BUILD_WITH_APP
#include <A++.h>
#endif//BUILD_WITH_APP

#include <string>
#include <vector>
#include <list>

using namespace std;

#include "ErrorQueue.h"
#include "Util.hpp"

/**
 * Abstract base class for model Input and Output of model data & metadata.
 */
class Io {

public:

  /**
   * Data may be written in super domains for improved parallel performance.
   *
   * \param superDomainSize { the number of superdomains to split a
   *   file into.  <=1 will write a single file.  >1 will split the data
   *   into two or more subdomains. }
   *
   * \warning { Super Domains have not been extensively profiled.  It's
   *   unclear how big of a benefit this will provide.}
   *
   * \warning { What are the limits of superDomainSize?  I think
   * there's an error lurking in here if you set superDomainSize=0!
   * Tread with caution!"}
   */
  Io(const int& superDomainSize = -1);

  virtual
  ~Io() {};
 
  /**
   * \return true if output format is available
   */ 
  virtual
  bool isEnabled() const = 0;

  virtual
  bool openRead(const string& filename) = 0;

  virtual
  bool openWrite(const string& filename) = 0;

  /** \brief Instantiate apprprioate I/O implementation based on the extension
   * \return new Io object.
   * \warning This allocates new memory.  Don't forget to delete!
   */
  static Io* extensionSelector(const string& extension, 
			 const int& superDomainSize=-1);

  /** \brief Not sure what this does!
   * \todo Remove this?
   * \return new Io object.
   * \warning This allocates new memory.  Don't forget to delete!
   */
  static Io* fileComplete(const string& fileName, 
			  const string& dir=".", 
			  const int& superDomainSize=-1);

  virtual 
  const list<string> getAttributeNames() const = 0;
  
  /** \brief Methods to read attributes
   * Sets data & dataLength (number of elements read in).
   *  
   * \warning { Assumes data is already allocated to the correct
   * length.  You might want to double check that dataLength didn't
   * exceed the size which data was allocated! }
   *
   * \returns true if succeeded without error.
   */
  //@{
  template<class T>
  bool readAttribute(const string& attributeName,
		     T& data,
		     int& dataLength,
		     const string& group="") const;
  template<class T>
  bool readAttribute(const string& attributeName,
		     T& data,
		     const string& group="") const;
  template<class T>
  bool readAttribute0(const string& attributeName,
		      T& data,
		      int& dataLength,
		      const string& group="") const;
  virtual
  bool readAttribute(const string& attributeName,
		     void *data,
		     int& dataLength,
		     const identify_data_type& dataType,
		     const string& group="") const = 0;
  //@}

  /** \brief Methods to write attributes
   * \returns true if succeeded without error.
   */
  //@{
  template<class T>
  bool writeAttribute(const string& attributeName,
		      const T& data,
		      const int& dataLength,
		      const string& group="");
  template<class T>
  bool writeAttribute0(const string& attributeName,
		       const T& data,
		       const int& dataLength,
		       const string& group="");

  virtual
  bool writeAttribute( const string& attributeName,
		       const void* data,
		       const int& dataLength,
		       const identify_data_type& dataType,
		       const string& group) = 0;
  //@}

  virtual
  const list<string> getVariableNames() const = 0;

  
  /** \brief Read variable information (dimensions, rank, etc) for group/variable
   * \see Util.hpp for array_info_t definition
   * \returns{ array_info_t object
   *   - nDim
   *   - nVars: defaults to 1 if missing "nVars" attribute 
   *   - nAttr
   *   - bytes
   *   - globalDims: defaults to localDims if missing "globalDims" attribute
   *   - localDims
   *   - offset: defaults to [0,0,...,0] if missing "offset" attribute
   *   - base: defaults to [0,0,...,0] if missing "base" attribute
   *   - dataType
   *  }
   */
  virtual
  array_info_t getArrayInfo(const string &variableName, 
			    const string &group="" ) const = 0;

  /// Methods to read variables
  //@{
  virtual
  bool readVariable( const string& variableName,
		     const string& group,
		     const array_info_t& info,
		     void* data ) const = 0;
  //@}

  /// Methods to write variables
  //@{
  virtual
  bool writeVariable( const string& variableName,
		      const string& group,
		      const array_info_t& info,
		      const void* data ) = 0;
  //@}


  /** \brief Methods that are specific to the LFM.
   *  \todo Refactor and remove these!
   */
  //@{
  virtual
  void getBcastArrayInfo( const string& group,
			  array_info_t& info ) const = 0;

  virtual
  void getLocalArrayInfo( const string& group,
			  array_info_t& info ) const = 0;

  virtual
  void putArrayInfo( const string& group,
		     const array_info_t& info ) = 0;
  //@}

  /// Assert that "info" matches the group/variable stored in file.
  virtual
  bool verifyShape( const string& variableName,
		    const string& group,
		    const array_info_t& info ) const = 0;

  virtual
  bool close() = 0;

  /// \return file extension for the selected I/O format.
  string getExtension() 
  { return extension; }

  /** 
   * \todo { Choose a better name than "SuperSize".  Would you like those fries super sized? }
   */
  int getSuperSize() 
  { return superSize; }

#ifdef BUILD_WITH_APP
  /// Methods to read/write A++ or P++ arrays.
  //@{
  template<class PppArray> 
  array_info_t& fillInfo(const PppArray& data, 
			 array_info_t& info, 
			 const int multiVarDims=0);

  template<class PppArray> 
  bool writeVariable(const PppArray& data,
		     const string& variableName,
		     const string& group="",
		     const int multiVarDims=0);

  template<class PppArray>
  void writeVarUnits(const PppArray& data,
		     const string& variableName,		     
		     const string &units="not set",
		     const string& group="");

  template<class PppArray>
  void writeMultiVar(const PppArray& data,
		     const string& variableName,
		     const string& group="",
		     const int multiVarDims=4);

  template<class PppArray>
  void writeMultiVarUnits(const PppArray& data,
			  const string& variableName,
			  const string &units="not set",
			  const string& group="",
			  const int multiVarDims=4);

  template<class PppArray>
  bool readVariable(PppArray& data,
		    const string& variableName,
		    const string& group="",
		    const int multiVarDims=0) const;

  template<class PppArray>
  void readVarUnits(PppArray& data,
		    const string& variableName,
		    string& units,
		    const string& group="") const;

  template<class PppArray>
  void readMultiVar(PppArray& data,
		    const string& variableName,
		    const string& group="",
		    const int multiVarDims=4) const;

  template<class PppArray>
  void readMultiVarUnits(PppArray& data,
			 const string& variableName,
			 string& units,
			 const string& group="",
			 const int multiVarDims=4) const;

  template<class PppArray>
  void writeShape(const PppArray& data,
		  const string& group="");

  template<class PppArray>
  PppArray& readShape(PppArray& data,
		      const string& group="",
		      const bool& conformityCheck=true) const;

  Partitioning_Type /*partitionAll,*/ partitionSuper;

  //@}
#endif//BUILD_WITH_APP

protected:

  
  /**
   * \brief {Use the errorQueue to glean information about why an Io
   * operation failed.}
   *
   * \see ErrorQueue.h for usage.
   */
  mutable ErrorQueue errorQueue;
  
  /// Extension for the selected I/O format.
  string extension;
  
  int nProcs, rank, superSize;
  
  /**
   * How are arrays ordered?
   * true: C style row-major ordering
   * false: Fortran style column-major ordering.
   */
  bool isArrayCOrdered;
};

#include "IoTemplates.hpp"

#ifndef IO_BASE_DEFINITION__

#include "Hdf.hpp"
#include "Hdf4.hpp"
#include "Hdf5.hpp"
#include "PHdf5.hpp"

#endif //IO_BASE_DEFINITION__

#endif //IO_HPP__
