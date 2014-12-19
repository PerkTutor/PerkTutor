
// PerkEvaluator Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <ctime>
#include <iostream>
#include <limits>
#include <sstream>

#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"


// Helper functions ------------------------------------------------------------------------


double*
MatrixStrToDouble( std::string str )
{
  std::stringstream ss( str );
  
  double e00; ss >> e00; double e01; ss >> e01; double e02; ss >> e02; double e03; ss >> e03;
  double e10; ss >> e10; double e11; ss >> e11; double e12; ss >> e12; double e13; ss >> e13;
  double e20; ss >> e20; double e21; ss >> e21; double e22; ss >> e22; double e23; ss >> e23;
  double e30; ss >> e30; double e31; ss >> e31; double e32; ss >> e32; double e33; ss >> e33;

  double* dmat = new double[16];

  dmat[0] = e00;
  dmat[1] = e01;
  dmat[2] = e02;
  dmat[3] = e03;

  dmat[4] = e10;
  dmat[5] = e11;
  dmat[6] = e12;
  dmat[7] = e13;

  dmat[8] = e20;
  dmat[9] = e21;
  dmat[10] = e22;
  dmat[11] = e23;

  dmat[12] = e30;
  dmat[13] = e31;
  dmat[14] = e32;
  dmat[15] = e33;
  
  return dmat;
}


//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkSlicerPerkEvaluatorLogic );


// Constructors and Desctructors ----------------------------------------------

vtkSlicerPerkEvaluatorLogic
::vtkSlicerPerkEvaluatorLogic()
{
  this->PlaybackTime = 0.0;

  this->TransformRecorderLogic = NULL;
}



vtkSlicerPerkEvaluatorLogic::
~vtkSlicerPerkEvaluatorLogic()
{
  this->ClearData();
}


void vtkSlicerPerkEvaluatorLogic
::ClearData()
{
  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {
    this->ToolTrajectories.at(i).Buffer->Delete();
  }
  this->ToolTrajectories.clear();
}



// Slicer functions ---------------------------------------------------------------

void vtkSlicerPerkEvaluatorLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "vtkSlicerPerkEvaluatorLogic: " << this->GetClassName() << "\n";
}



void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneEndClose()
{
  this->ClearData();
}


void vtkSlicerPerkEvaluatorLogic
::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}


void vtkSlicerPerkEvaluatorLogic
::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::New();
  this->GetMRMLScene()->RegisterNodeClass( peNode );
  peNode->Delete();
}


void vtkSlicerPerkEvaluatorLogic
::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeAdded( vtkMRMLNode* vtkNotUsed( node ) )
{
}


void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeRemoved( vtkMRMLNode* vtkNotUsed( node ) )
{
}



// -----------------------------------------------------------------------------------


std::vector<vtkSlicerPerkEvaluatorLogic::MetricType> vtkSlicerPerkEvaluatorLogic
::GetMetrics( vtkMRMLPerkEvaluatorNode* peNode )
{
  std::vector<MetricType> metrics;

  // Check conditions
  if ( peNode == NULL )
  {
    return metrics;
  }
  if ( peNode->GetMarkBegin() >= peNode->GetMarkEnd() ) // Is a test for to see if the MarkBegin and MarkEnd are within the bounds of the procedure time really necessary?
  {
    return metrics;
  }

  // Use the python metrics calculator module
  qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();
  pythonManager->executeString( "import PythonMetricsCalculator" );
  pythonManager->executeString( "PythonMetricsCalculatorLogic = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogic.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( "PythonMetricsVariable = PythonMetricsCalculatorLogic.CalculateAllMetrics()" );
  QVariant result = pythonManager->getVariable( "PythonMetricsVariable" );
  QStringList pythonMetrics = result.toStringList();

  int i = 0;
  while ( i < pythonMetrics.length() )
  {
    MetricType currentPythonMetric;
    currentPythonMetric.first = pythonMetrics.at( i ).toStdString();
    currentPythonMetric.second = atof( pythonMetrics.at( i + 1 ).toStdString().c_str() );
    metrics.push_back( currentPythonMetric );
    i = i + 2;
  }


  return metrics;
}


//Read XML file that was written by TransformRecorder module.
void vtkSlicerPerkEvaluatorLogic
::UpdateToolTrajectories( vtkMRMLTransformBufferNode* bufferNode )
{
  this->ClearData();
  // The import function from the Transform Recorder Logic will automatically add the transform nodes to the scene
  if ( bufferNode == NULL )
  {
    return;
  }

  std::vector< vtkMRMLTransformBufferNode* > toolBuffers = bufferNode->SplitBufferByName();

  for ( int i = 0; i < toolBuffers.size(); i++ )
  {	
    std::string toolName = toolBuffers.at(i)->GetCurrentTransform()->GetDeviceName();

    vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( toolName.c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( node == NULL )
    {
      node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) );
	    this->GetMRMLScene()->AddNode( node );
	    node->SetScene( this->GetMRMLScene() );
	    node->SetName( toolName.c_str() );
    }

    // TODO: This may not be optimal
    // Only set all of the nodes in the buffer to have role "Any" when the PerkEvaluator node is created?
    // this->SetTransformRole( node->GetName(), "Any" );

    // Add to the tool trajectories
    ToolTrajectory currentTrajectory;
    currentTrajectory.Node = node;
    currentTrajectory.Buffer = toolBuffers.at(i);
    this->ToolTrajectories.push_back( currentTrajectory );

  }

}



