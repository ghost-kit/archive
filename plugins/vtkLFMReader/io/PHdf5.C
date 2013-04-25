#include "PHdf5.hpp"
#include <sstream>

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

bool PHdf5::readVariable( const string& variable, 
			  const string& group,
			  const array_info_t& info,
			  void* data ) 
{
#ifdef HAS_PHDF5
  hsize_t ones[MAX_ARRAY_DIMENSION], dims[MAX_ARRAY_DIMENSION], offset[MAX_ARRAY_DIMENSION];
  for (int i=0; i<MAX_ARRAY_DIMENSION; i++) ones[i] = 1;

  if (!verifyShape(variable,group,info)) return false;

  if (rank < superSize) {
    hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
    ERRORCHECK(groupId);
    hid_t dataId = H5Dopen(groupId, variable.c_str(), H5P_DEFAULT);  
    ERRORCHECK(dataId);

    hid_t spaceId = H5Dget_space(dataId);
    ERRORCHECK(spaceId);

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

    ERRORCHECK( H5Dread(dataId, identifyH5Type(info.dataType,variable), memSpace, spaceId, plistId, data) );
    
    H5Sclose(memSpace);
    H5Sclose(spaceId);
    H5Dclose(dataId);
    if (group!="") ERRORCHECK(H5Gclose(groupId));

    if (collectiveRead)
      H5Pclose(plistId);
    
  }
  return true;
#else
  return false;
#endif
}
  
/*----------------------------------------------------------------------------*/

void PHdf5::writeVariable( const string& variable, 
			   const string& group,
			   const array_info_t& info,
			   const void* data ) 
{
#ifdef HAS_PHDF5
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

    hid_t dataSet = H5Dcreate(createGroup(group), variable.c_str(), identifyH5Type(info.dataType,variable),
			      fileSpace, H5P_DEFAULT, dataId, H5P_DEFAULT);
    ERRORCHECK(dataSet);

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
    
    ERRORCHECK( H5Dwrite(dataSet, identifyH5Type(info.dataType,variable), memSpace, fileSpace, plistId, data) );
    
    if (collectiveWrite)
      H5Pclose(plistId);
    
    H5Sclose(memSpace);
    H5Dclose(dataSet);
    H5Pclose(dataId);
    H5Sclose(fileSpace);
    putArrayInfo((group==""?variable:group+"/"+variable),info);
  }
#endif  
}

/*----------------------------------------------------------------------------*/

void PHdf5::getBcastArrayInfo( const string& group,
			 array_info_t& info  ) 
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

bool PHdf5::verifyShape( const string& variable,
			 const string& group,
			 const array_info_t& info ) 
{
#ifdef HAS_PHDF5
  int error = 0, errorAll = 0;
  hsize_t nPoints = 0;
  hsize_t dims[MAX_ARRAY_DIMENSION], maxDims[MAX_ARRAY_DIMENSION];

  if (rank < superSize){
    if (group=="" || H5Lexists(fileId,group.c_str(),H5P_DEFAULT)) {
      hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
      ERRORCHECK(groupId);
      hid_t dataId = H5Dopen(groupId,variable.c_str(),H5P_DEFAULT);
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
    cout << "Var: " << variable << " group: " << group << " rank: " << rank 
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
  fileId = -1;

  if (superSize == -1) {
    if (accessMode ==  H5F_ACC_RDONLY) {
      if (rank == 0) {      
	superSize = 1;
	fileId = H5Fopen(filename.c_str(), accessMode, H5P_DEFAULT);
	Io::readAttribute("superSize", superSize);
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
      ERRORCHECK( fileId );

    } else if (accessMode == H5F_ACC_TRUNC){

      hid_t plistId = H5Pcreate(H5P_FILE_ACCESS);
      ERRORCHECK( plistId );
#ifndef NOH5MPIO
      ERRORCHECK( H5Pset_fapl_mpio(plistId, comm, MPI_INFO_NULL) );
#endif
      fileId = H5Fcreate(filename.c_str(), accessMode, H5P_DEFAULT, plistId);
      ERRORCHECK( fileId );
      H5Pclose(plistId);
      Io::writeAttribute("superSize",superSize,1);
      Io::writeAttribute("rank",rank,1);
    } else {
      cerr << __FILE__ << " (" << __LINE__ << "): " << __FUNCTION__ 
	   << "Did not understand file access mode" << endl;
    }
  }
  return true;
}
#endif //HAS_PHDF5

