#include "PHdf5.hpp"
#include <sstream>

using namespace std;

/*----------------------------------------------------------------------------*/

PHdf5::PHdf5(int superDomainSize) : 
#ifndef NOH5MPIO
  Hdf5(superDomainSize)
#else
  Hdf5(1)
#endif  
{
  extension = "phdf5";

#ifdef NOH5MPIO
  if (rank==0) 
    cerr << "WARNING: Hdf5 MPIO disabled, defaulting to superSize " << superSize << " for PHdf5." << endl;
  superDomainSize = 1;
#endif

#ifdef HAS_PHDF5
  if (superDomainSize != -1) setupComm();
  collectiveRead = collectiveWrite = false;
#endif
}

/*----------------------------------------------------------------------------*/

#ifdef HAS_PHDF5
void PHdf5::setupComm() 
{
  MPI_Group  worldGroup, commGroup;
  int range[1][3];
  range[0][0]=0;
  range[0][1]=superSize-1;
  range[0][2]=1;

  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
  MPI_Group_range_incl(worldGroup, 1, range, &commGroup);
  MPI_Comm_create(MPI_COMM_WORLD, commGroup, &comm);
  MPI_Group_free(&worldGroup);
  MPI_Group_free(&commGroup);
}
#endif  

/*----------------------------------------------------------------------------*/

bool PHdf5::readVariable( const string& variableName, 
			  const string& group,
			  const array_info_t& info,
			  void* data ) const
{
#ifdef HAS_PHDF5
  bool hasError = false;

  hsize_t ones[MAX_ARRAY_DIMENSION], dims[MAX_ARRAY_DIMENSION], offset[MAX_ARRAY_DIMENSION];
  for (int i=0; i<MAX_ARRAY_DIMENSION; i++) ones[i] = 1;

  if (!verifyShape(variableName,group,info)) {
    errorQueue.pushError("could not verify array shape");    
    return false;
  }

  if (rank < superSize) {
    hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
    if( ERRORCHECK(groupId) )
      hasError = true;
    hid_t dataId = H5Dopen(groupId, variableName.c_str(), H5P_DEFAULT);  
    if( ERRORCHECK(dataId) )
      hasError = true;

    hid_t spaceId = H5Dget_space(dataId);
    if( ERRORCHECK(spaceId) )
      hasError = true;

    H5Sselect_hyperslab(spaceId, H5S_SELECT_SET, hsize_convert(info.offset,info.nDims,offset), 
			ones, ones, hsize_convert(info.localDims,info.nDims,dims));
    hid_t memSpace = H5Screate_simple(info.nDims, hsize_convert(info.localDims,info.nDims,dims), NULL);

    hid_t plistId;
    if (collectiveRead) {
      plistId = H5Pcreate(H5P_DATASET_XFER);
#ifndef NOH5MPIO
      H5Pset_dxpl_mpio(plistId, H5FD_MPIO_COLLECTIVE);
#endif
    } else
      plistId = H5P_DEFAULT;

    if( ERRORCHECK( H5Dread(dataId, identifyH5Type(info.dataType,variableName), memSpace, spaceId, plistId, data) ) )
      hasError = true;
    
    H5Sclose(memSpace);
    H5Sclose(spaceId);
    H5Dclose(dataId);
    if (group!="") {
      if( ERRORCHECK(H5Gclose(groupId)) )
	hasError = true;
    }

    if (collectiveRead)
      H5Pclose(plistId);
    
  }
  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariableName=" << variableName << endl
       << "\tgroup=" << group << endl;
    
    errorQueue.pushError(ss);
    //errorQueue.print(cerr);
  }
  return (not hasError);

#else
  errorQueue.pushError("HAS_PHDF5 is undefined");
  return false;
#endif
}
  
/*----------------------------------------------------------------------------*/

bool PHdf5::writeVariable( const string& variableName, 
			   const string& group,
			   const array_info_t& info,
			   const void* data ) 
{
#ifdef HAS_PHDF5
  bool hasError = false;
  hsize_t ones[MAX_ARRAY_DIMENSION], localDims[MAX_ARRAY_DIMENSION], 
    offset[MAX_ARRAY_DIMENSION], globalDims[MAX_ARRAY_DIMENSION];
  for (int i=0; i<MAX_ARRAY_DIMENSION; i++) ones[i] = 1;
  
  if (rank < superSize) {
    hsize_convert(info.globalDims,info.nDims,globalDims);
    hsize_convert(info.localDims,info.nDims,localDims);
    hsize_convert(info.offset,info.nDims,offset);

    hid_t fileSpace = H5Screate_simple(info.nDims, globalDims, NULL);

    hid_t dataId = H5Pcreate(H5P_DATASET_CREATE);
    //H5Pset_chunk(dataId, info.nDims, localDims);

    hid_t dataSet = H5Dcreate(createGroup(group), variableName.c_str(), identifyH5Type(info.dataType,variableName),
			      fileSpace, H5P_DEFAULT, dataId, H5P_DEFAULT);
    if( ERRORCHECK(dataSet) )
      hasError = true;

    H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, offset, ones, ones, localDims);
    
    hid_t memSpace = H5Screate_simple(info.nDims, localDims, NULL);

    hid_t plistId;
    if (collectiveWrite) {
      plistId = H5Pcreate(H5P_DATASET_XFER);
#ifndef NOH5MPIO
      H5Pset_dxpl_mpio(plistId, H5FD_MPIO_COLLECTIVE);
#endif
    } else
      plistId = H5P_DEFAULT;
    
    if( ERRORCHECK( H5Dwrite(dataSet, identifyH5Type(info.dataType,variableName), memSpace, fileSpace, plistId, data) ) )
      hasError = true;
    
    if (collectiveWrite)
      H5Pclose(plistId);
    
    H5Sclose(memSpace);
    H5Dclose(dataSet);
    H5Pclose(dataId);
    H5Sclose(fileSpace);
    putArrayInfo((group==""?variableName:group+"/"+variableName),info);
  }
  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariableName=" << variableName << endl
       << "\tgroup=" << group << endl;
    
    errorQueue.pushError(ss);
    //errorQueue.print(cerr);
  }
  return (not hasError);

