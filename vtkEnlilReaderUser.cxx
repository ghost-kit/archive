#include "vtkEnlilReaderHeader.h"


//------------------------------------------------------------
//    These methods to load the requested variables.
//  These are provided so that we can abstract out the reading
//  of the data from the rest of the reader.
//
//  Override these methods for your reader
//------------------------------------------------------------


//-- Return 0 for failure, 1 for success --//
int vtkEnlilReader::LoadVariableData(vtkInformationVector* outputVector)
{
  int newExtent[6];

  vtkStructuredGrid* Data = vtkStructuredGrid::GetData(outputVector, 0);
  vtkInformation* fieldInfo = outputVector->GetInformationObject(0);

  int status = this->checkStatus(Data, (char*)"Data Array Structured Grid");

  if(status)
    {
      //get new extent request
      fieldInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), newExtent);

      //check to see if exents have changed
      if(!this->eq(this->SubExtent, newExtent))
        {
          // Set the SubExtents to the NewExtents
          this->setExtents(this->SubExtent, newExtent);

          // The extents have changes, so mark grid dirty.
          this->gridClean = false;
        }

      //set the extents provided to Paraview
      Data->SetExtent(this->SubExtent);

      //Calculate Sub Dimensions
      this->extractDimensions(this->SubDimension, this->SubExtent);

      //Generate the Grid
      this->GenerateGrid();

      //set points and radius
      Data->SetPoints(this->Points);
      Data->GetPointData()->AddArray(this->Radius);

      //Load Variables
      int c = 0;
      double progress = 0.05;

      //Load Cell Data
      for(c = 0; c < this->CellDataArraySelection->GetNumberOfArrays(); c++)
        {
          //Load the current Cell array
          vtkstd::string array = vtkstd::string(this->CellDataArraySelection->GetArrayName(c));
          if(CellDataArraySelection->ArrayIsEnabled(array.c_str()))
            {
              this->LoadArrayValues(array, outputVector);
            }
        }

      //Load Point Data
      for(c=0; c < this->PointDataArraySelection->GetNumberOfArrays(); c++)
        {
          vtkstd::string array = vtkstd::string(this->PointDataArraySelection->GetArrayName(c));

          //Load the current Point array
          if(PointDataArraySelection->ArrayIsEnabled(array.c_str()))
            {
              this->LoadArrayValues(array, outputVector);
              this->SetProgress(progress);
              progress += 0.1;
            }
        }
    }

  return 1;
}

//-- Return 0 for Failure, 1 for Success --//
int vtkEnlilReader::LoadArrayValues(vtkstd::string array, vtkInformationVector* outputVector)
{

  bool vector
      = (this->VectorVariableMap.find(vtkstd::string(array)) != this->VectorVariableMap.end());

  double xyz[3] = {0.0, 0.0, 0.0};

  //get data from system
  vtkStructuredGrid *Data = vtkStructuredGrid::GetData(outputVector,0);

  //set up array to be added
  vtkDoubleArray *DataArray = vtkDoubleArray::New();
  DataArray->SetName(array.c_str());

  if(vector)      //load as a vector
    {
      //need three arrays for vector reads
      double* newArrayR;
      double* newArrayT;
      double* newArrayP;

      //configure DataArray
      DataArray->SetNumberOfComponents(3);  //3-Dim Vector

      //read in the arrays
      newArrayR
          = this->read3dPartialToArray((char*)this->VectorVariableMap[array][0].c_str(), this->SubExtent);

      newArrayT
          = this->read3dPartialToArray((char*)this->VectorVariableMap[array][1].c_str(), this->SubExtent);

      newArrayP
          = this->read3dPartialToArray((char*)this->VectorVariableMap[array][2].c_str(), this->SubExtent);


      // convert from spherical to cartesian
      int loc=0;
      int i,j,k;
      for(k=0; k<this->SubDimension[2]; k++)
        {
          for(j=0; j<this->SubDimension[1]; j++)
            {
              for(i=0; i<this->SubDimension[0]; i++)

                {

                  xyz[0] =newArrayR[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] =newArrayR[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] =newArrayR[loc]*cos(this->sphericalGridCoords[1][j]);

                  xyz[0] += newArrayT[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] += newArrayT[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] += -1.0*newArrayT[loc]*sin(this->sphericalGridCoords[1][j]);

                  xyz[0] += -1.0*newArrayP[loc]*sin(this->sphericalGridCoords[2][k]);
                  xyz[1] += newArrayP[loc]*cos(this->sphericalGridCoords[2][k]);

                  DataArray->InsertNextTuple(xyz);

                  loc++;

                }
            }
        }
      //Add array to grid
      Data->GetPointData()->AddArray(DataArray);
      DataArray->Delete();

      //free temporary memory
      delete [] newArrayR; newArrayR = NULL;
      delete [] newArrayP; newArrayP = NULL;
      delete [] newArrayT; newArrayT = NULL;

    }
  else
    {
      //load as a scalar
      //configure DataArray
      DataArray->SetNumberOfComponents(1);  //Scalar

      //array pointers
      double* newArray;

      //get data array
      newArray
          = this->read3dPartialToArray((char*)this->ScalarVariableMap[array].c_str(), this->SubExtent);

      //insert points
      int k;
      for(k=0; k<this->SubDimension[2]*this->SubDimension[1]*this->SubDimension[0]; k++)
        {
          DataArray->InsertNextValue(newArray[k]);
        }

      //Add array to grid
      Data->GetPointData()->AddArray(DataArray);
      DataArray->Delete();

      //free temporary memory
      delete [] newArray; newArray = NULL;

    }


  return 1;
}

