#ifndef VTKSPACECRAFTINFOSOURCE_H
#define VTKSPACECRAFTINFOSOURCE_H

#include "vtkSpaceCraftInfo.h"
#include "vtkTableAlgorithm.h"

class  VTKFILTERSEXTRACTION_EXPORT vtkSpaceCraftInfoSource : public vtkTableAlgorithm, vtkSpaceCraftInfoHandler
{
    typedef Superclass vtkSpaceCraftInfoHandler;
public:
    static vtkSpaceCraftInfoSource *New();
    vtkTypeMacro(vtkSpaceCraftInfoSource, vtkTableAlgorithm)
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkGetMacro(NumberOfTimeSteps, int);

protected:
    vtkSpaceCraftInfoSource();
    ~vtkSpaceCraftInfoSource();

    int RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
    int RequestData(vtkInformation *request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector);
    int FillInputPortInformation(int port, vtkInformation *info);
    int FillOutputPortInformation(int port, vtkInformation *info);

private:
    vtkSpaceCraftInfoSource(const vtkSpaceCraftInfoSource&);
    void operator =(const vtkSpaceCraftInfoSource&);

};

#endif // VTKSPACECRAFTINFOSOURCE_H
