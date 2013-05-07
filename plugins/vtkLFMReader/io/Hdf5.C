#include "Hdf5.hpp"

/*----------------------------------------------------------------------------*/

Hdf5::Hdf5(int superDomainSize) : Io(superDomainSize), fileId(-999)
{
  extension = "hdf5";
  //hid_t error_stack;
  //H5Eset_auto(error_stack, NULL, NULL);
#ifdef HAS_HDF5
  classId = H5Eregister_class("Hdf5", "Io", "0.1");
  majorErrorId = H5Ecreate_msg(classId, H5E_MAJOR, "Major error");
  minorErrorId = H5Ecreate_msg(classId, H5E_MINOR, "Minor error");
#endif
}

/*----------------------------------------------------------------------------*/

Hdf5::~Hdf5()
{
#ifdef HAS_HDF5
  close();
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf5::isEnabled()
{
#ifdef HAS_HDF5
  return true;
#else
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf5::openRead(const string& filename)
{  
#ifdef HAS_HDF5
  return open(filename, H5F_ACC_RDONLY);
#else
  if (rank==0)
    cerr << "HDF5 disabled, unable to open " << filename << " for reading." << endl;
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf5::openWrite(const string& filename)
{
#ifdef HAS_HDF5
  return open(filename, H5F_ACC_TRUNC);
#else
  if (rank==0)
    cerr << "HDF5 disabled, unable to open " << filename << " for writing." << endl;
  return false;
#endif
}

/*----------------------------------------------------------------------------*/
#ifdef HAS_HDF5

bool Hdf5::errorCheck(const int& status, const char* file, const int& line, const char* func)
{
  if (status < 0) {
    stringstream errorString;
    errorString << "*** Error in" << file << "(L " << line << "): " << func << "(...)" << endl;
    errorString << "HDF5 error code: " << status << endl;
    errorQueue.pushError(errorString);

    //todo: deprecate pushError?
    pushError("unknown error",file,line,func);
    
    return true;
  }
  return false;
}

/*----------------------------------------------------------------------------*/

void Hdf5::pushError(const string &e, const char *file, const int &line, const char *func)
{
  H5Epush(H5E_DEFAULT,file,func,line,classId,majorErrorId,minorErrorId,e.c_str());
  H5Eprint(H5E_DEFAULT, stderr);
}

#endif
/*----------------------------------------------------------------------------*/

bool Hdf5::readVariable( const string& variableName, 
			 const string& group,
			 const array_info_t& info,
			 void* data )
{   
#ifdef HAS_HDF5
  bool hasError = false;
  if (!verifyShape(variableName,group,info)){
    errorQueue.pushError("could not verify array shape");    
    return false;
  }
  if (rank < superSize) {
    hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
    if( ERRORCHECK(groupId) )
      hasError = true;
    hid_t datasetId = H5Dopen(groupId, variableName.c_str(), H5P_DEFAULT);
    if( ERRORCHECK(datasetId) )
      hasError = true;
    if( ERRORCHECK(H5Dread(datasetId, identifyH5Type(info.dataType,variableName), H5S_ALL, H5S_ALL, H5P_DEFAULT, data)) )
      hasError = true;
    if( ERRORCHECK(H5Dclose(datasetId)) )
      hasError = true;
    if (group!="") {
      if( ERRORCHECK(H5Oclose(groupId)) )
	hasError = true;
    }
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

bool Hdf5::readAttribute( const string& attributeName,
			  void* data,
			  int& dataLength,
			  const identify_data_type& dataType,
			  const string& group)
{
#ifdef HAS_HDF5
  bool hasError = false;

  hid_t h5type;
  if (rank < superSize){
    if (group=="" || H5Lexists(fileId,group.c_str(),H5P_DEFAULT)) {
      hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
      if( ERRORCHECK(groupId) )
	hasError = true;
      if (H5Aexists(groupId,attributeName.c_str())) {
	hid_t attributeId = H5Aopen(groupId,attributeName.c_str(),H5P_DEFAULT);      
	if( ERRORCHECK(attributeId) )
	  hasError = true;
	hid_t spaceId = H5Aget_space(attributeId);
	if( ERRORCHECK(spaceId) )
	  hasError = true;
	hsize_t nPoints = 0;	 
	if (dataType == identify_string_t) {
	  h5type = H5Aget_type(attributeId);
	  nPoints = H5Tget_size(h5type);
	} else {
	  h5type = identifyH5Type(dataType,attributeName);
	  nPoints = H5Sget_select_npoints(spaceId);
	}
	dataLength = int(nPoints);
	if( ERRORCHECK(H5Aread(attributeId,h5type,data)) )
	  hasError = true;
	if( ERRORCHECK(H5Aclose(attributeId)) )
	  hasError = true;
	if( ERRORCHECK(H5Sclose(spaceId)) )
	  hasError = true;
      } else  {
	hasError = true;
	errorQueue.pushError("Attribute " + attributeName + " does not exist");
      }
      if (group!="") {
	if( ERRORCHECK(H5Oclose(groupId)) )
	  hasError = true;
      }
    } else {
      hasError = true;
      errorQueue.pushError("Group " + group + " does not exist...");
    }
  }

  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tattributeName=" << attributeName << endl
       << "\tdataLength=" << dataLength << endl
       << "\tdata_type=" << dataType2String(dataType) << endl
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

bool Hdf5::writeVariable( const string& variableName, 
			  const string& group,
			  const array_info_t& info,
			  const void* data )
{
#ifdef HAS_HDF5
  bool hasError = false;

  if (rank < superSize){
    hsize_t localDims[MAX_ARRAY_DIMENSION];
    hid_t dataspaceId = H5Screate_simple(hsize_t(info.nDims), 
					 hsize_convert(info.localDims,info.nDims,localDims), NULL);
    if( ERRORCHECK(dataspaceId) )
      hasError = true;
    hid_t datasetId = H5Dcreate(createGroup(group), variableName.c_str(), 
				identifyH5Type(info.dataType,variableName), dataspaceId, 
				H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if( ERRORCHECK(datasetId) )
      hasError = true;
    if( ERRORCHECK(H5Dwrite(datasetId, identifyH5Type(info.dataType,variableName), 
			    H5S_ALL, H5S_ALL, H5P_DEFAULT, data)))
      hasError = true;
    if( ERRORCHECK(H5Dclose(datasetId)) )
      hasError = true;
    if( ERRORCHECK(H5Sclose(dataspaceId)) )
      hasError = true;
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

bool Hdf5::writeAttribute( const string& variableName,
			   const void* data,
			   const int& dataLength ,
			   const identify_data_type& dataType,
			   const string& group)
{
#ifdef HAS_HDF5
  bool hasError = false;

  if (rank < superSize){
    hsize_t hsize_len = hsize_t((dataLength>0?dataLength:1));
    hid_t h5type = identifyH5Type(dataType,variableName);
    if (dataType == identify_string_t) {
      h5type = H5Tcopy(h5type);
      H5Tset_size(h5type,hsize_len);
      hsize_len = 1;
    }
    hid_t dataspaceId = H5Screate_simple(hsize_t(1), &hsize_len, NULL);
    if( ERRORCHECK(dataspaceId) )
      hasError = true;
    hid_t attributeId = H5Acreate(createGroup(group), variableName.c_str(), h5type, dataspaceId, H5P_DEFAULT, H5P_DEFAULT);
    if( ERRORCHECK(attributeId) )
      hasError = true;
    if( ERRORCHECK(H5Awrite(attributeId, h5type, data)) )
      hasError = true;
    if( ERRORCHECK(H5Aclose(attributeId)) )
      hasError = true;
    if( ERRORCHECK(H5Sclose(dataspaceId)) )
      hasError = true;
  }
  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariableName=" << variableName << endl
       << "\tdataLength=" << dataLength << endl
       << "\tdata_type=" << dataType2String(dataType) << endl
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

void Hdf5::getBcastArrayInfo( const string& group,
			      array_info_t& info  ) {
#ifdef HAS_HDF5
#ifdef BUILD_WITH_MPI
  if (rank==0) {
    Io::readAttribute("globalDims",info.globalDims[0], info.nDims, group);
    Io::readAttribute("base",info.base[0],group);
  }
  
  MPI_Bcast(&info.nDims, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (info.nDims) {
    MPI_Bcast(info.globalDims, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(info.base, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
  }
#endif // BUILD_WITH_MPI
#endif // HAS_HDF5
}

/*----------------------------------------------------------------------------*/

void Hdf5::getLocalArrayInfo( const string& group,
			      array_info_t& info  ) {
#ifdef HAS_HDF5
  memset(&info,0,sizeof(info));
  if (rank<superSize) {
    Io::readAttribute("globalDims",info.globalDims[0], info.nDims, group);
    Io::readAttribute("offset",info.offset[0],group);
    Io::readAttribute("base",info.base[0],group);
    Io::readAttribute("nVars",info.nVars, group);
    hsize_t nPoints=0, dims[MAX_ARRAY_DIMENSION], maxDims[MAX_ARRAY_DIMENSION];
    hid_t dataId = H5Dopen(fileId,group.c_str(),H5P_DEFAULT);
    ERRORCHECK(dataId);
    hid_t spaceId = H5Dget_space(dataId);
    ERRORCHECK(spaceId);
    nPoints = H5Sget_simple_extent_ndims(spaceId);
    ERRORCHECK(H5Sget_simple_extent_dims(spaceId,dims,maxDims));
    H5O_info_t h5info;
    H5Oget_info(dataId,&h5info);
    info.nAttr = h5info.num_attrs;
    info.dataType = H5identifyType(H5Dget_type(dataId),group);
    ERRORCHECK(H5Sclose(spaceId));      
    ERRORCHECK(H5Dclose(dataId));
    if (info.nDims>0 && nPoints!=info.nDims) {
      cerr << rank << "] Dimensions do not match!" << nPoints << " : " << info.nDims << endl;
      exit(-1);
    }
    info.bytes = 1;
    for (int i=0; i<nPoints; i++) {
      info.localDims[i] = dims[i];
      info.bytes *= dims[i];
      if (info.nDims==-1) info.globalDims[i] = dims[i];
    }
    info.bytes *= identifySize(info.dataType);
    info.nDims = nPoints;
  }
#endif
}

/*----------------------------------------------------------------------------*/

void Hdf5::putArrayInfo( const string& group,
			 const array_info_t& info ) {
#ifdef HAS_HDF5
  //if (rank==0)
  Io::writeAttribute("base",info.base[0],info.nDims,group);
  Io::writeAttribute("globalDims",info.globalDims[0],info.nDims,group);
  Io::writeAttribute("offset",info.offset[0],info.nDims,group);
  //Io::writeAttribute("localDims",info.localDims[0],info.nDims,group);
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf5::verifyShape( const string& variableName,
			const string& group,
			const array_info_t& info ) {
#ifdef HAS_HDF5
  int error = 0;
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
	  if (dims[i] != info.localDims[i]) error = 1;
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

  return !error;
#else
  return false;
#endif
}
  
/*----------------------------------------------------------------------------*/

#ifdef HAS_HDF5
/** \brief Used by Hdf5::getVariableNames() to append variable names to list
 * \param op_data list<string>
 */
static herr_t appendVariableNameToList(hid_t id, const char *name, const H5O_info_t *info, void *op_data)
{
  if (info->type == H5O_TYPE_DATASET)
    reinterpret_cast<list<string> *>(op_data)->push_back(string(name));
  return(H5_ITER_CONT);
}
#endif

/*----------------------------------------------------------------------------*/

const list<string> Hdf5::getVariableNames()
{
  list<string> r;
#ifdef HAS_HDF5
  H5Ovisit(fileId,H5_INDEX_NAME,H5_ITER_NATIVE,appendVariableNameToList,&r);
#endif
  return r;
}

/*----------------------------------------------------------------------------*/

#ifdef HAS_HDF5
/** \brief Used by Hdf5::getAttributeNames() to append variable names to list
 * \param op_data list<string>
 */
static herr_t appendAttributeNameToList(hid_t id, const char *name, const H5A_info_t *ainfo, void *op_data)
{
  reinterpret_cast<list<string> *>(op_data)->push_back(string(name));
  return 0;
}
#endif

/*----------------------------------------------------------------------------*/

const list<string> Hdf5::getAttributeNames()
{
  list<string> attrNames;
  //herr_t H5Aiterate2( hid_t obj_id, H5_index_t idx_type, H5_iter_order_t order, hsize_t *n, H5A_operator2_t op, void *op_data, )
  H5Aiterate2( fileId, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, appendAttributeNameToList, &attrNames);

  return attrNames;
}

/*----------------------------------------------------------------------------*/
#ifdef HAS_HDF5

hid_t Hdf5::createGroup(const string& group)
{
  hid_t groupId = fileId;
  if (rank < superSize){
    if (group != "") {
      if (h5groups.find(group) != h5groups.end())
	groupId = h5groups[group];
      else {
	if (!H5Lexists(fileId,group.c_str(),H5P_DEFAULT)) {
	  groupId = H5Gcreate(fileId, group.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	} else {
	  groupId = H5Oopen(fileId, group.c_str(), H5P_DEFAULT);
	}
	h5groups[group] = groupId;
	ERRORCHECK(groupId);
      }
    }
  }
  return groupId;
}

/*----------------------------------------------------------------------------*/

bool Hdf5::open(const string& filename, const hid_t& accessMode)
{
  bool hasError = false;
  fileId = -1;

  if (superSize == -1) {
    if (accessMode ==  H5F_ACC_RDONLY) {
      if (rank == 0) {      
	superSize=1;
	fileId = H5Fopen(filename.c_str(), accessMode, H5P_DEFAULT);
	if( not Io::readAttribute("superSize", superSize) )
	  hasError = true;
      }
#ifdef BUILD_WITH_MPI
      MPI_Bcast(&superSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif //BUILD_WITH_MPI
    } else {
      superSize = nProcs;
    }
#ifdef BUILD_WITH_APP
    if (superSize>0) 
      partitionSuper = Partitioning_Type(superSize);
#endif//BUILD_WITH_APP
  }

  if (rank < superSize){
    if (accessMode == H5F_ACC_RDONLY) {
      if (fileId == -1) 
	fileId = H5Fopen(filename.c_str(), accessMode, H5P_DEFAULT);
    } else if (accessMode == H5F_ACC_TRUNC) {
      if (fileId == -1) 
	fileId = H5Fcreate(filename.c_str(), accessMode, H5P_DEFAULT, H5P_DEFAULT);
      if(not Io::writeAttribute("superSize",superSize,1) )
	hasError = true;
      if(not Io::writeAttribute("rank",rank,1) )
	hasError = true;
    } else {
      hasError = true;
    }
    if( ERRORCHECK(fileId) ){
      errorQueue.pushError("Did not understand accessMode");
      hasError = true;
    }
  }

  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << "failed.  Arguments:" << endl
       << "\tfilename=" << filename << endl
       << "\taccessMode=" << accessMode << endl;
    errorQueue.pushError(ss);
  }
  return not hasError;
}
#endif //HAS_HDF5

/*----------------------------------------------------------------------------*/

bool Hdf5::close()
{
#ifdef HAS_HDF5
  bool hasError = false;
  if (rank < superSize){
    if (fileId >= 0){
      if( not h5groups.empty() ){
	for ( map<string,hid_t>::iterator it=h5groups.begin(); it != h5groups.end(); it++ ) {
	  if( ERRORCHECK(H5Oclose((*it).second)) ){
	    hasError = true;
	    errorQueue.pushError("trouble closing group \"" + it->first + "\"!");
	  }
	}
      }
      /*      
      if (H5Fget_obj_count(fileId,H5F_OBJ_ALL)!=1) {
	cerr << "WARNING: OBJECTS STILL OPEN!" << endl;
	cerr << "  objects   : " << H5Fget_obj_count(fileId,H5F_OBJ_ALL)-1 << endl;
	cerr << "  groups    : " << H5Fget_obj_count(fileId,H5F_OBJ_GROUP) << endl;
	cerr << "  datasets  : " << H5Fget_obj_count(fileId,H5F_OBJ_DATASET) << endl;
	cerr << "  attributes: " << H5Fget_obj_count(fileId,H5F_OBJ_ATTR) << endl;
      }
      */
      if( ERRORCHECK(H5Fclose(fileId)) )
	hasError = true;
      fileId = -999;
    }
  }

  if (hasError)
    errorQueue.pushError("Hdf5::close failed!");

  return not hasError;
#else
  errorQueue.pushError("HAS_HDF5 is undefined");
  return false;
#endif
}
