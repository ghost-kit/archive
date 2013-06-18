#ifndef VTKSPACECRAFTINFOFILTER_H
#define VTKSPACECRAFTINFOFILTER_H

#include "vtkSpaceCraftInfo.h"
#include "vtkTableAlgorithm.h"

class vtkSpaceCraftInfoFilter : public vtkTableAlgorithm, vtkSpaceCraftInfoHandler
{
public:

    static vtkSpaceCraftInfoFilter *New();
    vtkTypeMacro(vtkSpaceCraftInfoFilter, vtkTableAlgorithm)
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkGetMacro(NumberOfTimeSteps, int);

protected:
    vtkSpaceCraftInfoFilter();
    ~vtkSpaceCraftInfoFilter();

    int RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
    int RequestData(vtkInformation *request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector);
    int FillInputPortInformation(int port, vtkInformation *info);
    int FillOutputPortInformation(int port, vtkInformation *info);

private:
    vtkSpaceCraftInfoFilter(const vtkSpaceCraftInfoFilter&);
    void operator =(const vtkSpaceCraftInfoFilter&);
};

#endif // VTKSPACECRAFTINFOFILTER_H
