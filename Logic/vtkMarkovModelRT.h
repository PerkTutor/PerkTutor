
#ifndef MARKOVMODELRT_H
#define MARKOVMODELRT_H

#include "RecordType.h"
#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vtkMarkovModel.h"

#include <vector>
#include <iostream>

// Class representing a procedure comprised of tracking records
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkMarkovModelRT : public vtkMarkovModel
{
public:

  static vtkMarkovModelRT *New();

  MarkovRecord CalculateStateRT( MarkovRecord element );

public:

  vtkMarkovModelRT();
  vtkMarkovModelRT( int numInitStates, int numInitSymbols );
  ~vtkMarkovModelRT();

protected:

  std::vector<MarkovRecord> sequence;
  LabelRecord currDelta;
  LabelRecord currPsi;

};

#endif
