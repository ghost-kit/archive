#include <iostream>
#include <sstream>

#include <A++.h>
#include <mpi.h>

using namespace std;

/*
PPP=/scratch/wilsone/pgi/install/P++/install; 
mpicxx.pgi partition.C -o partition -I${PPP}/include -L${PPP}/lib -lPpp -lPpp_static;
mpirun -np 8 ./partition 4 4 4 1 0;
mpirun -np 8 ./partition 4 4 4 1 1;
mpirun -np 8 ./partition 4 4 4 1 2;
*/

void printArray( const floatArray &v );
void delaySync(); // to flush stdout

int main(int argc, char **argv)
{
  int n_i=4, n_j=4, n_k=4, superSize=1, ghostCells=2, rank=0, numProcs=0;
  char *localName = "Partition test";

  Optimization_Manager::Initialize_Virtual_Machine(localName,numProcs,argc,argv);
  rank = Communication_Manager::My_Process_Number;
  numProcs = Communication_Manager::Number_Of_Processors;


  //------------------------------------------------
  // Parse arguments
  //------------------------------------------------

  if (argc>1 && (strcmp(argv[1],"?")==0 || strcmp(argv[1],"-?")==0)) {
    cout << argv[0] << " ni nj nk superSize ghostCells" << "\n\n";
    exit(0);
  }

  if (argc>3) {
    n_i = atoi(argv[1]);
    n_j = atoi(argv[2]);
    n_k = atoi(argv[3]);
  }
  if (argc>4) superSize = atoi(argv[4]);
  if (argc>5) ghostCells = atoi(argv[5]);


  //------------------------------------------------
  // Create the Grid
  //------------------------------------------------

  Partitioning_Type all(numProcs);
    
  all.partitionAlongAxis(0,TRUE,ghostCells);
  all.partitionAlongAxis(1,TRUE,ghostCells);
  all.partitionAlongAxis(2,TRUE,ghostCells);
  
  floatArray fdata(n_i, n_j, n_k);
  fdata.partition(all);
  fdata.setBase(1);

  //float *xp = fdata.getDataPointer();
  //int ii=0;
  /*
  for (int i=fdata.getLocalBase(2); i <= fdata.getLocalBound(2); i++){
    for (int j=fdata.getLocalBase(1); j <= fdata.getLocalBound(1); j++){
      for (int k=fdata.getLocalBase(0); k <= fdata.getLocalBound(0); k++){
	//xp[ii++] = i*10000 + j*100 + k + (rank+1)/100.;
	fdata(k,j,i) = i*10000 + j*100 + k + (rank+1)/100.;
      }
    }
  }
  */
  fdata = rank+1;

  //------------------------------------------------
  // Display Array
  //------------------------------------------------

  if (rank==0) cout << flush << endl << "Initial Array:" << endl << endl << flush;
  printArray(fdata);


  //------------------------------------------------
  // Perform Ghost Update and Display
  //------------------------------------------------

  if (rank==0) cout << flush << endl << "Ghost update:" << endl << endl << flush;
  fdata.updateGhostBoundaries();
  printArray(fdata);


  //------------------------------------------------
  // Super-Size and Display
  //------------------------------------------------

  Partitioning_Type super(superSize);
  floatArray serial = fdata;
  serial.partition(super);

  if (rank==0) cout << flush << endl << "Super-Sized result:" << endl << endl << flush;
  printArray(serial);

  
  //------------------------------------------------
  // Done
  //------------------------------------------------

  if (rank==0) cout << flush << endl << "Fin." << endl << endl << flush;
  Optimization_Manager::Exit_Virtual_Machine();
  return 0;

}