vtkMRMLTransformBufferNode* vtkSlicerPerkEvaluatorLogic
::GetSelfAndParentTransformBuffer( vtkMRMLLinearTransformNode* transformNode )
{
  // Iterate through the parents and add to temporary transform buffer if in the selected transform buffer for analysis
  vtkMRMLTransformBufferNode* selfParentBuffer = vtkMRMLTransformBufferNode::New();

  vtkMRMLLinearTransformNode* parent = transformNode;
  while( parent != NULL )
  {
    // Check if the parent's name matches one of the trajectory names
    for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
    {
      if ( this->ToolTrajectories.at(i).Buffer->GetCurrentTransform()->GetDeviceName().compare( parent->GetName() ) == 0 )
	    {
        // Concatenate into the transform if so
        vtkMRMLTransformBufferNode* currentCopyBuffer = vtkMRMLTransformBufferNode::New();
        currentCopyBuffer->Copy( this->ToolTrajectories.at( i ).Buffer );
        selfParentBuffer->Concatenate( currentCopyBuffer );
	    }
    }
	  parent = vtkMRMLLinearTransformNode::SafeDownCast( parent->GetParentTransformNode() );
  }

  return selfParentBuffer;
}



std::vector< std::string > vtkSlicerPerkEvaluatorLogic
::GetAllTransformRoles( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL )
  {
    return std::vector< std::string >();
  }

  // Use the python metrics calculator module
  qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();
  pythonManager->executeString( "import PythonMetricsCalculator" );
  pythonManager->executeString( "PythonMetricsCalculatorLogic = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogic.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( "PythonMetricsTransformRoles = PythonMetricsCalculatorLogic.GetAllTransformRoles()" );
  QVariant result = pythonManager->getVariable( "PythonMetricsTransformRoles" );
  QStringList transformRoles = result.toStringList();

  std::vector< std::string > transformRolesVector( transformRoles.length(), "" );
  for ( int i = 0; i < transformRoles.length(); i++ )
  {
    transformRolesVector.at( i ) = transformRoles.at( i ).toStdString();
  }
  return transformRolesVector;
}


void vtkSlicerPerkEvaluatorLogic
::GetSceneVisibleTransformNodes( vtkCollection* visibleTransformNodes )
{
  if ( visibleTransformNodes == NULL )
  {
    return;
  }
  visibleTransformNodes->RemoveAllItems();

  vtkSmartPointer< vtkCollection > transformNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLLinearTransformNode" );
  
  for ( int i = 0; i < transformNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNodes->GetItemAsObject( i ) );
    if ( transformNode != NULL && transformNode->GetHideFromEditors() == false )
    {
      visibleTransformNodes->AddItem( transformNode );
    }
  }
}


