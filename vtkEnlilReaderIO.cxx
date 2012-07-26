#include"vtkEnlilReaderHeader.h"


//-- returns array read via partial IO limited by extents --//
/* This method will automatically adjust for the periodic boundary
 *  condition that does not exist sequentially in file */
double* vtkEnlilReader::read3dPartialToArray(char* arrayName, int extents[])
{
  int extDims[3] = {0,0,0};
  size_t readDims[4]   = {1,1,1,1};
  long readStart[4]  = {0,extents[4],extents[2],extents[0]};

  // get dimensions from extents
  this->extractDimensions(extDims, extents);

  // Enlil encodes in reverse, so reverse the order, add fourth dimension 1st.
  readDims[1] = extDims[2];
  readDims[2] = extDims[1];
  readDims[3] = extDims[0];

  //find all conditions that need to be accounted for
  bool periodic = false;
  bool periodicRead = false;
  bool periodicOnly = false;

  if(extents[5] == this->WholeExtent[5])
    {
      periodic = true;
      if(extents[4] > 0)
        {
          periodicRead = true;
          if(extents[4] == this->WholeExtent[5])
            {
              periodicOnly = true;
            }
        }
    }

  // allocate memory for complete array
  double *array = new double[extDims[0]*extDims[1]*extDims[2]];

  //open file
  NcFile file(this->FileName);
  NcVar* variable = file.get_var(arrayName);

  // start to read in data
  if(periodic && !periodicOnly)
    {
      //adjust dims
      readDims[1] = readDims[1]-1;

      //adjust the start point
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else if(periodicOnly)
    {
      //set periodic only
      readDims[1] = 1;
      readStart[1] = 0;
      readStart[2] = 0;
      readStart[3] = 0;

      //set read location
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else
    {
      //set read location as stated
      variable->set_cur(readStart);

      //read as stated
      variable->get(array, readDims);

    }

  //fix periodic boundary if necesary
  if(periodic && !periodicRead && !periodicOnly)
    {
      //copy periodic data from begining to end
      size_t wedgeSize = (extDims[0]*extDims[1]);
      size_t wedgeLoc  = (extDims[0]*extDims[1])*(extDims[2]-1);

      for(int x = 0; x < wedgeSize; x++)
        {
          //copy the wedge
          array[wedgeLoc] = array[x];

          //advance index
          wedgeLoc++;
        }

    }
  else if (periodic && periodicRead && !periodicOnly)
    {
      //read in periodic data and place at end of array
      size_t wedgeSize = extDims[0]*extDims[1];
      size_t wedgeLoc  = (extDims[0]*extDims[1])*(extDims[2]-1);

      double * wedge = new double[wedgeSize];

      //start at 0,0,0
      readStart[1] = 0;
      readStart[2] = 0;
      readStart[3] = 0;

      //restrict to phi = 1 dimension
      readDims[1] = 1;

      //set start
      variable->set_cur(readStart);

      //read data
      variable->get(wedge, readDims);

      //populate wedge to array
      for(int x = 0; x < wedgeSize; x++)
        {
          //copy the wedge
          array[wedgeLoc] = wedge[x];

          //advance index
          wedgeLoc++;
        }

      //free temp memory
      delete [] wedge; wedge = NULL;
    }


  //close file
  file.close();

  return array;

}

//-- returns array read via partial IO limited by extents --//
/* This method will automatically adjust for the periodic boundary
 *  condition that does not exist sequentially in file */
double* vtkEnlilReader::readGridPartialToArray(char *arrayName, int subExtents[], bool isPeriodic = false)
{
  int     extDim = subExtents[1]-subExtents[0]+1;;
  size_t  readDims[2]  = {1,extDim};
  long    readStart[2] = {0,subExtents[0]};

  //Find conditions that need to be handled
  bool periodic = false;
  bool periodicRead = false;
  bool periodicOnly = false;

  //if isPeriodic is set, then we are looking at phi
  if(isPeriodic)
    {
      if(subExtents[1] == this->WholeExtent[5])
        {
          periodic = true;
          if(subExtents[0] > 0)
            {
              periodicRead = true;
              if(subExtents[0] == this->WholeExtent[5])
                {
                  periodicOnly = true;
                }
            }
        }
    }

  //allocate Memory for complete array
  double *array = new double[extDim];

  //Open file
  NcFile file(this->FileName);
  NcVar* variable = file.get_var(arrayName);

  //start to read in data
  if(periodic && !periodicOnly)
    {

      //adjust dims
      readDims[1] = readDims[1]-1;

      //adjust the start point
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else if(periodicOnly)
    {

      //set periodic only
      readDims[1] = 1;
      readStart[1] = 0;

      //set read location
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else
    {
      //set read location as stated
      variable->set_cur(readStart);

      //read as stated
      variable->get(array, readDims);
    }

  //fix periodic boundary if necesary
  if(periodic && !periodicRead && !periodicOnly)
    {

      //copy periodic data from begining to end
      array[extDim-1] = array[0];

    }
  else if (periodic && periodicRead && !periodicOnly)
    {

      //read in periodic data and place at end of array
      size_t wedgeSize = 1;
      size_t wedgeLoc  = (extDim-1);

      double * wedge = new double[wedgeSize];

      //start at 0,0,0
      readStart[1] = 0;

      //restrict to phi = 1 dimension
      readDims[1] = 1;

      //set start
      variable->set_cur(readStart);

      //read data
      variable->get(wedge, readDims);

      //populate wedge to array

      array[wedgeLoc] = wedge[0];

      //free temp memory
      delete [] wedge; wedge = NULL;
    }

  //close the file
  file.close();

  //return completed array
  return array;
}

