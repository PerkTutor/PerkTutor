
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

// STD includes
#include <cstdlib>

#include "vtkSmartPointer.h"
#include "vtkXMLDataParser.h"

#include "vtkSlicerPerkEvaluatorModuleLogicExport.h"
#include "vtkSlicerTransformRecorderLogic.h"


struct ToolTrajectory
{
  vtkMRMLTransformBufferNode* Buffer;
  vtkMRMLLinearTransformNode* Node;
};


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
  
  void UpdateToolTrajectories( vtkMRMLTransformBufferNode* bufferNode );
  vtkMRMLTransformBufferNode* GetTransformBuffer();
  vtkMRMLTransformBufferNode* GetSelfAndParentTransformBuffer( vtkMRMLLinearTransformNode* transform );

  std::string GetTransformRole( std::string transformNodeName );
  void SetTransformRole( std::string transformNodeName, std::string newTransformRole );
  std::vector< std::string > GetAllTransformRoles();

  std::string GetAnatomyNodeName( std::string anatomyRole );
  void SetAnatomyNodeName( std::string anatomyRole, std::string newAnatomyNodeName );
  std::vector< std::string > GetAllAnatomyRoles();

  void GetSceneVisibleTransformNodes( vtkCollection* visibleTransformNodes );
  void GetSceneVisibleAnatomyNodes( vtkCollection* visibleAnatomyNodes );
  
  double GetTotalTime() const;
  double GetMinTime() const;
  double GetMaxTime() const;
  double GetPlaybackTime() const;
  
  void SetPlaybackTime( double time );
  void SetMarkBegin( double begin );
  double GetMarkBegin();
  void SetMarkEnd( double end );
  double GetMarkEnd();
  void SetNeedleBase( double x, double y, double z );

  typedef std::pair<std::string,double> MetricType;  
  std::vector<MetricType> GetMetrics();

  void SetMetricsDirectory( std::string newDirectory );
  std::string GetMetricsDirectory();

  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;  
  
private:

  vtkSlicerPerkEvaluatorLogic(const vtkSlicerPerkEvaluatorLogic&); // Not implemented
  void operator=(const vtkSlicerPerkEvaluatorLogic&);               // Not implemented

private:
  
  void ClearData();

  std::vector< ToolTrajectory > ToolTrajectories;
  
  vtkMRMLTransformBufferNode* TransformBuffer;
  std::map< std::string, std::string > TransformRoleMap;
  std::map< std::string, std::string > AnatomyNodeMap;
  
  double PlaybackTime;
  double MarkBegin;
  double MarkEnd;
  double NeedleBase[4];

  std::string MetricsDirectory;

};

const double NEEDLE_LENGTH = 300; // Assume 300mm

#endif
