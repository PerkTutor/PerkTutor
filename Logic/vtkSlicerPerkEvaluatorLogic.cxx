
// PerkEvaluator Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"

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


//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkSlicerPerkEvaluatorLogic );


// Constructors and Desctructors ----------------------------------------------

vtkSlicerPerkEvaluatorLogic
::vtkSlicerPerkEvaluatorLogic()
{
}



vtkSlicerPerkEvaluatorLogic::
~vtkSlicerPerkEvaluatorLogic()
{
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

  // TODO: Remove when table nodes integrated into Slicer core
  vtkMRMLTableNode* tNode = vtkMRMLTableNode::New();
  this->GetMRMLScene()->RegisterNodeClass( tNode );
  tNode->Delete();
  vtkMRMLTableStorageNode* tsNode = vtkMRMLTableStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( tsNode );
  tsNode->Delete();
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


void vtkSlicerPerkEvaluatorLogic
::ComputeMetrics( vtkMRMLPerkEvaluatorNode* peNode )
{

  // Check conditions
  if ( peNode == NULL )
  {
    return;
  }
  if ( peNode->GetMarkBegin() > peNode->GetMarkEnd() ) // TODO: Is a test for to see if the MarkBegin and MarkEnd are within the bounds of the procedure time really necessary?
  {
    return;
  }

  // Use the python metrics calculator module
  qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();
  pythonManager->executeString( "import PythonMetricsCalculator" );
  pythonManager->executeString( "PythonMetricsCalculatorLogicCompute = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogicCompute.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogicCompute.SetMetricsTableID( '%1' )" ).arg( peNode->GetMetricsTableID().c_str() ) );
  pythonManager->executeString( "PythonMetricsCalculatorLogicCompute.CalculateAllMetrics()" );

  peNode->GetMetricsTableNode()->Modified(); // Table has been modified
  peNode->GetMetricsTableNode()->StorableModified(); // Make sure the metrics table is saved by default
}


void vtkSlicerPerkEvaluatorLogic
::SetupRealTimeProcessing( vtkMRMLPerkEvaluatorNode* peNode )
{

  // Check conditions
  if ( peNode == NULL )
  {
    return;
  }

  // Use the python metrics calculator module
  qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();
  pythonManager->executeString( "import PythonMetricsCalculator" );
  pythonManager->executeString( "PythonMetricsCalculatorLogicRealTime = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTime.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTime.SetMetricsTableID( '%1' )" ).arg( peNode->GetMetricsTableID().c_str() ) );
}


bool vtkSlicerPerkEvaluatorLogic
::IsSelfOrDescendentTransformNode( vtkMRMLLinearTransformNode* parent, vtkMRMLLinearTransformNode* child )
{
  while( child != NULL )
  {
    if ( strcmp( parent->GetID(), child->GetID() ) == 0 )
    {
      return true;
    }
    child = vtkMRMLLinearTransformNode::SafeDownCast( child->GetParentTransformNode() );
  }

  return false;
}


void vtkSlicerPerkEvaluatorLogic
::GetSelfAndParentRecordBuffer( vtkMRMLPerkEvaluatorNode* peNode, vtkMRMLLinearTransformNode* transformNode, vtkLogRecordBuffer* selfParentRecordBuffer )
{
  // TODO: We only care about this for times. Is there a more efficient way to do this?
  selfParentRecordBuffer->Clear();

  // Iterate through the parents and add to temporary transform buffer if in the selected transform buffer for analysis
  if ( peNode == NULL || peNode->GetTransformBufferNode() == NULL )
  {
    return;
  }

  std::vector< std::string > recordedTransformNames = peNode->GetTransformBufferNode()->GetAllRecordedTransformNames();

  vtkMRMLLinearTransformNode* parent = transformNode;
  while( parent != NULL )
  {

    // Check if the parent's name matches one of the trajectory names
    for ( int i = 0; i < recordedTransformNames.size(); i++ )
    {
      if ( recordedTransformNames.at( i ).compare( parent->GetName() ) == 0 )
	    {
        // Concatenate into the record buffer if so. Note: No need to deep copy - the times are really all we need
        selfParentRecordBuffer->Concatenate( peNode->GetTransformBufferNode()->GetTransformRecordBuffer( recordedTransformNames.at( i ) ) );
	    }
    }

	  parent = vtkMRMLLinearTransformNode::SafeDownCast( parent->GetParentTransformNode() );
  }

}


