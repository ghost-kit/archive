#ifndef HDF_HPP__
#define HDF_HPP__

#include "Hdf4.hpp"

class Hdf : public Hdf4 {

 public:

  Hdf(int superDomainSize);
  Hdf(int superDomainSize, const std::string& extension);

  bool openRead(const std::string& filename);
  bool openWrite(const std::string& filename);

  void getBcastArrayInfo( const std::string& group,
			  array_info_t& info ) const;
  
  void putArrayInfo( const std::string& group,
		     const array_info_t& info );

 protected:

#ifdef HAS_HDF4
  bool open(const std::string& filename, const int32& accessMode);
#endif

};

#endif