//-- Return 0 for failure, 1 for success --//
/* You will want to over-ride this method to
 * Populate the system with your own arrays */
int vtkEnlilReader::PopulateArrays()
{

  /*Open File and Find array names*/
  NcFile file(this->FileName);

  this->addPointArray((char*)"D");
  this->addPointArray((char*)"DP");
  this->addPointArray((char*)"T");
  this->addPointArray((char*)"BP");
  this->addPointArray((char*)"B1", (char*)"B2", (char*)"B3");
  this->addPointArray((char*)"V1", (char*)"V2", (char*)"V3");

  file.close();
  return 1;
}

//-- Meta Data Population
int vtkEnlilReader::LoadMetaData(vtkInformationVector *outputVector)
{
  int ncFileID = 0;
  int ncSDSID = 0;
  int natts = 0;

  nc_type type;

  char* attname;

  char*    attvalc;
  int     attvali;
  double  attvald;

  vtkStructuredGrid *Data = vtkStructuredGrid::GetData(outputVector,0);
  int status = this->checkStatus(Data, (char*)"(MetaData)Structured Grid Data Object");


  if(status)
    {

      NcFile file(this->FileName);
      natts = file.num_atts();

      for(int q=0; q < natts; q++)
        {
          vtkStringArray *MetaString = vtkStringArray::New();
          vtkIntArray *MetaInt = vtkIntArray::New();
          vtkDoubleArray *MetaDouble = vtkDoubleArray::New();

          attname = (char*)file.get_att(q)->name();
          type = file.get_att(q)->type();

          switch(type)
            {
            case 1:
              break;

            case 2: //text

              attvalc = file.get_att(q)->as_string(0);

              MetaString->SetName(attname);
              MetaString->SetNumberOfComponents(1);
              MetaString->InsertNextValue(attvalc);

              Data->GetFieldData()->AddArray(MetaString);
              MetaString->Delete();
              break;

            case 3:
              break;

            case 4: //int
              attvali = file.get_att(q)->as_int(0);

              MetaInt->SetName(attname);
              MetaInt->SetNumberOfComponents(1);
              MetaInt->InsertNextValue(attvali);

              Data->GetFieldData()->AddArray(MetaInt);
              MetaInt->Delete();

              break;

            case 5:
              break;

            case 6: //double
              attvald = file.get_att(q)->as_double(0);

              MetaDouble->SetName(attname);
              MetaDouble->SetNumberOfComponents(1);
              MetaDouble->InsertNextValue(attvald);

              Data->GetFieldData()->AddArray(MetaDouble);
              MetaDouble->Delete();
              break;
            }
        }

      file.close();
    }

  return 1;
}


void vtkEnlilReader::addPointArray(char* name)
{
  NcFile file(this->FileName);
  try
  {
    // look up the "Long Name" of the variable
    vtkstd::string varname = file.get_var(name)->get_att("long_name")->as_string(0);
    this->ScalarVariableMap[varname] = vtkstd::string(name);

    // Add it to the point grid
    this->PointDataArraySelection->AddArray(varname.c_str());
  }
  catch (...)
  {
    std::cerr << "Failed to retrieve variable " << name
              << ". Verify variable name." << std::endl;

    file.close();
    return;
  }

  file.close();
}

