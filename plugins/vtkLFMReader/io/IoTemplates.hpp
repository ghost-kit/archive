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
		    const string& group) {
  array_info_t info;
  PppArray tmpArray = data;

  tmpArray.partition(partitionSuper);
  putArrayInfo(group,fillInfo(tmpArray,info));
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeVariable(const PppArray& data, 
		       const string& variable, 
		       const string& group, 
		       const int multiVarDims) {
  array_info_t info;
  PppArray tmpArray = data;
  info.dataType = identify(tmpArray.getDataPointer()[0]);

  tmpArray.partition(partitionSuper);
  fillInfo(tmpArray,info,multiVarDims);

  size_t offset = 1;
  for (int i=0; i<info.nDims; i++) offset *= info.localDims[i];
  
  for (int i=0; i<info.nVars; i++) {
    writeVariable(variable+(i==0?"":"."+toString(i)),group,info,
		  tmpArray.getLocalArray().getDataPointer()+(i*offset));
  }

  if (multiVarDims)
    writeAttribute(info.nVars,"nVars",(group==""?variable:group+"/"+variable));

}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeVarUnits(const PppArray& data, 
		       const string& variable, 
		       const string& units, 
		       const string& group) {
  writeVariable(data,variable,group);
  writeAttribute(units,"units",(group==""?variable:group+"/"+variable));
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeMultiVar(const PppArray& data, 
		       const string& variable, 
		       const string& group,
		       const int multiVarDims) {
  writeVariable(data,variable,group,multiVarDims);
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::writeMultiVarUnits(const PppArray& data, 
			    const string& variable, 
			    const string& units, 
			    const string& group,
			    const int multiVarDims) {
  writeVariable(data,variable,group,multiVarDims);
  writeAttribute(units,"units",(group==""?variable:group+"/"+variable));
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
PppArray& Io::readShape(PppArray& data, 
			const string& group, 
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
void Io::readVariable(PppArray& data, 
		      const string& variable, 
		      const string& group, 
		      const int multiVarDims) {

  array_info_t info;
  PppArray tmpArray = data;
  info.dataType = identify(tmpArray.getDataPointer()[0]);

  tmpArray.partition(partitionSuper); 
  fillInfo(tmpArray,info,multiVarDims);  

  int nVars = 1;
  if (multiVarDims) 
    readAttribute0(nVars,"nVars",(group==""?variable:group+"/"+variable));
  
  size_t offset = 1;
  for (int i=0; i<info.nDims; i++) offset *= info.localDims[i];

  MPI_Bcast(&nVars,1,MPI_INT,0,MPI_COMM_WORLD);

  if (rank==0 && nVars > info.nVars) {
    cerr << rank << ": more multi-vars in file than memory available for variable " 
	 << variable << " : " << nVars << " > " << info.nVars << endl;
    MPI_Abort(MPI_COMM_WORLD,-1);
  }

  for (int i=0; i<nVars; i++) {
    if (!readVariable(variable+(i==0?"":"."+toString(i)),group,info,
		      tmpArray.getLocalArray().getDataPointer()+(i*offset))) {      
      Communication_Manager::Sync();
      usleep(10000*rank);
      cerr << rank << ": shape for " << variable << " does not match?!" << endl;
      MPI_Abort(MPI_COMM_WORLD,-1);
    }
  }

  memset(data.getLocalArray().getDataPointer(),0,
	 data.getLocalSize()*sizeof(*(data.getLocalArray().getDataPointer())));
  tmpArray.partition(data.getPartition());
  data = tmpArray;
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::readVarUnits(PppArray& data, 
		      const string& variable, 
		      string& units, 
		      const string& group) {
  readVariable(data,variable,group);
  readAttribute(units,"units",(group==""?variable:group+"/"+variable));
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::readMultiVar(PppArray& data, 
		      const string& variable, 
		      const string& group,
		      const int multiVarDims) {
  readVariable(data,variable,group,multiVarDims);
}

/*----------------------------------------------------------------------------*/

template<class PppArray>
void Io::readMultiVarUnits(PppArray& data, 
			   const string& variable, 
			   string& units, 
			   const string& group,
			   const int multiVarDims) {
  readVariable(data,variable,group,multiVarDims);
  readAttribute(units,"units",(group==""?variable:group+"/"+variable));
}

#endif//BUILD_WITH_APP

/*----------------------------------------------------------------------------*/

template<class T>
void Io::writeAttribute(const T& data, 
			const string& variable, 
			const string& group, 
			const int& len) {
  return writeAttribute(variable,group,identify(data),&data,len);
}

/*----------------------------------------------------------------------------*/

template<class T>
void Io::writeAttribute0(const T& data, 
			 const string& variable, 
			 const string& group, 
			 const int& len) {
  return (rank==0?writeAttribute(data,variable,group,len):(void)0);
}

/*----------------------------------------------------------------------------*/

template<> inline
void Io::writeAttribute<string>(const string& data, 
				const string& variable, 
				const string& group, 
				const int& len) {
  return writeAttribute(variable,group,identify(data),data.c_str(),
			(len==1?data.length():len));
}

/*----------------------------------------------------------------------------*/

template<class T>
int Io::readAttribute(T& data, 
		      const string& variable, 
		      const string& group, 
		      const int& len) {
  return readAttribute(variable,group,identify(data),&data,len);
}

/*----------------------------------------------------------------------------*/

template<class T>
int Io::readAttribute0(T& data, 
		       const string& variable, 
		       const string& group, 
		       const int& len) {
  
  return (rank==0?readAttribute(data,variable,group,len):0);
}

/*----------------------------------------------------------------------------*/

template<> inline
int Io::readAttribute<string>(string& data, 
			      const string& variable, 
			      const string& group, 
			      const int& len) {
  static const int MAX_STR_LEN = 2048;
  static char str[MAX_STR_LEN] = { 0 };
  int readLen = readAttribute(variable,group,identify(data),str,MAX_STR_LEN);
  if (readLen>0) data = str;
  return readLen;
}

#endif//IO_TEMPLATES_HPP__
