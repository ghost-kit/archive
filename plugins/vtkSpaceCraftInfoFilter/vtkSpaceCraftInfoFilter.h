#ifndef VTKSPACECRAFTINFOFILTER_H
#define VTKSPACECRAFTINFOFILTER_H

#include "vtkSpaceCraftInfo.h"
#include "vtkTableAlgorithm.h"
#include "vtkMultiBlockDataSetAlgorithm.h"

class VTKFILTERSEXTRACTION_EXPORT vtkSpaceCraftInfoFilter : public vtkMultiBlockDataSetAlgorithm
{

public:

    static vtkSpaceCraftInfoFilter *New();
    vtkTypeMacro(vtkSpaceCraftInfoFilter, vtkMultiBlockDataSetAlgorithm)
    void PrintSelf(ostream& os, vtkIndent indent);

    double getStartTime();
    double getEndTime();
    void SetSCIData(const char *group, const char *observatory, const char *list);
    void SetTimeFitHandler(int handler);
    void SetBadDataHandler(int handler);
    void setTempPath(const char *path);


protected:
    vtkSpaceCraftInfoFilter();
    ~vtkSpaceCraftInfoFilter();

    int ProcessRequest(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
    int RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
    int RequestData(vtkInformation *request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector);
    int FillInputPortInformation(int port, vtkInformation *info);
    int FillOutputPortInformation(int port, vtkInformation *info);

    //handler
    vtkSpaceCraftInfoHandler infoHandler;

private:
    vtkSpaceCraftInfoFilter(const vtkSpaceCraftInfoFilter&);
    void operator =(const vtkSpaceCraftInfoFilter&);
};

#endif // VTKSPACECRAFTINFOFILTER_H
