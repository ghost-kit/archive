#include <cmath>
#include <sys/time.h>

#include <iostream>
#include <sstream>

#include "Io.hpp"
#include "Hdf4.hpp"
#include "Hdf5.hpp"
#include "PHdf5.hpp"

#include <A++.h>
#include <mpi.h>

using namespace std;

string unit;
double runTime=10;
int writeMode=1, weak=1, numProcs=0, numVars=3, rank=0, superSize=1, ghostCells=0;
char *method = "h5";

// Share current time on proc 0 with all processors
long timeSync() {
  long now;
  if (rank == 0) {
    struct timeval time;
    gettimeofday(&time, NULL);
    now = time.tv_sec*1000000 + time.tv_usec;
    MPI_Bcast (&now, 1, MPI_LONG, 0, MPI_COMM_WORLD);
  } else {
    MPI_Bcast (&now, 1, MPI_LONG, 0, MPI_COMM_WORLD);
  }
  return now;
}

// Count how many times an action can be performed within a time constraint
int countS( double & seconds, void func(Io*, string), Io *io, string str ) {
  struct timeval start, end;
  double deltaSeconds;
  int count=0;
  long timeA, timeB;
  stringstream oss;
  timeB = timeA = timeSync();
  
  do {
    // Create a timestamped name if str is a suffix
    if (str[0]=='.') {
      oss.str("");
      oss << "ioBenchmark-" << count << str;
      func(io,oss.str());
    } else {
      func(io,str);
    }
    count++;
    timeB = timeSync();
    deltaSeconds = (timeB-timeA)/1000000.0;
  } while(deltaSeconds<seconds);
  seconds = deltaSeconds;
  return count;
}

// Empty function test
void empty(Io *io, string test) { }

// Sleep for 1/100 of a second
void sleep10k(Io *io, string test) { usleep(10000); }

// Read and write Io wrapper functions
void writeIo(Io *output, string name) { 
  /*
  output->openWrite("testData");
  output->writeVariable(a,"testVar","testData");
  output->writeShape(a,"testData");
  output->close();
  */
}
void readIo(Io *io, string name) { 
  /*
  io->read(name); 
  */
}

// Run a test and display the results
void runTest(Io *io, string type) {
  double seconds = runTime;
  int count;
  if (writeMode) {
    count = ::countS(seconds,writeIo,io,string(".")+type);
  } else {
    //io->write(string("ioBenchmark.")+type);
    count = ::countS(seconds,readIo,io,string("ioBenchmark.")+type);
  }
  if (rank==0) cerr << type << ": \t" << count << " " << unit << "s in " 
		    << seconds << "s\t =  " << count/seconds << " " << unit << "s/s"
		    << ", or  " << seconds/count << " s/" << unit << endl;
  delete io;
}

//  Main: Parse arguments and run Io tests
//  args: [I J K] ["w/s"] ["w/r"] [S]
//         grid    weak    write   seconds
//  
//  argv[1,2,3] grid (integer): size of the grid, default is 20 20 20
//  argv[4] weak (char): 'w' for weak or 's' for strong, default is weak
//  argv[5] write (char): 'w' for write or 'r' for read, default is write
//  argv[6] seconds (float): number of seconds per test, default is 1
//  argv[7] num of variables: number of variables to write, default is 3
//  argv[8] super size: number of processors to superdomain to, default is 1
//  argv[9] ghost cells: number of ghost cells to use, default is 0


