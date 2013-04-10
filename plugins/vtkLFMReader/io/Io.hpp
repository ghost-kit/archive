#ifndef IO_HPP__
#define IO_HPP__

#ifndef NOMPI
#include <mpi.h>
#else
#include "null-mpi/mpi.h"
#endif//NOMPI

#ifndef NOPP
#include <A++.h>
#endif//NOPP

using namespace std;

#include <string>
#include <vector>
#include <list>
#include "Util.hpp"

#ifndef MAX_ARRAY_DIMENSION
#define MAX_ARRAY_DIMENSION 6
#endif

struct array_info_t {
  int nDims, nVars, nAttr, bytes,
    globalDims[MAX_ARRAY_DIMENSION], 
    localDims[MAX_ARRAY_DIMENSION], 
    offset[MAX_ARRAY_DIMENSION], 
    base[MAX_ARRAY_DIMENSION];
  identify_data_type dataType;
};

static void printArrayInfo( array_info_t& info ) {
  cout << "   Dims: "  << info.nDims 
       << "   nVars: " << info.nVars 
       << "   nAttr: " << info.nAttr 
       << "   bytes: " << info.bytes 
       << "   type: "  << info.dataType << endl;
  
  for (int i=0; i<info.nDims; i++) {
    cout << i << ")  " 
	 << info.globalDims[i] << " : " 
	 << info.localDims[i] << " : "
	 << info.offset[i] << " : "
	 << info.base[i] << endl;
  }
  
}

class Io {

public:

  Io(int superDomainSize = -1);

  virtual
  ~Io() {};
 
  virtual
  bool enabled() = 0;

  virtual
  bool openRead(const string& filename) = 0;

  virtual
  bool openWrite(const string& filename) = 0;

  static Io* extSelector(const string& ext, 
			 const int& superDomainSize=-1);

  static Io* fileComplete(const string& fileName, 
			  const string& dir=".", 
			  const int& superDomainSize=-1);

  template<class T>
  void writeAttribute(const T& data,
		      const string& variable,
		      const string& group="",
		      const int& len=1);

  template<class T>
  int readAttribute(T& data,
		    const string& variable,
		    const string& group="",
		    const int& len=1);

  template<class T>
  void writeAttribute0(const T& data,
		       const string& variable,
		       const string& group="",
		       const int& len=1);
  
  template<class T>
  int readAttribute0(T& data,
		     const string& variable,
		     const string& group="",
		     const int& len=1);

  virtual
  void writeVariable( const string& variable,
		      const string& group,
		      const array_info_t& info,
		      const void* data ) = 0;

  virtual
  void writeAttribute( const string& variable,
		       const string& group,
		       const identify_data_type& dataType,
		       const void* data,
		       const int& len=1 ) = 0;

  virtual
  bool readVariable( const string& variable,
		     const string& group,
		     const array_info_t& info,
		     void* data ) = 0;

  virtual
  int readAttribute( const string& variable,
		     const string& group,
		     const identify_data_type& dataType,
		     void* data,
		     const int& len=1 ) = 0;

  virtual
  void getBcastArrayInfo( const string& group,
			  array_info_t& info ) = 0;

  virtual
  void getLocalArrayInfo( const string& group,
			  array_info_t& info ) = 0;

  virtual
  void putArrayInfo( const string& group,
		     const array_info_t& info ) = 0;

  virtual
  bool verifyShape( const string& variable,
		    const string& group,
		    const array_info_t& info ) = 0;

  virtual
  const list<string> getVarNames() = 0;

  virtual
  void close() = 0;

  string extName() 
  { return ext; }

  int getSuperSize() 
  { return superSize; }

#ifndef NOPP
  template<class PppArray> 
  array_info_t& fillInfo(const PppArray& data, 
			 array_info_t& info, 
			 const int multiVarDims=0);

  template<class PppArray> 
  void writeVariable(const PppArray& data,
		     const string& variable,
		     const string& group="",
		     const int multiVarDims=0);

  template<class PppArray>
  void writeVarUnits(const PppArray& data,
		     const string& variable,		     
		     const string &units="not set",
		     const string& group="");

  template<class PppArray>
  void writeMultiVar(const PppArray& data,
		     const string& variable,
		     const string& group="",
		     const int multiVarDims=4);

  template<class PppArray>
  void writeMultiVarUnits(const PppArray& data,
			  const string& variable,
			  const string &units="not set",
			  const string& group="",
			  const int multiVarDims=4);

  template<class PppArray>
  void readVariable(PppArray& data,
		    const string& variable,
		    const string& group="",
		    const int multiVarDims=0);

  template<class PppArray>
  void readVarUnits(PppArray& data,
		    const string& variable,
		    string& units,
		    const string& group="");

  template<class PppArray>
  void readMultiVar(PppArray& data,
		    const string& variable,
		    const string& group="",
		    const int multiVarDims=4);

  template<class PppArray>
  void readMultiVarUnits(PppArray& data,
			 const string& variable,
			 string& units,
			 const string& group="",
			 const int multiVarDims=4);

  template<class PppArray>
  void writeShape(const PppArray& data,
		  const string& group="");

  template<class PppArray>
  PppArray& readShape(PppArray& data,
		      const string& group="",
		      const bool& conformityCheck=true);
#endif//NOPP

protected:

  string ext;
  
  int nProcs, rank, superSize;
  
  bool c_order;

#ifndef NOPP  
  Partitioning_Type /*partitionAll,*/ partitionSuper;
#endif//NOPP
};

#include "IoTemplates.hpp"

#ifndef IO_BASE_DEFINITION__

#include "Hdf.hpp"
#include "Hdf4.hpp"
#include "Hdf5.hpp"
#include "PHdf5.hpp"

#endif //IO_BASE_DEFINITION__

#endif //IO_HPP__