void displayInfo(const floatArray &fdata){
  cout << "LocalBase:"
       << fdata.getLocalBase(0) << ","
       << fdata.getLocalBase(1) << ","
       << fdata.getLocalBase(2) << "\n";
  cout << "LocalStride:"
       << fdata.getLocalStride(0) << ","
       << fdata.getLocalStride(1) << ","
       << fdata.getLocalStride(2) << "\n";
  cout << "LocalBound:"
       << fdata.getLocalBound(0) << ","
       << fdata.getLocalBound(1) << ","
       << fdata.getLocalBound(2) << "\n";
  cout << "LocalLength:"
       << fdata.getLocalLength(0) << ","
       << fdata.getLocalLength(1) << ","
       << fdata.getLocalLength(2) << "\n";
  cout << "LocalRawBase:"
       << fdata.getLocalRawBase(0) << ","
       << fdata.getLocalRawBase(1) << ","
       << fdata.getLocalRawBase(2) << "\n";
  cout << "LocalRawStride:"
       << fdata.getLocalRawStride(0) << ","
       << fdata.getLocalRawStride(1) << ","
       << fdata.getLocalRawStride(2) << "\n";
  cout << "LocalRawBound:"
       << fdata.getLocalRawBound(0) << ","
       << fdata.getLocalRawBound(1) << ","
       << fdata.getLocalRawBound(2) << "\n";
  cout << "GhostCells:"
       << fdata.getGhostBoundaryWidth(0) << ","
       << fdata.getGhostBoundaryWidth(1) << ","
       << fdata.getGhostBoundaryWidth(2) << "\n";
  cout << "InternalGhost:"
       << fdata.getInternalGhostCellWidth(0) << ","
       << fdata.getInternalGhostCellWidth(0) << ","
       << fdata.getInternalGhostCellWidth(0) << "\n";
  cout << endl;
  cout.flush();
}

void displayInfo( const floatSerialArray &fdata) {
  cout << "Base:"
       << fdata.getBase(0) << ","
       << fdata.getBase(1) << ","
       << fdata.getBase(2) << "\n";
  cout << "Stride:"
       << fdata.getStride(0) << ","
       << fdata.getStride(1) << ","
       << fdata.getStride(2) << "\n";
  cout << "Bound:"
       << fdata.getBound(0) << ","
       << fdata.getBound(1) << ","
       << fdata.getBound(2) << "\n";
  cout << "Length:"
       << fdata.getLength(0) << ","
       << fdata.getLength(1) << ","
       << fdata.getLength(2) << "\n";
  cout << "RawBase:"
       << fdata.getRawBase(0) << ","
       << fdata.getRawBase(1) << ","
       << fdata.getRawBase(2) << "\n";
  cout << "RawStride:"
       << fdata.getRawStride(0) << ","
       << fdata.getRawStride(1) << ","
       << fdata.getRawStride(2) << "\n";
  cout << "RawBound:"
       << fdata.getRawBound(0) << ","
       << fdata.getRawBound(1) << ","
       << fdata.getRawBound(2) << "\n";
  cout << endl;
  cout.flush();
}


void printArray( const floatArray &fdata ) {
  int flushSleep = 100000;
  floatSerialArray ldata = fdata.getLocalArrayWithGhostBoundaries();
  float *xp = ldata.getDataPointer();
  int ii=0;
  int rank = Communication_Manager::My_Process_Number;
  int numProcs = Communication_Manager::Number_Of_Processors;

  for( int p = 0; p < numProcs; p++ ) {
    MPI_Barrier(MPI_COMM_WORLD);
    if ( p == rank ) {
      usleep(flushSleep); 
      cout << flush << "Proc: " << rank+1 << "\n";
      displayInfo(ldata);
      for (int i=0; i<ldata.getLength(2); i++){    
	for (int j=0; j<ldata.getLength(1); j++){
	  cout << "(" 
	       << setfill(' ') << setw(2) << i << "," 
	       << setfill(' ') << setw(2) << j << "," 
	       << setfill(' ') << setw(2) << 0 << "):";
	  for (int k=0; k<ldata.getLength(0); k++){
	    cout << (k==0?" ":",") << setfill(' ') << fixed << setw(9) << setprecision(2) << xp[ ii++ ];
	  }
	  cout << endl << flush;
	}
      }
      cout << endl << flush;
    }
  }
  usleep(flushSleep); 
  MPI_Barrier(MPI_COMM_WORLD);
}