void vtkEnlilReader::addPointArray(char* name1, char* name2, char* name3)
{
  NcFile file(this->FileName);
  try
  {
    //get the long name of the first variable in the vector
    vtkstd::string varname1 = file.get_var(name1)->get_att("long_name")->as_string(0);

    //remove the vector component of the name
    size_t pos = varname1.find("-");
    vtkstd::string varname2 = varname1.substr(pos+1);

    //ensure that first work is capitalized
    varname2[0] = toupper((unsigned char) varname2[0]);

    //add components of vector to vector map
    vtkstd::vector<vtkstd::string> nameArray;
    nameArray.push_back(name1);
    nameArray.push_back(name2);
    nameArray.push_back(name3);
    this->VectorVariableMap[varname2] = nameArray;

    //add array to point array name list
    this->PointDataArraySelection->AddArray(varname2.c_str());




  }
  catch(...)
  {
    std::cerr << "Failed to retrieve variable "
              << name1 << " or " << name2 << " or " << name3
              << ". Verify variable names." << std::endl;

    file.close();
    return;
  }
  file.close();
}

//-- Return 0 for failure, 1 for success --//
/* You will need to over-ride this method to provide
 * your own grid-information */
int vtkEnlilReader::GenerateGrid()
{

  int i = 0;
  int j = 0;
  int k = 0;

  const int GridScale = this->GetGridScaleType();

  double *X1;
  double *X2;
  double *X3;

  int X1_extents[2] = {this->SubExtent[0], this->SubExtent[1]};
  int X2_extents[2] = {this->SubExtent[2], this->SubExtent[3]};
  int X3_extents[2] = {this->SubExtent[4], this->SubExtent[5]};

  //build the grid if it is dirty (modified in application)
  if(!this->gridClean)
    {
      if(this->Points != NULL)
        {
          this->Points->Delete();
          this->Radius->Delete();
          this->sphericalGridCoords.clear();
        }

      //build the Grid
      this->Points = vtkPoints::New();

      //build the Radius Array
      this->Radius = vtkDoubleArray::New();
      this->Radius->SetName("Radius");
      this->Radius->SetNumberOfComponents(1);

      // read data from file
      X1 = this->readGridPartialToArray((char*)"X1", X1_extents, false);
      X2 = this->readGridPartialToArray((char*)"X2", X2_extents, false);
      X3 = this->readGridPartialToArray((char*)"X3", X3_extents, true);

      // Populate the Spherical Grid Coordinates (to be used in calcs later)
      vtkstd::vector<double> R(X1, X1 + this->SubDimension[0]);
      vtkstd::vector<double> T(X2, X2 + this->SubDimension[1]);
      vtkstd::vector<double> P(X3, X3 + this->SubDimension[2]);

      this->sphericalGridCoords.push_back(R);
      this->sphericalGridCoords.push_back(T);
      this->sphericalGridCoords.push_back(P);

      // Generate the grid based on the R-P-T coordinate system.
      double xyz[3] = { 0, 0, 0 };
      for (k = 0; k < this->SubDimension[2]; k++)
        {
          for (j = 0; j < this->SubDimension[1]; j++)
            {
              for (i = 0; i < this->SubDimension[0]; i++)
                {
                  xyz[0] = (X1[i] * sin(X2[j]) * cos(X3[k]))
                      / GRID_SCALE::ScaleFactor[GridScale];
                  xyz[1] = (X1[i] * sin(X2[j]) * sin(X3[k]))
                      / GRID_SCALE::ScaleFactor[GridScale];
                  xyz[2] = (X1[i] * cos(X2[j]))
                      / GRID_SCALE::ScaleFactor[GridScale];

                  //insert point information into the grid
                  this->Points->InsertNextPoint(xyz);

                  // insert radius value into radius array.
                  // Scaled by grid scale factor
                  this->Radius->InsertNextValue(
                        X1[i] / GRID_SCALE::ScaleFactor[GridScale]);
                }
            }
        }

      //grid just created, so clean by definition.
      this->gridClean=true;
    }
  return 1;
}


//-- Return 0 for failure, 1 for success --//
/* Over-ride this method to provide the
 * extents of your data */
int vtkEnlilReader::PopulateDataInformation()
{

  NcFile data(this->FileName);
  NcDim* dims_x = data.get_dim(0);
  NcDim* dims_y = data.get_dim(1);
  NcDim* dims_z = data.get_dim(2);

  NcVar* time = data.get_var("TIME");

  this->Dimension[0] = (int)dims_x->size();
  this->Dimension[1] = (int)dims_y->size();
  this->Dimension[2] = (int)dims_z->size()+1;

  int Time = time->as_double(0);

  data.close();

  //Populate Extents
  this->setExtents(this->WholeExtent,
                   0, this->Dimension[0]-1,
                   0, this->Dimension[1]-1,
                   0, this->Dimension[2]-1);

  //Set Time step Information
  this->NumberOfTimeSteps = 1;
  this->TimeSteps = new double[this->NumberOfTimeSteps];
  this->TimeSteps[0] = Time;

  //We just populated info, so we are clean
  this->infoClean = true;

  return 1;
}


//=================== END USER METHODS =========================

