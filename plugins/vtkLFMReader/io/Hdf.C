#include "Hdf.hpp"

/*----------------------------------------------------------------------------*/

Hdf::Hdf(int superDomainSize) : Hdf4(1)
{
  extension = "hdf";
}

/*----------------------------------------------------------------------------*/

Hdf::Hdf(int superDomainSize, const string& extension) : Hdf4(1)
{
  this->extension = extension;
}

/*----------------------------------------------------------------------------*/

bool Hdf::openRead(const string& filename)
{
#ifdef HAS_HDF4
  return open(filename, DFACC_RDONLY);
#else
  if (rank==0)
    cerr << "HDF4 disabled, unable to open " << filename << " for reading." << endl;
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf::openWrite(const string& filename)
{
#ifdef HAS_HDF4
  return open(filename, DFACC_CREATE);
#else
  if (rank==0)
    cerr << "HDF4 disabled, unable to open " << filename << " for writing." << endl;
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

void Hdf::getBcastArrayInfo( const string& group,
			     array_info_t& info  ) 
{
#ifdef HAS_HDF4
  memset(&info,0,sizeof(info));
  char name[3] = { 'n', 0, 0 };
#ifdef BUILD_WITH_MPI
  if (rank == 0) {
    int32 dims[MAX_ARRAY_DIMENSION], count=0;
    memset(&dims,0,sizeof(int32)*MAX_ARRAY_DIMENSION);
    info.nDims = 0;

    while (info.nDims<MAX_ARRAY_DIMENSION && count!=-1) {
      name[1] = 'i'+info.nDims;
      Io::readAttribute(name, dims[info.nDims++], count, group);
    }
    info.nDims-=1;
    for (int i=0; i<info.nDims; i++) {
      info.globalDims[info.nDims-1-i] = dims[i];
      info.base[i] = 1;
    }
  }

  MPI_Bcast(&info.nDims, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (info.nDims) {
    MPI_Bcast(info.globalDims, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(info.base, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
  }
#endif // BUILD_WITH_MPI
#endif // HAS_HDF4
}



/*----------------------------------------------------------------------------*/

void Hdf::putArrayInfo( const string& group,
			const array_info_t& info ) 
{
#ifdef HAS_HDF4
  char name[3] = { 'n', 0, 0 };
  if (rank < superSize) {
    for (int i=0; i<info.nDims; i++) {
      name[1] = 'i'+i;
      Io::writeAttribute(info.globalDims[info.nDims-1-i],name,group);
    }
  }
#endif
}

/*----------------------------------------------------------------------------*/

#ifdef HAS_HDF4
bool Hdf::open(const string& filename, const int32& accessMode)
{
  sdId = -1;
  if (rank < superSize) {
    sdId = SDstart(filename.c_str(), accessMode);
    if (sdId<0)
      {
        cout << "Cannot open " << filename << " with mode " <<
	  (accessMode==DFACC_RDONLY ? "read only" : "create") << "!" << endl;
      }
    ERRORCHECK(sdId);
  }
  return true;
}
#endif
