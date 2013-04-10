
#include "Io.hpp"

#include <dirent.h>
#include <errno.h>

/*----------------------------------------------------------------------------*/

Io::Io(int superDomainSize) : superSize( superDomainSize ), c_order(true) {

#ifndef NOMPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &nProcs);
#else
  rank = 0;
  nProcs = 1;
#endif//NOMPI

  if (rank == -1 && nProcs == 1) {
    rank = 0; 
    superSize = 1;
  }

#ifndef NOPP
  if (superSize>0) partitionSuper = Partitioning_Type(superSize);
#endif//NOPP

}

/*----------------------------------------------------------------------------*/

Io* Io::extSelector(const string& ext, const int& superDomainSize) {

  if (ext == "hdf" || ext == "dmp") 
    return new Hdf(superDomainSize);
  else if (ext == "hdf4") 
    return new Hdf4(superDomainSize);
  else if (ext == "hdf5") 
    return new Hdf5(superDomainSize);
  else if (ext == "phdf5") 
    return new PHdf5(superDomainSize);
  else
    return NULL;
}

/*----------------------------------------------------------------------------*/

Io* Io::fileComplete(const string& fileName, const string& dir, const int& superDomainSize) {

  DIR *dp;
  struct dirent *dirp;
  if((dp  = opendir(dir.c_str())) == NULL) {
    cerr << "Errorno " << errno << " opening " << dir << endl;
    return NULL;
  }

  Io* r = NULL;

  while ((dirp=readdir(dp))!=NULL && r==NULL) {
    string file = dirp->d_name;

    if (file.compare(fileName+".hdf")==0 || file.compare(fileName+".dmp")==0) 
      r = new Hdf(superDomainSize);
    else if (file.compare(fileName+"_0.hdf4")==0) 
      r = new Hdf4(superDomainSize);
    else if (file.compare(fileName+"_0.hdf5")==0) 
      r = new Hdf5(superDomainSize);
    else if (file.compare(fileName+".phdf5")==0) 
      r = new PHdf5(superDomainSize);
  }
  closedir(dp);

  return r;
}
