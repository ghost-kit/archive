
/*
  The purpose of this file is to provide an empty implementation of mpi.h,
  or any associated functions which require MPI, for libIoApp.
*/

#ifndef MPI_NULL_H__
#define MPI_NULL_H__

#define MPI_INCLUDED
#define MPIO_INCLUDE

#define NOH5MPIO

enum MPI_CONSTS { MPI_COMM_WORLD,  MPI_INT, MPI_LOR, MPI_INFO_NULL };

#ifndef MPI_Comm
typedef int MPI_Comm;
#endif
#ifndef MPI_Group
typedef int MPI_Group;
#endif

int MPI_Bcast( ... );
int MPI_Allreduce( ... );
int MPI_Abort( ... );
int MPI_Comm_create( ... );
int MPI_Comm_group( ... );
int MPI_Group_range_incl( ... );
int MPI_Group_create( ... );
int MPI_Group_free( ... );

int H5Pset_fapl_mpio( ... );
int H5Pset_dxpl_mpio( ... );

#endif //MPI_NULL_H__