#else
  errorQueue.pushError("HAS_HDF5 is undefined");
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

void PHdf5::getBcastArrayInfo( const string& group,
			 array_info_t& info ) const
{
#ifdef HAS_PHDF5
  if (rank == 0) {
    readAttribute("globalDims", info.globalDims, info.nDims, identify(info.globalDims[0]), group);
    readAttribute("base",info.base);
  }

  MPI_Bcast(&info.nDims, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (info.nDims) {
    MPI_Bcast(info.globalDims, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(info.base, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
  }
#endif
}

/*----------------------------------------------------------------------------*/

void PHdf5::putArrayInfo( const string& group,
			  const array_info_t& info ) 
{
#ifdef HAS_PHDF5
  writeAttribute("globalDims",info.globalDims,info.nDims,identify(info.globalDims[0]),group);
  writeAttribute("base",info.base,info.nDims,identify(info.base[0]),group);
#endif
}

/*----------------------------------------------------------------------------*/

bool PHdf5::verifyShape( const string& variableName,
			 const string& group,
			 const array_info_t& info ) const
{
#ifdef HAS_PHDF5
  int error = 0, errorAll = 0;
  hsize_t nPoints = 0;
  hsize_t dims[MAX_ARRAY_DIMENSION], maxDims[MAX_ARRAY_DIMENSION];

  if (rank < superSize){
    if (group=="" || H5Lexists(fileId,group.c_str(),H5P_DEFAULT)) {
      hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
      ERRORCHECK(groupId);
      hid_t dataId = H5Dopen(groupId,variableName.c_str(),H5P_DEFAULT);
      ERRORCHECK(dataId);
      hid_t spaceId = H5Dget_space(dataId);
      ERRORCHECK(spaceId);
      nPoints = H5Sget_simple_extent_ndims(spaceId);
      if (nPoints == info.nDims) {
	ERRORCHECK(H5Sget_simple_extent_dims(spaceId,dims,maxDims));
	for (int i=0; i<nPoints && !error; i++) {
	  if (dims[i] != info.globalDims[i]) error = 1;
	}
      } else error = 1;
      ERRORCHECK(H5Sclose(spaceId));      
      ERRORCHECK(H5Dclose(dataId));
      if (group!="") ERRORCHECK(H5Gclose(groupId));
    }
  }

  if (error) {
    cout << "Var: " << variableName << " group: " << group << " rank: " << rank 
	 << " npoints: " << nPoints << " error: " << error << endl;
    if (rank<superSize) {    
      cout << " Dims for rank " << rank << endl;
      for (int i=0; i<nPoints; i++) {
	cout << "Dim " << i << ": expect " << dims[i] << " have " << info.localDims[i] << endl;
      }    
    }
  }

  MPI_Allreduce(&error, &errorAll, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
  return !errorAll;
#else
  return false;
#endif
}


/*----------------------------------------------------------------------------*/

#ifdef HAS_PHDF5
bool PHdf5::open(const string& filename, const hid_t& accessMode)
{
  bool hasError = false;
  fileId = -1;

  if (superSize == -1) {
    if (accessMode ==  H5F_ACC_RDONLY) {
      if (rank == 0) {      
	superSize = 1;
	fileId = H5Fopen(filename.c_str(), accessMode, H5P_DEFAULT);
	if( not Io::readAttribute("superSize", superSize) )
	  hasError = true;
      }
      MPI_Bcast(&superSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
      superSize = nProcs;
    }
#ifdef BUILD_WITH_APP
    if (superSize>0) partitionSuper = Partitioning_Type(superSize);
#endif//BUILD_WITH_APP
    setupComm();
  }

  if (rank < superSize) {

    if (accessMode ==  H5F_ACC_RDONLY) {

      if (fileId == -1) fileId = H5Fopen(filename.c_str(), accessMode, H5P_DEFAULT);
      if( ERRORCHECK( fileId ) )
	hasError = true;
      
    } else if (accessMode == H5F_ACC_TRUNC){

      hid_t plistId = H5Pcreate(H5P_FILE_ACCESS);
      if( ERRORCHECK( plistId ) )
	hasError = true;
#ifndef NOH5MPIO
      if( ERRORCHECK( H5Pset_fapl_mpio(plistId, comm, MPI_INFO_NULL) ) )
	hasError = true;
#endif
      fileId = H5Fcreate(filename.c_str(), accessMode, H5P_DEFAULT, plistId);
      if( ERRORCHECK( fileId ) )
	hasError = true;
      H5Pclose(plistId);
      if(not Io::writeAttribute("superSize",superSize,1) )
	hasError = true;
      if(not Io::writeAttribute("rank",rank,1) )
	hasError = true;
    } else {
      hasError = true;
      stringstream ss;
      ss << __FILE__ << " (" << __LINE__ << "): " << __FUNCTION__ 
	 << "Did not understand file access mode" << endl;
      errorQueue.pushError(ss);
    }
  }

  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << "failed.  Arguments:" << endl
       << "\tfilename=" << file << endl
       << "\taccessMode=" << accessMode << endl;
    errorQueue.pushError(ss);
  }
  return not hasError;
}
#endif //HAS_PHDF5

