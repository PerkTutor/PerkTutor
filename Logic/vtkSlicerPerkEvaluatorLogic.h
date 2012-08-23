
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

#include "vtkTransformTimeSeries.h"



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
  
  void ImportFile( std::string fileName );
  
  double GetTotalTime() const;
  double GetMinTime() const;
  double GetMaxTime() const;
  double GetPlaybackTime() const;
  
  void SetPlaybackTime( double time );
  void SetMarkBegin( double begin );
  void SetMarkEnd( double end );
  
  
  typedef std::pair< double, std::string > AnnotationType;
  typedef std::vector< AnnotationType > AnnotationVectorType;
  
  AnnotationVectorType GetAnnotations();
  
  
  typedef std::pair< std::string, double > MetricType;
  typedef std::vector< MetricType > MetricVectorType;
  
  void Analyse();
  MetricVectorType GetMetrics();
  
  
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
  
  
protected:
  
  vtkSlicerPerkEvaluatorLogic();
  virtual ~vtkSlicerPerkEvaluatorLogic();
  
  virtual void SetMRMLSceneInternal( vtkMRMLScene* newScene );
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded( vtkMRMLNode* node );
  virtual void OnMRMLSceneNodeRemoved( vtkMRMLNode* node );
  

private:

  vtkSlicerPerkEvaluatorLogic(const vtkSlicerPerkEvaluatorLogic&); // Not implemented
  void operator=(const vtkSlicerPerkEvaluatorLogic&);               // Not implemented
  
  void ClearData();
  double GetTimestampFromElement( vtkXMLDataElement* element );
  vtkTransformTimeSeries* UpdateToolList( std::string name );
  void CreateTransformNodes();
  
  void AnalyseTrajectory( vtkTransformTimeSeries* Trajectory );
  void AnalyseNeedle( vtkMRMLLinearTransformNode* tnode );
  double SpanArea( double* E0, double* E1, double* T0, double* T1 );
  
  typedef std::vector< vtkSmartPointer< vtkTransformTimeSeries > > TrajectoryContainerType;
  TrajectoryContainerType ToolTrajectories;
  
  AnnotationVectorType Annotations;
  
  double PlaybackTime;
  double MarkBegin;
  double MarkEnd;
  
  MetricVectorType Metrics;
  
};

#endif
