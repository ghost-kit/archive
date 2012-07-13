#ifndef __vtkENLILReader_H__
#define __vtkENLILReader_H__

#include "generic/vtkGenericReader.h"
#include "vtkStructuredGridAlgorithm.h"

typedef vtkGenericReader Superclass;

namespace GRID_SCALE
{
  enum ScaleType{
    NONE   = 0,
    REARTH = 1,
    RSOLAR = 2,
    AU     = 3
  };
  static const float ScaleFactor[4] = { 1.0,
                                        6.5e6,
                                        6.955e8,
                                        1.5e11 };
}

class VTK_EXPORT vtkENLILReader : public vtkGenericReader
{
public:
  static vtkENLILReader *New();
  vtkTypeMacro(vtkENLILReader, vtkGenericReader)

  vtkENLILReader();
  ~vtkENLILReader();

private:
  vtkENLILReader(const vtkENLILReader&); // Not implemented
  void operator=(const vtkENLILReader&); // Not implemented
};



#endif


