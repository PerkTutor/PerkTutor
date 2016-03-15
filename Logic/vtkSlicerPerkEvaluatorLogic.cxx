
// PerkEvaluator Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTableNode.h"

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


//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkSlicerPerkEvaluatorLogic );


// Helper for converting QVariants holding string lists to std::vectors of std::strings
std::vector< std::string > QVariantToVector( QVariant variant )
{
  if ( ! variant.canConvert( QVariant::StringList ) )
  {
    return std::vector< std::string >();
  }

  QStringList stringList = variant.toStringList();

  std::vector< std::string > resultVector( stringList.length(), "" );
  for ( int i = 0; i < stringList.length(); i++ )
  {
    resultVector.at( i ) = stringList.at( i ).toStdString();
  }
  return resultVector;
}


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

  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::New();
  this->GetMRMLScene()->RegisterNodeClass( msNode );
  msNode->Delete();

  vtkMRMLMetricScriptStorageNode* mssNode = vtkMRMLMetricScriptStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( mssNode );
  mssNode->Delete();

  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::New();
  this->GetMRMLScene()->RegisterNodeClass( miNode );
  miNode->Delete();

  this->PythonManager = qSlicerApplication::application()->pythonManager();
  this->PythonManager->executeString( "import PythonMetricsCalculator" );
  this->PythonManager->executeString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.AddCoreMetricsToScene()" );
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
  if ( peNode == NULL || this->GetMRMLScene()->GetNodeByID( peNode->GetID() ) == NULL )
  {
    return;
  }
  if ( peNode->GetMarkBegin() > peNode->GetMarkEnd() ) // TODO: Is a test for to see if the MarkBegin and MarkEnd are within the bounds of the procedure time really necessary?
  {
    return;
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicComputeInstance = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicComputeInstance.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicComputeInstance.SetMetricsTableID( '%1' )" ).arg( peNode->GetMetricsTableID().c_str() ) );
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicComputeInstance.CalculateAllMetrics()" );

  peNode->GetMetricsTableNode()->Modified(); // Table has been modified
  peNode->GetMetricsTableNode()->StorableModified(); // Make sure the metrics table is saved by default
}


void vtkSlicerPerkEvaluatorLogic
::SetupRealTimeProcessing( vtkMRMLPerkEvaluatorNode* peNode )
{

  // Check conditions
  if ( peNode == NULL || this->GetMRMLScene()->GetNodeByID( peNode->GetID() ) == NULL )
  {
    return;
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicRealTimeInstance = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.SetPerkEvaluatorNodeID( '%1' )" ).arg( peNode->GetID() ) );
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.SetMetricsTableID( '%1' )" ).arg( peNode->GetMetricsTableID().c_str() ) );
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.allMetrics = PythonMetricsCalculatorLogicRealTimeInstance.GetFreshMetrics()" ) ); // Create the metrics
}


void vtkSlicerPerkEvaluatorLogic
::ObserveGlobalMetricInstances( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode->GetTransformBufferNode() == NULL )
  {
    return;
  }

  vtkCollection* metricInstanceNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricInstanceNode" );
  std::vector< std::string > activeTransformIDs = peNode->GetTransformBufferNode()->GetActiveTransformIDs();

  // Since all of the "global" metrics instances should already be added to the scene, we just need to find them
  for ( int i = 0; i < metricInstanceNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( metricInstanceNodes->GetItemAsObject( i ) );
    std::vector< std::string > transformRoles = this->GetAllRoles( miNode->GetAssociatedMetricScriptID(), vtkMRMLMetricInstanceNode::TransformRole );
    if ( transformRoles.size() != 1 )
    {
      continue;
    }

    for ( int j = 0; j < activeTransformIDs.size(); j++ )
    {      
      if ( miNode->GetRoleID( transformRoles.at( 0 ), vtkMRMLMetricInstanceNode::TransformRole ).compare( activeTransformIDs.at( j ) ) == 0 )
      {
        peNode->AddMetricInstanceID( miNode->GetID() );
      }
    }

  }
     
}


