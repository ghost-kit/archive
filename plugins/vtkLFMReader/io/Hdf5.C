#include "Hdf5.hpp"

/*----------------------------------------------------------------------------*/

Hdf5::Hdf5(int superDomainSize) : Io(superDomainSize), fileId(-999)
{
  ext = "hdf5";
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

bool Hdf5::enabled()
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

void Hdf5::errorCheck(const int& status, const char* file, const int& line, const char* func)
{
  if (status < 0) {
    pushError("unknown error",file,line,func);
  }
}

/*----------------------------------------------------------------------------*/

void Hdf5::pushError(const string &e, const char *file, const int &line, const char *func)
{
  H5Epush(H5E_DEFAULT,file,func,line,classId,majorErrorId,minorErrorId,e.c_str());
  H5Eprint(H5E_DEFAULT, stderr);
  MPI_Abort(MPI_COMM_WORLD,-1);
  exit(-1);
}

#endif
/*----------------------------------------------------------------------------*/

bool Hdf5::readVariable( const string& variable, 
			 const string& group,
			 const array_info_t& info,
			 void* data )
{   
#ifdef HAS_HDF5
  if (!verifyShape(variable,group,info)) return false;
  if (rank < superSize) {
    hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
    ERRORCHECK(groupId);
    hid_t datasetId = H5Dopen(groupId, variable.c_str(), H5P_DEFAULT);
    ERRORCHECK(datasetId);
    ERRORCHECK(H5Dread(datasetId, identifyH5Type(info.dataType,variable), H5S_ALL, H5S_ALL, H5P_DEFAULT, data));
    ERRORCHECK(H5Dclose(datasetId));
    if (group!="") ERRORCHECK(H5Oclose(groupId));
  }
  return true;
#else
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

int Hdf5::readAttribute( const string& variable,
			 const string& group,
			 const identify_data_type& dataType,
			 void* data,
			 const int& len ) 
{
#ifdef HAS_HDF5
  hsize_t nPoints = 0;
  hid_t h5type;
  if (rank < superSize){
    if (group=="" || H5Lexists(fileId,group.c_str(),H5P_DEFAULT)) {
      hid_t groupId = (group==""?fileId:H5Oopen(fileId,group.c_str(),H5P_DEFAULT));
      ERRORCHECK(groupId);
      if (H5Aexists(groupId,variable.c_str())) {
	hid_t attributeId = H5Aopen(groupId,variable.c_str(),H5P_DEFAULT);      
	ERRORCHECK(attributeId);
	hid_t spaceId = H5Aget_space(attributeId);
	ERRORCHECK(spaceId);
	if (dataType == identify_string_t) {
	  h5type = H5Aget_type(attributeId);
	  nPoints = H5Tget_size(h5type);
	} else {
	  h5type = identifyH5Type(dataType,variable);
	  nPoints = H5Sget_select_npoints(spaceId);
	}
	if (len>=nPoints) {
	  ERRORCHECK(H5Aread(attributeId,h5type,data));
	} else {
	  PUSHERROR("Unable to read attribute " + variable + " with provided length.");
	}
	ERRORCHECK(H5Aclose(attributeId));
	ERRORCHECK(H5Sclose(spaceId));      
      } else  {
	return -1;
      }
      if (group!="") ERRORCHECK(H5Oclose(groupId));
    } else {
      PUSHERROR("Group " + group + "does not exist...");
    }
  }
  return nPoints;
#else
  return 0;
#endif
}


/*----------------------------------------------------------------------------*/

void Hdf5::writeVariable( const string& variable, 
			  const string& group,
			  const array_info_t& info,
			  const void* data )
{
#ifdef HAS_HDF5
  if (rank < superSize){
    hsize_t localDims[MAX_ARRAY_DIMENSION];
    hid_t dataspaceId = H5Screate_simple(hsize_t(info.nDims), 
					 hsize_convert(info.localDims,info.nDims,localDims), NULL);
    ERRORCHECK(dataspaceId);
    hid_t datasetId = H5Dcreate(createGroup(group), variable.c_str(), 
				identifyH5Type(info.dataType,variable), dataspaceId, 
				H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    ERRORCHECK(datasetId);
    ERRORCHECK(H5Dwrite(datasetId, identifyH5Type(info.dataType,variable), 
			H5S_ALL, H5S_ALL, H5P_DEFAULT, data));
    ERRORCHECK(H5Dclose(datasetId));
    ERRORCHECK(H5Sclose(dataspaceId));
    putArrayInfo((group==""?variable:group+"/"+variable),info);
  }
#endif
}

/*----------------------------------------------------------------------------*/

void Hdf5::writeAttribute( const string& variable,
			   const string& group,
			   const identify_data_type& dataType,
			   const void* data,
			   const int& len ) 
{
#ifdef HAS_HDF5
  if (rank < superSize){
    hsize_t hsize_len = hsize_t((len>0?len:1));
    hid_t h5type = identifyH5Type(dataType,variable);
    if (dataType == identify_string_t) {
      h5type = H5Tcopy(h5type);
      H5Tset_size(h5type,hsize_len);
      hsize_len = 1;
    }
    hid_t dataspaceId = H5Screate_simple(hsize_t(1), &hsize_len, NULL);
    ERRORCHECK(dataspaceId);
    hid_t attributeId = H5Acreate(createGroup(group), variable.c_str(), h5type, dataspaceId, H5P_DEFAULT, H5P_DEFAULT);
    ERRORCHECK(attributeId);
    ERRORCHECK(H5Awrite(attributeId, h5type, data));
    ERRORCHECK(H5Aclose(attributeId));
    ERRORCHECK(H5Sclose(dataspaceId));
  }
#endif
}

/*----------------------------------------------------------------------------*/

void Hdf5::getBcastArrayInfo( const string& group,
			      array_info_t& info  ) {
#ifdef HAS_HDF5
  if (rank==0) {
    info.nDims = Io::readAttribute(info.globalDims[0],"globalDims",group,MAX_ARRAY_DIMENSION);
    Io::readAttribute(info.base[0],"base",group,MAX_ARRAY_DIMENSION);
  }
  
  MPI_Bcast(&info.nDims, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (info.nDims) {
    MPI_Bcast(info.globalDims, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(info.base, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
  }
#endif
}

/*----------------------------------------------------------------------------*/

void Hdf5::getLocalArrayInfo( const string& group,
			      array_info_t& info  ) {
#ifdef HAS_HDF5
  memset(&info,0,sizeof(info));
  if (rank<superSize) {
    info.nDims = Io::readAttribute(info.globalDims[0],"globalDims",group,MAX_ARRAY_DIMENSION);
    Io::readAttribute(info.offset[0],"offset",group,MAX_ARRAY_DIMENSION);
    Io::readAttribute(info.base[0],"base",group,MAX_ARRAY_DIMENSION);
    Io::readAttribute(info.nVars,"nVars",group);
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
  Io::writeAttribute(info.base[0],"base",group,info.nDims);
  Io::writeAttribute(info.globalDims[0],"globalDims",group,info.nDims);
  Io::writeAttribute(info.offset[0],"offset",group,info.nDims);
  //Io::writeAttribute(info.localDims[0],"localDims",group,info.nDims);
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf5::verifyShape( const string& variable,
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
      hid_t dataId = H5Dopen(groupId,variable.c_str(),H5P_DEFAULT);
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
    cout << "Var: " << variable << " group: " << group << " rank: " << rank 
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
static herr_t visit_objs(hid_t id, const char *name, const H5O_info_t *info, void *op_data)
{
  if (info->type == H5O_TYPE_DATASET)
    reinterpret_cast<list<string> *>(op_data)->push_back(string(name));
  return(H5_ITER_CONT);
}
#endif

const list<string> Hdf5::getVarNames()
{
  list<string> r;
#ifdef HAS_HDF5
  H5Ovisit(fileId,H5_INDEX_NAME,H5_ITER_NATIVE,visit_objs,&r);
#endif
  return r;
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
  string file = filename + "_" + toString(rank) + "." + ext;
  fileId = -1;

  if (superSize == -1) {
    if (accessMode ==  H5F_ACC_RDONLY) {
      if (rank == 0) {      
	superSize=1;
	fileId = H5Fopen(file.c_str(), accessMode, H5P_DEFAULT);
	Io::readAttribute(superSize,"superSize");
      }
      MPI_Bcast(&superSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
      superSize = nProcs;
    }
#ifndef NOPP
    if (superSize>0) partitionSuper = Partitioning_Type(superSize);
#endif//NOPP
  }

  if (rank < superSize){
    if (accessMode == H5F_ACC_RDONLY) {
      if (fileId == -1) fileId = H5Fopen(file.c_str(), accessMode, H5P_DEFAULT);
    } else if (accessMode == H5F_ACC_TRUNC) {
      if (fileId == -1) fileId = H5Fcreate(file.c_str(), accessMode, H5P_DEFAULT, H5P_DEFAULT);
      Io::writeAttribute(superSize,"superSize");
      Io::writeAttribute(rank,"rank");
    } else {
      cerr << __FILE__ << " (" << __LINE__ << "): " << __FUNCTION__ 
	   << "Did not understand file access mode" << endl;
    }
    ERRORCHECK(fileId);
  }
  return true;
}
#endif

/*----------------------------------------------------------------------------*/

void Hdf5::close()
{
#ifdef HAS_HDF5
  if (rank < superSize){
    if (fileId >= 0){
      for ( map<string,hid_t>::iterator it=h5groups.begin(); it != h5groups.end(); it++ ) {
	ERRORCHECK(H5Oclose((*it).second));
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
      ERRORCHECK(H5Fclose(fileId));
      fileId = -999;
    }
  }
#endif
}