void vtkSlicerPerkEvaluatorLogic
::GetSelfAndParentTimes( vtkMRMLPerkEvaluatorNode* peNode, vtkMRMLLinearTransformNode* transformNode, vtkDoubleArray* timesArray )
{
  // TODO: We only care about this for times. Is there a more efficient way to do this?
  vtkSmartPointer< vtkLogRecordBuffer > selfParentRecordBuffer = vtkSmartPointer< vtkLogRecordBuffer >::New();
  this->GetSelfAndParentRecordBuffer( peNode, transformNode, selfParentRecordBuffer );

  // Now, just grab the times
  timesArray->SetNumberOfComponents( 1 );
  timesArray->SetNumberOfTuples( selfParentRecordBuffer->GetNumRecords() );
  
  for ( int i = 0; i < selfParentRecordBuffer->GetNumRecords(); i++ )
  {
    timesArray->SetValue( i, selfParentRecordBuffer->GetRecord( i )->GetTime() );
  }
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
  pythonManager->executeString( "PythonMetricsCalculatorLogicTransformRoles = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogicTransformRoles.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( "PythonMetricsTransformRoles = PythonMetricsCalculatorLogicTransformRoles.GetAllTransformRoles()" );
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
  pythonManager->executeString( "PythonMetricsCalculatorLogicAnatomyRoles = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  pythonManager->executeString( QString( "PythonMetricsCalculatorLogicAnatomyRoles.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  pythonManager->executeString( "PythonMetricsAnatomyRoles = PythonMetricsCalculatorLogicAnatomyRoles.GetAllAnatomyRoles()" );
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



void vtkSlicerPerkEvaluatorLogic
::UpdateSceneToPlaybackTime( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL || peNode->GetTransformBufferNode() == NULL )
  {
    return;
  }
  
  std::vector< std::string > recordedTransformNames = peNode->GetTransformBufferNode()->GetAllRecordedTransformNames();

  for ( int i = 0; i < recordedTransformNames.size(); i++ )
  {	
    // Find the linear transform node assicated with the transform name
    vtkMRMLLinearTransformNode* linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( recordedTransformNames.at( i ).c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( linearTransformNode == NULL )
    {
      continue;
    }

    vtkTransformRecord* currentRecord = peNode->GetTransformBufferNode()->GetTransformAtTime( peNode->GetPlaybackTime(), recordedTransformNames.at( i ) );
    if ( currentRecord == NULL )
    {
      continue;
    }

    std::string matrixString = currentRecord->GetTransformString();

    vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    PerkTutorCommon::MatrixStringTo4x4( matrixString, transformMatrix );
    linearTransformNode->SetMatrixTransformToParent( transformMatrix );
  }

}


// Get/Set playback for node -----------------------------------------------------------------------

double vtkSlicerPerkEvaluatorLogic
::GetRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL )
  {
    return 0.0;
  }
  if ( peNode->GetTransformBufferNode() == NULL )
  {
    return peNode->GetPlaybackTime();
  }

  return peNode->GetPlaybackTime() - peNode->GetTransformBufferNode()->GetMinimumTime();
}

void vtkSlicerPerkEvaluatorLogic
::SetRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode, double time )
{
  if ( peNode == NULL )
  {
    return;
  }
  if ( peNode->GetTransformBufferNode() == NULL )
  {
    peNode->SetPlaybackTime( time );
    return;
  }

  peNode->SetPlaybackTime( time + peNode->GetTransformBufferNode()->GetMinimumTime() );
}


double vtkSlicerPerkEvaluatorLogic
::GetMaximumRelativePlaybackTime( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL )
  {
    return 0.0;
  }
  if ( peNode->GetTransformBufferNode() == NULL )
  {
    return 0.0;
  }

  return peNode->GetTransformBufferNode()->GetTotalTime();
}


// Node update methods ----------------------------------------------------------

void vtkSlicerPerkEvaluatorLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( caller );

  // The caller must be a vtkMRMLPerkEvaluatorNode

  // Setup the real-time processing
  if ( peNode != NULL && event == vtkMRMLPerkEvaluatorNode::RealTimeProcessingStartedEvent )
  {
    this->SetupRealTimeProcessing( peNode );
  }

  // Handle an event in the real-time processing
  if ( peNode != NULL && peNode->GetRealTimeProcessing() && event == vtkMRMLPerkEvaluatorNode::TransformRealTimeAddedEvent )
  {
    // The transform name
    std::string* transformName = reinterpret_cast< std::string* >( callData );
    // The time
    double absTime = peNode->GetTransformBufferNode()->GetTransformRecordBuffer( *transformName )->GetCurrentRecord()->GetTime();
    // Call the metrics update function
    qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager(); // This is a constant each time the function is called
    pythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTime.UpdateSelfAndChildMetrics( '%1', %2 )" ).arg( transformName->c_str() ).arg( absTime ) );
    // Make sure the widget is updated to reflect the updated metric values
    peNode->GetMetricsTableNode()->Modified();
  }
}


void vtkSlicerPerkEvaluatorLogic
::ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLScene* callerNode = vtkMRMLScene::SafeDownCast( caller );

  // If the added node was a perk evaluator node then observe it
  vtkMRMLNode* addedNode = reinterpret_cast< vtkMRMLNode* >( callData );
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && peNode != NULL )
  {
    // Observe if a real-time transform event is added
    peNode->AddObserver( vtkMRMLPerkEvaluatorNode::TransformRealTimeAddedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
    peNode->AddObserver( vtkMRMLPerkEvaluatorNode::RealTimeProcessingStartedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
  }

}


// TODO: THIS SHOULD BE REMOVED WHEN vtkMRMLTableNode is properly added to Slicer
vtkMRMLTableNode* vtkSlicerPerkEvaluatorLogic
::AddTable(const char* fileName, const char* name)
{
  if (this->GetMRMLScene() == 0 || fileName == 0)
    {
    return 0;
    }

  // Storage node
  vtkNew<vtkMRMLTableStorageNode> tableStorageNode;
  tableStorageNode->SetFileName(fileName);
  this->GetMRMLScene()->AddNode(tableStorageNode.GetPointer());

  // Storable node
  vtkNew<vtkMRMLTableNode> tableNode;
  this->GetMRMLScene()->AddNode(tableNode.GetPointer());

  // Read
  int res = tableStorageNode->ReadData(tableNode.GetPointer());
  if (res == 0) // failed to read
    {
    this->GetMRMLScene()->RemoveNode(tableStorageNode.GetPointer());
    this->GetMRMLScene()->RemoveNode(tableNode.GetPointer());
    return 0;
    }
  if (name)
    {
    tableNode->SetName(name);
    }
  return tableNode.GetPointer();
}