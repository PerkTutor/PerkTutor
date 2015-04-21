
// .NAME vtkSlicerPerkEvaluatorLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerPerkEvaluatorLogic_h
#define __vtkSlicerPerkEvaluatorLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLPerkEvaluatorNode.h"


// STD includes
#include <cstdlib>

#include "vtkSmartPointer.h"
#include "vtkXMLDataParser.h"
#include "vtkDoubleArray.h"

#include "vtkSlicerPerkEvaluatorModuleLogicExport.h"
#include "vtkSlicerTransformRecorderLogic.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_PERKEVALUATOR_MODULE_LOGIC_EXPORT
vtkSlicerPerkEvaluatorLogic
 : public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPerkEvaluatorLogic *New();
  vtkTypeMacro(vtkSlicerPerkEvaluatorLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void OnMRMLSceneEndClose();

protected:
  
  vtkSlicerPerkEvaluatorLogic();
  virtual ~vtkSlicerPerkEvaluatorLogic();
  
  // Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void SetMRMLSceneInternal( vtkMRMLScene* newScene );
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded( vtkMRMLNode* node );
  virtual void OnMRMLSceneNodeRemoved( vtkMRMLNode* node );

public:

  // TODO: THIS SHOULD BE REMOVED WHEN vtkMRMLTableNode is properly added to Slicer
  vtkMRMLTableNode* AddTable(const char* fileName, const char* name = 0);
  
  void GetSelfAndParentRecordBuffer( vtkMRMLPerkEvaluatorNode* peNode, vtkMRMLLinearTransformNode* transform, vtkLogRecordBuffer* selfParentRecordBuffer );
  void GetSelfAndParentTimes( vtkMRMLPerkEvaluatorNode* peNode, vtkMRMLLinearTransformNode* transform, vtkDoubleArray* timesArray );

  std::vector< std::string > GetAllTransformRoles( vtkMRMLPerkEvaluatorNode* peNode );
  std::vector< std::string > GetAllAnatomyRoles( vtkMRMLPerkEvaluatorNode* peNode );

  void GetSceneVisibleTransformNodes( vtkCollection* visibleTransformNodes );
  void GetSceneVisibleAnatomyNodes( vtkCollection* visibleAnatomyNodes );

  void UpdateSceneToPlaybackTime( vtkMRMLPerkEvaluatorNode* peNode );

  double GetRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode );
  void SetRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode, double time );
  double GetMaximumRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode );

  void ComputeMetrics( vtkMRMLPerkEvaluatorNode* peNode );

  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;  
  
private:

  vtkSlicerPerkEvaluatorLogic(const vtkSlicerPerkEvaluatorLogic&); // Not implemented
  void operator=(const vtkSlicerPerkEvaluatorLogic&);               // Not implemented


};


#endif