int main(int argc, char **argv)
{
  char *localName = "IO Benchmark";

  Optimization_Manager::Initialize_Virtual_Machine(localName,numProcs,argc,argv);
  numProcs = Communication_Manager::Number_Of_Processors;
  rank = Communication_Manager::My_Process_Number;

  int s_i = 20, s_j = 20, s_k = 20, count=0;
  double pow, seconds = 1.0;
  int remain, mult;

  //------------------------------------------------

  if (argc>1 && (strcmp(argv[1],"?")==0 || strcmp(argv[1],"-?")==0)) {
    if (rank==0) {
      cout << argv[0] 
	   << "\n\tni nj nk" 
	   << "\n\t[(w)eak/(s)trong]" 
	   << "\n\t[(w)rite/(r)ead]"
	   << "\n\trunTime numVars superSize ghostCells" << "\n\n";
      cout << "Example: " << argv[0] << " 2 2 2 w w 0 1 1 0\n";
      cout << "         (run benchmark on a 2x2x2 grid in weak write mode,\n" 
	   << "          for 0 seconds (1 call), for 1 variable, to 1 node,\n" 
	   << "          with 0 ghost cells)\n";
      cout << identify(pow) << "\n";
      cout << identify(mult) << "\n";
      cout << identify(localName[0]) << "\n";
    }
    exit(0);
  }

  // Parse arguments
  if (argc>3) {
    s_i = atoi(argv[1]);
    s_j = atoi(argv[2]);
    s_k = atoi(argv[3]);
  }

  if (argc>4) weak = (argv[4][0]=='w'?1:0);
  if (argc>5) writeMode = (argv[5][0]=='w'?1:0);
  if (argc>6) runTime = atof(argv[6]);
  if (argc>7) numVars = atoi(argv[7]);
  if (argc>8) superSize = atoi(argv[8]);
  if (argc>9) ghostCells = atoi(argv[9]);
  if (argc>10) method = argv[10];

  if (writeMode) { unit = "write"; } 
  else { unit = "read"; }

  //------------------------------------------------
  
  // If in weak mode then expand grid size to suite number of procs
  if (weak) {
    pow = ::log((double)numProcs)/::log(2.0);
    remain = (int)::floor(pow) % 3;
    mult = ::floor(pow / 3.0);

    if (::pow(2.0,::floor(pow)) != (double)numProcs) {
      if (Communication_Manager::My_Process_Number == 0)
	cerr << "Processor count must be a power of 2, aborting!\n";
      Optimization_Manager::Exit_Virtual_Machine();
      exit(-1);
    }

    s_i *= ::pow(2.0,(double)(remain > 0 ? mult+1 : mult));
    s_j *= ::pow(2.0,(double)(remain > 1 ? mult+1 : mult));
    s_k *= ::pow(2.0,(double)(mult));
  }

  //------------------------------------------------

  // Sleep for a second to synch output
  MPI_Barrier(MPI_COMM_WORLD);

  seconds = 1;
  count = ::countS(seconds,empty,NULL,"test");
  if (rank==0) cerr << "Test Count: null() * " << count << " = " << seconds << "s" << endl;

  seconds = 1;
  count = ::countS(seconds,sleep10k,NULL,"test");
  if (rank==0) cerr << "          : sleep(.01s) * " << count << " = " << seconds << "s" << endl;

  //------------------------------------------------

  // Display summary of run
  if (rank==0) {
    cerr << "localName: " << localName << " \t numProcs: " << Communication_Manager::Number_Of_Processors << endl;
    for (int i=0; i < argc; i++)
      cerr << "\targv[" << i << "]: " << argv[i] << endl;
    cerr << endl;
    //cerr << pow << " . " << remain << " . " << mult << endl;
    cerr << "Grid size: " << s_i << " x " << s_j << " x " << s_k << endl;
    cerr << "Scaling: " << (weak?"weak":"strong") << endl;
    cerr << "Mode: " << (writeMode?"write":"read") << endl;
    cerr << "Approximate seconds per test: " << runTime << endl;
    cerr << "Number of variables: " << numVars << endl;
    cerr << "Super size: " << superSize << endl;
    cerr << "Ghost Cells: " << ghostCells << endl;
    cerr << "Method: " << method << endl;
    cerr << endl;
  }

  //Grid gridObj( s_i, s_j, s_k, numProcs, numVars, ghostCells, superSize );
 
  if (rank==0) cerr << endl << "Running tests..." << endl << endl;

  //------------------------------------------------
  
  // Perform runs
  if (strcmp(method,"h4")==0) runTest(new Hdf4(superSize),"hdf4");
  if (strcmp(method,"h5")==0) runTest(new Hdf5(superSize),"hdf5");
  if (strcmp(method,"p5")==0) runTest(new PHdf5(superSize),"phdf5");

  //------------------------------------------------

  if (Communication_Manager::My_Process_Number == 0)
    cerr << endl << "Fin." << endl << endl;

  Communication_Manager::Sync();
  Optimization_Manager::Exit_Virtual_Machine();

  return 0;
}
