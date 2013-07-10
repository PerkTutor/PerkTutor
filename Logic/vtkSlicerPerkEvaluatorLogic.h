
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

// STD includes
#include <cstdlib>

#include "vtkSmartPointer.h"
#include "vtkXMLDataParser.h"

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
  
  void UpdateToolTrajectories( vtkMRMLTransformBufferNode* bufferNode );
  
  double GetTotalTime() const;
  double GetMinTime() const;
  double GetMaxTime() const;
  double GetPlaybackTime() const;
  
  void SetPlaybackTime( double time );
  void SetMarkBegin( double begin );
  void SetMarkEnd( double end );
  void SetNeedleBase( double x, double y, double z );

  typedef std::pair<std::string,double> MetricType;  
  std::vector<MetricType> GetMetrics();

  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;

  vtkXMLDataParser* Parser;
  vtkXMLDataElement* ParseXMLFile( std::string fileName );

  
  // Reference to body model node.  
public:
  vtkGetObjectMacro( BodyModelNode, vtkMRMLModelNode );
  void SetBodyModelNode( vtkMRMLModelNode* node );
private:
  vtkMRMLModelNode* BodyModelNode;
  
  
  // Reference to the needle coordinate system.
public:
  vtkGetObjectMacro( NeedleTransformNode, vtkMRMLLinearTransformNode );
  void SetNeedleTransformNode( vtkMRMLLinearTransformNode* node );
private:
  vtkMRMLLinearTransformNode* NeedleTransformNode;
  
  
private:

  vtkSlicerPerkEvaluatorLogic(const vtkSlicerPerkEvaluatorLogic&); // Not implemented
  void operator=(const vtkSlicerPerkEvaluatorLogic&);               // Not implemented

private:
  
  void ClearData();
  double GetTimestampFromElement( vtkXMLDataElement* element );
  
  std::vector<MetricType> CalculateToolMetrics( vtkMRMLTransformBufferNode* Trajectory );
  std::vector<MetricType> CalculateNeedleMetrics();
  double TriangleArea( double* p1, double* p2, double* p3 );
  
  std::vector<vtkMRMLTransformBufferNode*> ToolTrajectories;
  
  double PlaybackTime;
  double MarkBegin;
  double MarkEnd;
  double NeedleBase[4];

};

const double NEEDLE_LENGTH = 300; // Assume 300mm

#endif
