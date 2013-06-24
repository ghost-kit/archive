#ifndef HDF_HPP__
#define HDF_HPP__

#include "Hdf4.hpp"

class Hdf : public Hdf4 {

 public:

  Hdf(int superDomainSize);
  Hdf(int superDomainSize, const string& extension);

  bool openRead(const string& filename);
  bool openWrite(const string& filename);

  void getBcastArrayInfo( const string& group,
			  array_info_t& info ) const;
  
  void putArrayInfo( const string& group,
		     const array_info_t& info );

 protected:

#ifdef HAS_HDF4
  bool open(const string& filename, const int32& accessMode);
#endif

};

#endif
