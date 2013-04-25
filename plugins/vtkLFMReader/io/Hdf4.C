#include "Hdf4.hpp"

#ifndef MAX_VAR_DIMS
#define MAX_VAR_DIMS H4_MAX_VAR_DIMS
#endif

#ifndef MAX_NC_NAME
#define MAX_NC_NAME H4_MAX_NC_NAME
#endif


/*----------------------------------------------------------------------------*/

Hdf4::Hdf4(int superDomainSize) : Io(superDomainSize), sdId(-999)
{
  extension = "hdf4";
}

/*----------------------------------------------------------------------------*/

Hdf4::~Hdf4()
{
#ifdef HAS_HDF4
  close();
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf4::isEnabled()
{
#ifdef HAS_HDF4
  return true;
#else
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf4::openRead(const string &filename)
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

bool Hdf4::openWrite(const string &filename)
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

#ifdef HAS_HDF4
bool Hdf4::errorCheck(const int &status, const char *file, const int &line, const char *func)
{
  if (status < 0) {
    stringstream errorString;
    errorString << "*** Error in" << file << "(L " << line << "): " << func << "(...)" << endl;
    errorString << "HDF4 error code: " << status << endl;
    HEprint(stderr,0);
    errorQueue.pushError(errorString);
    return true;
  }
  return false;
}
#endif

/*----------------------------------------------------------------------------*/

bool Hdf4::readVariable( const string& variable, 
			 const string& group,
			 const array_info_t& info,
			 void* data )
{
#ifdef HAS_HDF4
  int32 i, indexStart[MAX_VAR_DIMS], dims[MAX_ARRAY_DIMENSION];

  //cout << "Reading " << variable << " in group " << group << endl;

  for(i=0;i<MAX_VAR_DIMS;i++) indexStart[i] = 0;

  if (!verifyShape(variable,group,info)) return false;

  if (rank < superSize){    
    string id = (group==""?variable:group+"/"+variable);
    int varId = SDnametoindex(sdId,id.c_str());
    ERRORCHECK(varId);
    int sdsId = SDselect(sdId,varId);
    ERRORCHECK(sdsId);
    ERRORCHECK(SDreaddata(sdsId,indexStart,NULL,int32_convert(info.localDims,info.nDims,dims),data));
    ERRORCHECK(SDendaccess(sdsId));
  }
  return true;
#else
  return false;
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf4::readAttribute( const string& variable,
			  void* data,
			  int& dataLength, 
			  const identify_data_type& dataType,
			  const string& group) 
{
#ifdef HAS_HDF4
  bool hasError = false;

  char readName[MAX_NC_NAME];
  if (rank < superSize) {
    int32 groupId = openGroup(group);
    if( ERRORCHECK(groupId) )
      hasError = true;
    int32 attrIndx = SDfindattr(groupId,variable.c_str());
    if (attrIndx==-1) return -1;
    int32 type;
    if (ERRORCHECK(SDattrinfo(groupId,attrIndx,readName,&type,&dataLength)))
      hasError = true;
    if (ERRORCHECK(( identifyH4Type(dataType,variable) == type ? 1 : -1 )))
      hasError = true;
    if (ERRORCHECK(SDreadattr(groupId,attrIndx,data)))
      hasError = true;

    if (hasError){
      stringstream ss;
      ss << __FUNCTION__ << " arguments:" << endl
	 << "\tvariable=" << variable << endl
	 << "\tdataLength=" << dataLength << endl
	 << "\tdata_type=" << dataType2String(dataType) << endl
	 << "\tgroup=" << group << endl;
      
      errorQueue.pushError(ss);
      //errorQueue.print(cerr);
    }

    return (not hasError);
  }
#endif
  errorQueue.pushError("HAS_HDF4 is undefined");
  return false;
}

/*----------------------------------------------------------------------------*/

void Hdf4::writeVariable( const string& variable, 
			  const string& group,
			  const array_info_t& info,
			  const void* data )
{
#ifdef HAS_HDF4
  int32 i,indexStart[MAX_VAR_DIMS], dims[MAX_ARRAY_DIMENSION];
  for(i=0;i<MAX_VAR_DIMS;i++) indexStart[i] = 0;

  if (rank < superSize){
    string id = (group==""?variable:group+"/"+variable);
    int32 varId = SDcreate(sdId, id.c_str(), identifyH4Type(info.dataType,variable), 
			   info.nDims, int32_convert(info.localDims,info.nDims,dims));
    ERRORCHECK(varId);
    ERRORCHECK(SDwritedata(varId, indexStart, NULL, 
			   int32_convert(info.localDims,info.nDims,dims), (void*)data));
    ERRORCHECK(SDendaccess(varId));
    putArrayInfo(id,info);
  }
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf4::writeAttribute( const string& variable,
			   const void* data,
			   const int& dataLength,
			   const identify_data_type& dataType,
			   const string& group)
{
#ifdef HAS_HDF4
  bool hasError = false;

  if (rank < superSize && dataLength>0) {
    int32 groupId = createGroup(group);
    if( ERRORCHECK(SDsetattr(groupId,variable.c_str(),identifyH4Type(dataType,variable),dataLength,data)) )
      hasError = true;
  }

  if (hasError){
    stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariable=" << variable << endl
       << "\tdataLength=" << dataLength << endl
       << "\tdata_type=" << dataType2String(dataType) << endl
       << "\tgroup=" << group << endl;
    
    errorQueue.pushError(ss);
    //errorQueue.print(cerr);
  }
  return (not hasError);
#endif
  errorQueue.pushError("HAS_HDF4 is undefined");
  return true;
}


/*----------------------------------------------------------------------------*/

void Hdf4::getBcastArrayInfo( const string& group,
			      array_info_t& info  ) {
#ifdef HAS_HDF4
#ifdef BUILD_WITH_MPI
  if (rank==0) {
    Io::readAttribute("globalDims", info.globalDims[0], info.nDims, group);
    Io::readAttribute("base", info.base[0], group);
  }

  MPI_Bcast(&info.nDims, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (info.nDims) {
    MPI_Bcast(info.globalDims, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(info.base, info.nDims, MPI_INT, 0, MPI_COMM_WORLD);
  }
#endif
#endif // BUILD_WITH_MPI
}

/*----------------------------------------------------------------------------*/


void Hdf4::getLocalArrayInfo( const string& group,
			      array_info_t& info  ) {
#ifdef HAS_HDF4
  memset(&info,0,sizeof(info));
  if (rank<superSize) {
    Io::readAttribute("globalDims", info.globalDims[0], info.nDims, group);
    Io::readAttribute("offset", info.offset[0], group);
    Io::readAttribute("base", info.base[0], group);
    Io::readAttribute("nVars", info.nVars, group);
    int32 nPoints=0, dims[MAX_VAR_DIMS], dataType;
    int32 varId = SDnametoindex( sdId, group.c_str() );
    ERRORCHECK(varId);
    int32 sdsId = SDselect( sdId, varId );
    ERRORCHECK(sdsId);
    ERRORCHECK(SDgetinfo( sdsId, NULL, &nPoints, dims, &dataType, &info.nAttr ));
    ERRORCHECK(SDendaccess(sdsId));
    info.dataType = H4identifyType(dataType,group);
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

void Hdf4::putArrayInfo( const string& group,
			 const array_info_t& info ) {
#ifdef HAS_HDF4
  //if (rank==0)
  Io::writeAttribute("base", info.base[0], info.nDims, group);
  Io::writeAttribute("globalDims", info.globalDims[0], info.nDims, group);
  Io::writeAttribute("offset", info.offset[0],info.nDims, group);
  //Io::writeAttribute("localDims",info.localDims[0],info.nDims,group);
#endif
}

/*----------------------------------------------------------------------------*/

bool Hdf4::verifyShape( const string& variable,
			const string& group,
			const array_info_t& info ) {
#ifdef HAS_HDF4
  int nPoints=0, dims[MAX_VAR_DIMS], dataType, nAttrs, error=0;

  if (rank < superSize){
    string id = (group==""?variable:group+"/"+variable);
    int32 varId = SDnametoindex( sdId, id.c_str() );
    if (varId<0)
      cout << "Variable " << variable << " not found." << endl;
    ERRORCHECK(varId);
    int32 sdsId = SDselect( sdId, varId );
    ERRORCHECK(sdsId);
    ERRORCHECK(SDgetinfo( sdsId, NULL, &nPoints, dims, &dataType, &nAttrs ));
    ERRORCHECK(SDendaccess(sdsId));
    
    if (nPoints == info.nDims) {
      for (int i=0; i<nPoints && !error; i++) {
	if (dims[i] != info.localDims[i]) error = 1;
      }
    } else error = 1;
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

const list<string> Hdf4::getVarNames()
{
  list<string> r;
#ifdef HAS_HDF4
  if (rank < superSize){
    int num_data, num_attr;
    int nPoints=0, dims[MAX_VAR_DIMS], dataType, nAttrs, error=0;
    SDfileinfo(sdId, &num_data, &num_attr);
    for (int i=0; i<num_data; i++) {
      int32 sdsId = SDselect(sdId,i);
      ERRORCHECK(sdsId);
      uint16 nameLen;
      SDgetnamelen(sdsId,&nameLen);
      char* name = new char[nameLen];
      ERRORCHECK(SDgetinfo( sdsId, name, &nPoints, dims, &dataType, &nAttrs ));
      ERRORCHECK(SDendaccess(sdsId));
      r.push_back(string(name));
      delete[] name;
    }
  }
#endif
  return r;
}


#ifdef HAS_HDF4
/*----------------------------------------------------------------------------*/

int32 Hdf4::openGroup(const string &group)
{
  int32 groupId = sdId;
  if (rank < superSize && group != ""){
    if (h4groups.find(group) != h4groups.end())
      groupId = h4groups[group];
    else {
      int32 varId = SDnametoindex(sdId,group.c_str());
      ERRORCHECK(varId);
      groupId = SDselect(sdId,varId);
      ERRORCHECK(groupId);
      h4groups[group] = groupId;
    }
  }
  return groupId;
}

/*----------------------------------------------------------------------------*/

int32 Hdf4::createGroup(const string &group)
{
  int32 groupId = sdId;
  if (rank < superSize && group != ""){
    if (h4groups.find(group) != h4groups.end())
      groupId = h4groups[group];
    else {
      int32 varId = SDnametoindex(sdId,group.c_str());
      groupId = (varId != -1 ? SDselect(sdId,varId) :
		 SDcreate(sdId, group.c_str(), DFNT_CHAR8, 0, NULL));		  
      ERRORCHECK(groupId);
      h4groups[group] = groupId;
    }
  }
  return groupId;
}

/*----------------------------------------------------------------------------*/

bool Hdf4::open(const string &filename, const int32 &accessMode)
{
  sdId = -1;

  if (superSize == -1) {
    if (accessMode == DFACC_RDONLY) {
      if (rank == 0) {      
	superSize = 1;
	sdId = SDstart(filename.c_str(), accessMode);
	ERRORCHECK(sdId);
	Io::readAttribute("superSize", superSize);
      }
#ifdef BUILD_WITH_MPI
      MPI_Bcast(&superSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif //BUILD_WITH_MPI
    } else {
      superSize = nProcs;
    }
#ifdef BUILD_WITH_APP
    if (superSize>0) partitionSuper = Partitioning_Type(superSize);
#endif//BUILD_WITH_APP
  }

  if (rank < superSize){
    if (accessMode == DFACC_RDONLY) {
      if (sdId == -1) {
        sdId = SDstart(filename.c_str(), accessMode);
      }
    } else if (accessMode == DFACC_CREATE) {
      sdId = SDstart(filename.c_str(), accessMode);
      Io::writeAttribute("superSize",superSize, 1);
      Io::writeAttribute("rank",rank, 1);
    } else {
      cerr << __FILE__ << " (" << __LINE__ << "): " << __FUNCTION__ 
	   << "Did not understand file access mode" << endl;
    }
    ERRORCHECK(sdId);
  }
  return true;
}

#endif

/*----------------------------------------------------------------------------*/

void Hdf4::close(void)
{
#ifdef HAS_HDF4
  if (rank < superSize){
    if (sdId >= 0){
      for ( map<string,int32>::iterator it=h4groups.begin(); it != h4groups.end(); it++ ) {
        ERRORCHECK(SDendaccess((*it).second));
      }
      ERRORCHECK(SDend(sdId));
      sdId = -999;
    }
  }
#endif
}

