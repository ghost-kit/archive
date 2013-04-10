#include <mpi.h>
#include <cmath>
#include <sys/time.h>

#include <iostream>
#include <sstream>

#include "Io.hpp"

#include <A++.h>

#include <stdio.h>
#include <stdarg.h>
using namespace std;

#define HAVE_EXECINFO_H

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif /* HAVE_EXECINFO_H */

#include <csignal>

string fileName = "testData";

void print_trace (void)
{
#ifdef HAVE_EXECINFO_H
  size_t max = 256;
  void *array[max];
  size_t size;
  char **strings;
  size_t i;
     
  size = backtrace (array, max);
  strings = backtrace_symbols (array, size);     
  printf ("Obtained %zd stack frames.\n", size);     
  for (i = 0; i < size; i++)
    printf ("%s\n", strings[i]);
  free (strings);
#endif /* HAVE_EXECINFO_H */
}


int _err(const char *fmt, ...) {
  va_list argp;
  fprintf(stderr, "error: ");
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
  fprintf(stderr, "\n");

  print_trace();

  MPI_Abort(MPI_COMM_WORLD,-1);

  exit(-1);
}


void sigHandler( int param ) {
  _err("%s","received signal...");
}

int numProcs, rank;
//Partitioning_Type all;

template<class PppArray> bool equals(PppArray &a, Io *output, Io *input ) {

  PppArray b;
  Partitioning_Type part = a.getPartition();
  string units;
  if (rank==0)
    cout << "Testing with output " << (output?output->extName():"NULL") 
	 << " and input " << (input?input->extName():"NULL") << endl;

  if (output!=NULL) {
    if (output->openWrite(fileName))
      {
	output->writeVarUnits(a,"testVar","megaTests","testData");
	output->writeMultiVar(a,"testMultiVar","testData");
	output->close();
      }
  }

  if (input!=NULL) {
    //input->setReadPartition(part);
    if (input->openRead(fileName))
      {
	input->readShape(b,"testData/testVar");
	b.partition(part);
	input->readVarUnits(b,"testVar",units,"testData");
	
	list<string> names = input->getVarNames();
	for (list<string>::iterator it = names.begin(); it != names.end(); it++)
	  {
	    cout << rank << "] file " << fileName << " has var " << *it << endl;
	    array_info_t info;
	    input->getLocalArrayInfo(*it,info);
	    printArrayInfo(info);
	  }
	
	usleep(1000);
	MPI_Barrier(MPI_COMM_WORLD);

	input->close();
      }
  }
  
  if (!input->enabled())
    return false;
  
  int l1 = a.getLocalSize();
  int l2 = b.getLocalSize();

  int error = (l1==l2?0:1);
  int errorAll = 0;
  MPI_Allreduce(&error, &errorAll, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);

  if (errorAll) {
    usleep(10000*rank);
    if (l1!=l2) cerr << rank << " Sizes do not match?! l1:" << l1 << " l2:" << l2 << endl;
    return false;
  }
  
  int tot_bad = 0;
  for (int r=0; r<numProcs; r++) {
    if (r==rank) {
      int bad=0;
      for (int i=0; i<l1; i++) {
	if (*(a.getLocalArray().getDataPointer()+i) != 
	    *(b.getLocalArray().getDataPointer()+i)) {      
	  bad++;
	}
      }
      tot_bad += bad;
      if (bad) {
	cerr << rank << ": had " << bad << "/" << l1 << " bad values" << endl;
	usleep(10000);
      }
    }
    Communication_Manager::Sync();
  }
  delete input;
  delete output;

  return !tot_bad;
}

int main(int argc, char **argv)
{
  signal( SIGINT,  sigHandler );  
  signal( SIGTERM, sigHandler );
  //signal( SIGUSR1, sigHandler );
  signal( SIGUSR2, sigHandler );
  signal( SIGSEGV, sigHandler );

  numProcs = 0;

  char *localName = "IO test";
  Optimization_Manager::Initialize_Virtual_Machine(localName,numProcs,argc,argv);

  int numGhostCells = 4;
  rank = Communication_Manager::My_Process_Number;

  if (rank == -1 && numProcs == 1) {
    rank = 0;
  }

  Partitioning_Type all = Partitioning_Type(numProcs);
  all.partitionAlongAxis(0,TRUE,numGhostCells);
  all.partitionAlongAxis(1,TRUE,numGhostCells);
  all.partitionAlongAxis(2,TRUE,numGhostCells);

  int nip1=16, njp1=15, nkp1=13, nlp1=2;

  intArray testGhost(nip1, njp1, nkp1, nlp1), testTmp, testRead, testRead2, testRead3;
  testTmp = testGhost;

  Partitioning_Type noGhost(numProcs);
  testTmp.partition(noGhost);
  testTmp.setBase(1);
  testTmp.setBase(0,3);

  int ii=0;
  int *data1 = testTmp.getDataPointer();
  int i=0;

  for (int l=testTmp.getLocalBase(3); l <= testTmp.getLocalBound(3); l++){
    for (int k=testTmp.getLocalBase(2); k <= testTmp.getLocalBound(2); k++){
      for (int j=testTmp.getLocalBase(1); j <= testTmp.getLocalBound(1); j++){
	for (int i=testTmp.getLocalBase(0); i <= testTmp.getLocalBound(0); i++){
	  data1[ii++] = l*1000000 + k*10000 + j*100 + i + (rank+1)/100.;
	}
      }
    }
  }

  testGhost.partition(all);
  testGhost.setBase(1);
  testGhost.setBase(0,3);

  memset(testGhost.getLocalArray().getDataPointer(),0,testGhost.getLocalSize()*sizeof(int));
  testTmp.partition(all);
  testGhost = testTmp;

  if (!equals(testGhost, new Hdf(3), new Hdf(-1))) {
    if (rank==0) cout << "Hdf test FAILED!" << endl;
  } else {
    if (rank==0) cout << "Hdf test passed." << endl;
  }
  
  if (!equals(testGhost, new Hdf4(3), new Hdf4(-1))) {
    if (rank==0) cout << "Hdf4 test FAILED!" << endl;
  } else {
    if (rank==0) cout << "Hdf4 test passed." << endl;
  }
  
  if (!equals(testGhost, new Hdf5(3), new Hdf5(-1))) {
    if (rank==0) cout << "Hdf5 test FAILED!" << endl;
  } else {
    if (rank==0) cout << "Hdf5 test passed." << endl;
  }
 
  if (!equals(testGhost, new PHdf5(3), new PHdf5(-1))) {
    if (rank==0) cout << "PHdf5 test FAILED!" << endl;
  } else {
    if (rank==0) cout << "PHdf5 test passed." << endl;
  }
  
  Optimization_Manager::Exit_Virtual_Machine();
  return 0;

}
