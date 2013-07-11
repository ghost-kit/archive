#ifndef IO_TEMPLATES_HPP__
#define IO_TEMPLATES_HPP__

#ifdef BUILD_WITH_APP

/*----------------------------------------------------------------------------*/

template<class PppArray>
array_info_t& Io::fillInfo(const PppArray& data, 
			   array_info_t& info, 
			   const int multiVarDims) {
  if (multiVarDims) {
    info.nDims = multiVarDims-1;
    info.nVars = data.getLocalLength(info.nDims);
  } else {
    info.nDims = data.numberOfDimensions();
    info.nVars = 1;
  }
  
  for (int i=0; i<MAX_ARRAY_DIMENSION; i++){
    if (i<info.nDims && rank<superSize) {
      info.globalDims[(isArrayCOrdered?info.nDims-i-1:i)] = data.getLength(i);
      info.localDims[(isArrayCOrdered?info.nDims-i-1:i)] = data.getLocalLength(i);
      info.offset[(isArrayCOrdered?info.nDims-i-1:i)] = data.getLocalBase(i) - data.getBase(i);
      info.base[(isArrayCOrdered?info.nDims-i-1:i)] = data.getBase(i);
    } else {
      info.globalDims[i] = info.localDims[i] = info.offset[i] = info.base[i] = 0;
    }
  }
  return info;
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeShape(const PppArray& data, 
		    const std::string& group) {
  array_info_t info;
  PppArray tmpArray = data;

  tmpArray.partition(partitionSuper);
  putArrayInfo(group,fillInfo(tmpArray,info));
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
bool Io::writeVariable(const PppArray& data, 
		       const std::string& variableName, 
		       const std::string& group, 
		       const int multiVarDims) {
  bool hasError = false;

  array_info_t info;
  PppArray tmpArray = data;
  info.dataType = identify(tmpArray.getDataPointer()[0]);

  tmpArray.partition(partitionSuper);
  fillInfo(tmpArray,info,multiVarDims);

  size_t offset = 1;
  for (int i=0; i<info.nDims; i++) offset *= info.localDims[i];
  
  for (int i=0; i<info.nVars; i++) {
    if(not writeVariable(variableName+(i==0?"":"."+toString(i)),group,info,
			 tmpArray.getLocalArray().getDataPointer()+(i*offset)) ){
      hasError = true;
    }
  }

  if (multiVarDims){
    if( not writeAttribute("nVars", info.nVars, 1(group==""?variableName:group+"/"+variableName)) ){
      hasError = true;
      errorQueue.pushError("Error writing attribute nVars=" + std::string(info.nVars));
    }
  }

  if (hasError){
    std::stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariableName=" << variableName << endl
       << "\tgroup=" << group << endl
       << "\tmultiVarDims=" << multiVarDims << endl;
    errorQueue.pushError(ss);
  }

  return not hasError;
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeVarUnits(const PppArray& data, 
		       const std::string& variableName, 
		       const std::string& units, 
		       const std::string& group) {
  bool hasError = false;
  
  if(not writeVariable(data,variableName,group) )
    hasError = true;
  if(not writeAttribute("units",units, units.length(), (group==""?variableName:group+"/"+variableName)) )
    hasError = true;

    if (hasError){
    std::stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariableName=" << variableName << endl
       << "\tunits=" << units << endl
       << "\tgroup=" << group << endl;
    
    errorQueue.pushError(ss);
    //errorQueue.print(cerr);
  }
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeMultiVar(const PppArray& data, 
		       const std::string& variableName, 
		       const std::string& group,
		       const int multiVarDims) {
  return writeVariable(data,variableName,group,multiVarDims);
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeMultiVarUnits(const PppArray& data, 
			    const std::string& variableName, 
			    const std::string& units, 
			    const std::string& group,
			    const int multiVarDims) {
  bool hasError = false;

  if(not writeVariable(data,variableName,group,multiVarDims) )
    hasError = true;
  if(not writeAttribute("units",units, units.length(), (group==""?variableName:group+"/"+variableName)) )
    hasError = true;
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
PppArray& Io::readShape(PppArray& data, 
			const std::string& group, 
			const bool& conformityCheck) {
  array_info_t info;

  for (int i=0; i<MAX_ARRAY_DIMENSION; i++) 
    info.globalDims[i] = info.localDims[i] = info.offset[i] = info.base[i] = 0;

  if (data.isNullArray()) data = PppArray(1);
  else data.redim(1);

  data.partition(partitionSuper);

  getBcastArrayInfo(group,info);

  switch(info.nDims) {
  case 0: cerr << "No dimension info..." << endl; break;
  case 1: data.redim(info.globalDims[0]); break;
  case 2: data.redim(info.globalDims[1],
		     info.globalDims[0]); break;
  case 3: data.redim(info.globalDims[2],
		     info.globalDims[1],
		     info.globalDims[0]); break;
  case 4: data.redim(info.globalDims[3],
		     info.globalDims[2],
		     info.globalDims[1],
		     info.globalDims[0]); break;
  case 5: data.redim(info.globalDims[4],
		     info.globalDims[3],
		     info.globalDims[2],
		     info.globalDims[1],
		     info.globalDims[0]); break;
  case 6: data.redim(info.globalDims[5],
		     info.globalDims[4],
		     info.globalDims[3],
		     info.globalDims[2],
		     info.globalDims[1],
		     info.globalDims[0]); break;
  default:
    cerr << "Too many dimensions: " << info.nDims << endl;
    exit(-1);
  }

  for (int i=0; i<info.nDims; i++) data.setBase(info.base[info.nDims-i-1],i);

  if (conformityCheck && rank<superSize) {
    for (int i=0; i<info.nDims; i++) {
      if ((info.base[info.nDims-i-1] != data.getBase(i)) || 
	  (info.globalDims[info.nDims-i-1] != data.getLength(i)) ) {
	cerr << "Conformity check failed on proc " << rank 
	     << " for dimension " << i << ": " << endl;
	//exit(-1);
      }
    }
  }
  return data;
}

/*----------------------------------------------------------------------------*/


template<class PppArray>
bool Io::readVariable(PppArray& data, 
		      const std::string& variableName, 
		      const std::string& group, 
		      const int multiVarDims) {

  bool hasError = false;

  array_info_t info;
  PppArray tmpArray = data;
  info.dataType = identify(tmpArray.getDataPointer()[0]);

  tmpArray.partition(partitionSuper); 
  fillInfo(tmpArray,info,multiVarDims);  

  int nVars = 1;
  if (multiVarDims) {
    int rank;
    if(not readAttribute0("nVars",nVars,rank, group) )
      hasError = true;
  }
  
  size_t offset = 1;
  for (int i=0; i<info.nDims; i++) 
    offset *= info.localDims[i];

  MPI_Bcast(&nVars,1,MPI_INT,0,MPI_COMM_WORLD);

  if (rank==0 && nVars > info.nVars) {
    hasError = true;
    std::stringstream ss;
    ss << rank << ": more multi-vars in file than memory available for variable " 
       << variableName << " : " << nVars << " > " << info.nVars << endl;
    errorQueue.pushError(ss);
    //errorQueue.print(cerr);
    MPI_Abort(MPI_COMM_WORLD,-1);
  }

  for (int i=0; i<nVars; i++) {
    if (not readVariable(variableName+(i==0?"":"."+toString(i)),group,info,
			 tmpArray.getLocalArray().getDataPointer()+(i*offset))) {      
      hasError = true;
      Communication_Manager::Sync();
      usleep(10000*rank);
      std::stringstream ss;
      ss << rank << ": shape for " << variableName << " does not match?!" << endl;
      errorQueue.pushError(ss);
      //errorQueue.print(cerr);
      
      MPI_Abort(MPI_COMM_WORLD,-1);
    }
  }

  memset(data.getLocalArray().getDataPointer(),0,
	 data.getLocalSize()*sizeof(*(data.getLocalArray().getDataPointer())));
  tmpArray.partition(data.getPartition());
  data = tmpArray;

  return (not hasError);
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::readVarUnits(PppArray& data, 
		      const std::string& variableName, 
		      std::string& units, 
		      const std::string& group) {
  bool hasError = false;
  if(not readVariable(data,variableName,group) )
    hasError = true;
  int nUnits;
  if(not readAttribute("units",units, nUnits, group) )
    hasError = true;
  
  if (hasError){
    std::stringstream ss;
    ss << __FUNCTION__ << " arguments:" << endl
       << "\tvariableName=" << variableName << endl
       << "\tunits=" << units << endl
       << "\tgroup=" << group << endl;
    
    errorQueue.pushError(ss);
    //errorQueue.print(cerr);
  }

  return hasError;
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::readMultiVar(PppArray& data, 
		      const std::string& variableName, 
		      const std::string& group,
		      const int multiVarDims) {
  return readVariable(data,variableName,group,multiVarDims);
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::readMultiVarUnits(PppArray& data, 
			   const std::string& variableName, 
			   std::string& units, 
			   const std::string& group,
			   const int multiVarDims) {
  bool hasError = false;
  
  if( not readVariable(data,variableName,group,multiVarDims) )
    hasError = true; 
  int nUnits;
  if( not readAttribute("units",units, nUnits, group) )
    hasError = true;
}

#endif//BUILD_WITH_APP

/*----------------------------------------------------------------------------*/

template<class T>
bool Io::writeAttribute(const std::string& attributeName, 
			const T& data, 
			const int& dataLength,
			const std::string& group) {
  return writeAttribute(attributeName,&data,dataLength,identify(data),group);
}

/*----------------------------------------------------------------------------*/

template<class T>
bool Io::writeAttribute0(const std::string& attributeName,
			 const T& data,
			 const int& dataLength,
			 const std::string& group) {
  return (rank==0?writeAttribute(attributeName,data,dataLength,group):(void)0);
}

/*----------------------------------------------------------------------------*/

template<> inline
bool Io::writeAttribute<std::string>(const std::string& attributeName, 
				const std::string& data,
				const int& dataLength,
				const std::string& group) {
  return writeAttribute(attributeName,data.c_str(), data.length(), identify(data), group);
}

/*----------------------------------------------------------------------------*/

template<class T>
bool Io::readAttribute(const std::string& attributeName, 
		       T& data, 
		       int& dataLength,
		       const std::string& group) const {
  return readAttribute(attributeName,&data,dataLength,identify(data),group);
}

/*----------------------------------------------------------------------------*/

template<class T>
bool Io::readAttribute(const std::string& attributeName, 
		       T& data, 
		       const std::string& group) const {
  int dataLength;
  return readAttribute(attributeName,data,dataLength,group);
}

/*----------------------------------------------------------------------------*/

template<class T>
bool Io::readAttribute0(const std::string& attributeName, 
			T& data, 		       
			int& dataLength,
			const std::string& group) const{
  return (rank==0?readAttribute(attributeName,data,dataLength,group):0);
}

/*----------------------------------------------------------------------------*/

template<> inline
bool Io::readAttribute<std::string>(const std::string& attributeName,
			       std::string& data, 
			       int& dataLength,
			       const std::string& group) const {
  static const int MAX_STR_LEN = 2048;
  static char str[MAX_STR_LEN] = { 0 };
  readAttribute(attributeName, str, dataLength, identify(data), group);
  if (dataLength>0) data = str;
  return true;
}

#endif//IO_TEMPLATES_HPP__