// This function is kind of akin to the old "role" system
void vtkSlicerPerkEvaluatorLogic
::SetMetricInstancesRolesToID( vtkMRMLPerkEvaluatorNode* peNode, std::string nodeID, std::string role, vtkMRMLMetricInstanceNode::RoleTypeEnum roleType )
{
  if ( peNode == NULL )
  {
    return;
  }

  // Iterate over all of the metric instance nodes used by the Perk Evaluator node
  std::vector< std::string > metricInstanceIDs = peNode->GetMetricInstanceIDs();
  for ( int i = 0; i < metricInstanceIDs.size(); i++ )
  {
    vtkMRMLMetricInstanceNode* metricInstanceNode = vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( metricInstanceIDs.at( i ) ) );
    if ( metricInstanceNode == NULL )
    {
      continue;
    }

    std::vector< std::string > allRoles = this->GetAllRoles( metricInstanceNode->GetAssociatedMetricScriptID(), roleType );

    for ( int j = 0; j < allRoles.size(); j++ )
    {
      if ( role.compare( allRoles.at( j ) ) == 0 )
      {
        metricInstanceNode->SetRoleID( nodeID, role, roleType ); // Only add if this role is accepted by the metric
      }
    }
    
  }
}


void vtkSlicerPerkEvaluatorLogic
::UpdateGlobalMetrics( vtkMRMLLinearTransformNode* transformNode )
{
  if ( transformNode == NULL )
  {
    return;
  }

  // Grab all metric scripts
  vtkCollection* metricScriptNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricScriptNode" );

  for ( int i = 0; i < metricScriptNodes->GetNumberOfItems(); i++ )
  {
    // Assume that we want to add any metric whose only transform role in "Any" and has no anatomy roles
    vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( metricScriptNodes->GetItemAsObject( i ) );
    if ( this->GetContext( msNode->GetID() ).compare( "Global" ) != 0 )
    {
      continue;
    }

    // Check that we only have one transform role
    std::vector< std::string > transformRoles = this->GetAllRoles( msNode->GetID(), vtkMRMLMetricInstanceNode::TransformRole );
    if ( transformRoles.size() != 1 )
    {
      continue;
    }

    // Create a metric instance node, with this transform serving the lone transform role
    vtkSmartPointer< vtkMRMLMetricInstanceNode > newMINode;
    newMINode.TakeReference( vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLMetricInstanceNode" ) ) );
    newMINode->SetScene( this->GetMRMLScene() );
	  this->GetMRMLScene()->AddNode( newMINode );
    newMINode->SetName( msNode->GetName() );
    newMINode->SetAssociatedMetricScriptID( msNode->GetID() );
    newMINode->SetRoleID( transformNode->GetID(), transformRoles.at( 0 ), vtkMRMLMetricInstanceNode::TransformRole );   
  }

}


void vtkSlicerPerkEvaluatorLogic
::UpdateGlobalMetrics( vtkMRMLMetricScriptNode* msNode )
{
  if ( msNode == NULL || this->GetContext( msNode->GetID() ).compare( "Global" ) != 0 )
  {
    return;
  }
  // Check that we only have one transform role
  std::vector< std::string > transformRoles = this->GetAllRoles( msNode->GetID(), vtkMRMLMetricInstanceNode::TransformRole );
  if ( transformRoles.size() != 1 )
  {
    return;
  }

  // Grab all transforms
  vtkCollection* transformNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLLinearTransformNode" );

  for ( int i = 0; i < transformNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNodes->GetItemAsObject( i ) );
    // Create a metric instance node, with this transform serving the lone transform role
    vtkSmartPointer< vtkMRMLMetricInstanceNode > newMINode;
    newMINode.TakeReference( vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLMetricInstanceNode" ) ) );
    newMINode->SetScene( this->GetMRMLScene() );
	  this->GetMRMLScene()->AddNode( newMINode );
    newMINode->SetName( msNode->GetName() );
    newMINode->SetAssociatedMetricScriptID( msNode->GetID() );
    newMINode->SetRoleID( transformNode->GetID(), transformRoles.at( 0 ), vtkMRMLMetricInstanceNode::TransformRole );     
  }

}


void vtkSlicerPerkEvaluatorLogic
::UpdateLocalMetrics( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL )
  {
    return;
  }

  // Grab all metric scripts
  vtkCollection* metricScriptNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricScriptNode" );

  for ( int i = 0; i < metricScriptNodes->GetNumberOfItems(); i++ )
  {
    // Assume that we want to add any metric whose only transform role in "Any" and has no anatomy roles
    vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( metricScriptNodes->GetItemAsObject( i ) );
    if ( this->GetContext( msNode->GetID() ).compare( "Local" ) != 0 )
    {
      continue;
    }

    // Create a metric instance node, with empty roles
    vtkSmartPointer< vtkMRMLMetricInstanceNode > newMINode;
    newMINode.TakeReference( vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLMetricInstanceNode" ) ) );
    newMINode->SetScene( this->GetMRMLScene() );
	  this->GetMRMLScene()->AddNode( newMINode );
    newMINode->SetName( msNode->GetName() );
    newMINode->SetAssociatedMetricScriptID( msNode->GetID() );
    peNode->AddMetricInstanceID( newMINode->GetID() );      
  }

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
::GetAllRoles( std::string msNodeID, vtkMRMLMetricInstanceNode::RoleTypeEnum roleType )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return std::vector< std::string >();
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicRolesInstance = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicRolesInstance.SetPerkEvaluatorNodeID( '' )" ); // Note that we don't actually need a PerkEvaluator node for this, but this is convenient for setting things up
  this->PythonManager->executeString( QString( "PythonMetricScriptRoles = PythonMetricsCalculatorLogicRolesInstance.GetAllRoles( '%1', %2 )" ).arg( msNodeID.c_str() ).arg( roleType ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptRoles" );

  return QVariantToVector( result );
}


std::string vtkSlicerPerkEvaluatorLogic
::GetAnatomyRoleClassName( std::string msNodeID, std::string role )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return "";
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicAnatomyInstance = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicAnatomyInstance.SetPerkEvaluatorNodeID( '' )" ); // Note that we don't actually need a PerkEvaluator node for this, but this is convenient for setting things up
  this->PythonManager->executeString( QString( "PythonMetricScriptAnatomyClassName = PythonMetricsCalculatorLogicAnatomyInstance.GetAnatomyRoleClassName( '%1', '%2' )" ).arg( msNodeID.c_str() ).arg( role.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptAnatomyClassName" );
  
  return result.toString().toStdString();
}



std::string vtkSlicerPerkEvaluatorLogic
::GetContext( std::string msNodeID )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return "";
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicContextInstance = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicContextInstance.SetPerkEvaluatorNodeID( '' )" ); // Note that we don't actually need a PerkEvaluator node for this, but this is convenient for setting things up
  this->PythonManager->executeString( QString( "PythonMetricScriptContext = PythonMetricsCalculatorLogicContextInstance.GetContext( '%1' )" ).arg( msNodeID.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptContext" );
  
  return result.toString().toStdString();
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

    vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    currentRecord->GetTransformMatrix( transformMatrix );
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

  // Update the metric instances for the node
  if ( peNode != NULL && event == vtkMRMLPerkEvaluatorNode::BufferActiveTransformsChangedEvent )
  {
    this->ObserveGlobalMetricInstances( peNode );
  }

  // Handle an event in the real-time processing
  if ( peNode != NULL && peNode->GetRealTimeProcessing() && event == vtkMRMLPerkEvaluatorNode::TransformRealTimeAddedEvent )
  {
    // The transform name
    std::string* transformName = reinterpret_cast< std::string* >( callData );
    // The time
    double absTime = peNode->GetTransformBufferNode()->GetTransformRecordBuffer( *transformName )->GetCurrentRecord()->GetTime();
    // Call the metrics update function
    this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.UpdateSelfAndChildMetrics( '%1', %2, True )" ).arg( transformName->c_str() ).arg( absTime ) );
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
    peNode->AddObserver( vtkMRMLPerkEvaluatorNode::BufferActiveTransformsChangedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
    // Create all of the necessary metric instances for all local metrics
    this->UpdateLocalMetrics( peNode );
  }

  // If a transform or metric script was added to the scene, make sure all transforms have all global metric instances
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && transformNode != NULL )
  {
    this->UpdateGlobalMetrics( transformNode );
  }
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && msNode != NULL )
  {
    this->UpdateGlobalMetrics( msNode );
  }

}