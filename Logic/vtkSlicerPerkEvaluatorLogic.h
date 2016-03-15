
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
#include "vtkMRMLMetricScriptNode.h"
#include "vtkMRMLMetricScriptStorageNode.h"
#include "vtkMRMLMetricInstanceNode.h"


// STD includes
#include <cstdlib>

#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"

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

  qSlicerPythonManager* PythonManager;

public:
  
  bool IsSelfOrDescendentTransformNode( vtkMRMLLinearTransformNode* parent, vtkMRMLLinearTransformNode* child );

  void GetSelfAndParentRecordBuffer( vtkMRMLPerkEvaluatorNode* peNode, vtkMRMLLinearTransformNode* transform, vtkLogRecordBuffer* selfParentRecordBuffer );
  void GetSelfAndParentTimes( vtkMRMLPerkEvaluatorNode* peNode, vtkMRMLLinearTransformNode* transform, vtkDoubleArray* timesArray );

  std::vector< std::string > GetAllRoles( std::string msNodeID, vtkMRMLMetricInstanceNode::RoleTypeEnum roleType );
  std::string GetAnatomyRoleClassName( std::string msNodeID, std::string role );
  std::string GetContext( std::string msNodeID );

  void GetSceneVisibleTransformNodes( vtkCollection* visibleTransformNodes );

  void UpdateSceneToPlaybackTime( vtkMRMLPerkEvaluatorNode* peNode );

  double GetRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode );
  void SetRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode, double time );
  double GetMaximumRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode );

  void ComputeMetrics( vtkMRMLPerkEvaluatorNode* peNode );

  void SetupRealTimeProcessing( vtkMRMLPerkEvaluatorNode* peNode );

  void ObserveGlobalMetricInstances( vtkMRMLPerkEvaluatorNode* peNode );
  void SetMetricInstancesRolesToID( vtkMRMLPerkEvaluatorNode* peNode, std::string nodeID, std::string role, vtkMRMLMetricInstanceNode::RoleTypeEnum roleType );
  void UpdateGlobalMetrics( vtkMRMLLinearTransformNode* transformNode );
  void UpdateGlobalMetrics( vtkMRMLMetricScriptNode* msNode );
  void UpdateLocalMetrics( vtkMRMLPerkEvaluatorNode* peNode );

  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );
  void ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData );

  
private:

  vtkSlicerPerkEvaluatorLogic(const vtkSlicerPerkEvaluatorLogic&); // Not implemented
  void operator=(const vtkSlicerPerkEvaluatorLogic&);               // Not implemented


};


#endif
