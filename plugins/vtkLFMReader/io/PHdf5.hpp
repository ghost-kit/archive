#ifndef PHDF5_HPP__
#define PHDF5_HPP__

#include "Hdf5.hpp"

class PHdf5 : public Hdf5 {

 public:

  PHdf5(int superDomainSize);

  bool readVariable( const std::string& variableName, 
		     const std::string& group,
		     const array_info_t& info,
		     void* data ) const;
  
  bool writeVariable( const std::string& variableName, 
		      const std::string& group,
		      const array_info_t& info,
		      const void* data );

  void getBcastArrayInfo( const std::string& group,
			  array_info_t& info ) const;
  
  void putArrayInfo( const std::string& group,
		     const array_info_t& info );

  bool verifyShape( const std::string& variableName,
		    const std::string& group,
		    const array_info_t& info ) const;

 protected:

#ifdef HAS_PHDF5
  bool open(const std::string& filename, const hid_t& accessMode );

  MPI_Comm comm;
  void setupComm();

  bool collectiveRead, collectiveWrite;
#endif
};

#endif