std::vector< std::string > vtkSlicerPerkEvaluatorLogic
::GetAllAnatomyRoles( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL )
  {
    return std::vector< std::string >();
  }

  // Use the python metrics calculator module
  qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();
  pythonManager->executeString( "import PythonMetricsCalculator" );
  pythonManager->executeString( "PythonMetricsCalculatorLogic = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogic.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( "PythonMetricsAnatomyRoles = PythonMetricsCalculatorLogic.GetAllAnatomyRoles()" );
  QVariant result = pythonManager->getVariable( "PythonMetricsAnatomyRoles" );
  QStringList anatomyRoles = result.toStringList();

  std::vector< std::string > anatomyRolesVector( anatomyRoles.length(), "" );
  for ( int i = 0; i < anatomyRoles.length(); i++ )
  {
    anatomyRolesVector.at( i ) = anatomyRoles.at( i ).toStdString();
  }
  return anatomyRolesVector;
}



void vtkSlicerPerkEvaluatorLogic
::GetSceneVisibleAnatomyNodes( vtkCollection* visibleAnatomyNodes )
{
  if ( visibleAnatomyNodes == NULL )
  {
    return;
  }
  visibleAnatomyNodes->RemoveAllItems();

  // Assume that all anatomy are either models or fiducials
  std::vector< std::string > anatomyNodeTypes;
  anatomyNodeTypes.push_back( "vtkMRMLModelNode" );
  anatomyNodeTypes.push_back( "vtkMRMLMarkupsFiducialNode" );
  // Add more node types here if it is necessary

  // We could allow all nodes, but then the user interface would be cluttered

  for ( int i = 0; i < anatomyNodeTypes.size(); i++ )
  {

    vtkSmartPointer< vtkCollection > nodes = this->GetMRMLScene()->GetNodesByClass( anatomyNodeTypes.at( i ).c_str() );  
    for ( int j = 0; j < nodes->GetNumberOfItems(); j++ )
    {
      vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast( nodes->GetItemAsObject( j ) );
      if ( currentNode != NULL && currentNode->GetHideFromEditors() == false )
      {
        visibleAnatomyNodes->AddItem( currentNode );
      }
    }

  }

}


double vtkSlicerPerkEvaluatorLogic
::GetTotalTime() const
{
  double minTime = this->GetMinTime();
  double maxTime = this->GetMaxTime();
  
  double totalTime = maxTime - minTime;
  
  if ( totalTime < 0 )
  {
    return 0.0;
  }
  else
  {
    return totalTime;
  }

}



double vtkSlicerPerkEvaluatorLogic
::GetMinTime() const
{
  if ( this->ToolTrajectories.size() == 0 )
  {
    return 0.0;
  }

  double minTime = std::numeric_limits< double >::max();
  
  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {
    if ( this->ToolTrajectories.at(i).Buffer->GetTransformAt(0)->GetTime() < minTime )
    {
      minTime = this->ToolTrajectories.at(i).Buffer->GetTransformAt(0)->GetTime();
    }
  }
  
  return minTime;
}



double vtkSlicerPerkEvaluatorLogic
::GetMaxTime() const
{
  if ( this->ToolTrajectories.size() == 0 )
  {
    return 0.0;
  }

  double maxTime = - std::numeric_limits< double >::max();
  
  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {
    if ( this->ToolTrajectories.at(i).Buffer->GetCurrentTransform()->GetTime() > maxTime )
    {
      maxTime = this->ToolTrajectories.at(i).Buffer->GetCurrentTransform()->GetTime();
    }
  }
  
  return maxTime;
}



double vtkSlicerPerkEvaluatorLogic
::GetPlaybackTime() const
{
  return this->PlaybackTime;
}



void vtkSlicerPerkEvaluatorLogic
::SetPlaybackTime( double time )
{
  if ( time < this->GetMinTime() || time > this->GetMaxTime() )
  {
    return;
  }
  
  this->PlaybackTime = time;  

  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {	
    vtkMRMLLinearTransformNode* node = this->ToolTrajectories.at(i).Node;
    std::string transformString = this->ToolTrajectories.at(i).Buffer->GetTransformAtTime( time )->GetTransform();

#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
    vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    transformMatrix->DeepCopy( MatrixStrToDouble( transformString ) );
    node->SetMatrixTransformToParent( transformMatrix );
#else
	node->GetMatrixTransformToParent()->DeepCopy( MatrixStrToDouble( transformString ) );
#endif

  }

}