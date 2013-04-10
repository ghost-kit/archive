#include "Hdf.hpp"

/*----------------------------------------------------------------------------*/

Hdf::Hdf(int superDomainSize) : Hdf4(1)
{
  ext = "hdf";
}

/*----------------------------------------------------------------------------*/

Hdf::Hdf(int superDomainSize, string some_ext) : Hdf4(1)
{
  ext = some_ext;
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

  if (rank == 0) {
    int32 dims[MAX_ARRAY_DIMENSION], count=0;
    memset(&dims,0,sizeof(int32)*MAX_ARRAY_DIMENSION);
    info.nDims = 0;

    while (info.nDims<MAX_ARRAY_DIMENSION && count!=-1) {
      name[1] = 'i'+info.nDims;
      count = Io::readAttribute(dims[info.nDims++],name,group);
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
#endif
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
  string file = filename + "." + ext;
  sdId = -1;
  if (rank < superSize) {
    sdId = SDstart(file.c_str(), accessMode);            
    if (sdId<0)
      {
	cout << "Cannot open " << file.c_str() << " with mode " << 
	  (accessMode==DFACC_RDONLY ? "read only" : "create") << "!" << endl;
      }
    ERRORCHECK(sdId);
  }
  return true;
}
#endif
