
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
  this->PythonManager->executeString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.Initialize()" );
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
  this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.CalculateAllMetrics( '%1' )" ).arg( peNode->GetID() ) );

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
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.SetupRealTimeMetricComputation( '%1' )" ).arg( peNode->GetID() ) );
}


void vtkSlicerPerkEvaluatorLogic
::ObserveGlobalMetricInstances( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode->GetTransformBufferNode() == NULL )
  {
    return;
  }

  // Hold off on modified events until we are done
  int modifyFlag = peNode->StartModify();

  vtkCollection* metricInstanceNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricInstanceNode" );
  std::vector< std::string > activeTransformIDs = peNode->GetTransformBufferNode()->GetActiveTransformIDs();

  // Since all of the "global" metrics instances should already be added to the scene, we just need to find them
  for ( int i = 0; i < metricInstanceNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( metricInstanceNodes->GetItemAsObject( i ) );
    
    std::vector< std::string > transformRoles = this->GetAllRoles( miNode->GetAssociatedMetricScriptID(), vtkMRMLMetricInstanceNode::TransformRole );
    bool oneTransformRole = transformRoles.size() == 1;
    bool globalContext = this->GetContext( miNode->GetAssociatedMetricScriptID() ).compare( "Global" ) == 0;
    if ( ! oneTransformRole || ! globalContext )
    {
      continue;
    }

    // Remove the metric instance (it will be added back if it should be)
    peNode->RemoveMetricInstanceID( miNode->GetID() );

    // Observe the metric instance if it matches one of the active transforms
    for ( int j = 0; j < activeTransformIDs.size(); j++ )
    {      
      if ( miNode->GetRoleID( transformRoles.at( 0 ), vtkMRMLMetricInstanceNode::TransformRole ).compare( activeTransformIDs.at( j ) ) == 0 )
      {
        peNode->AddMetricInstanceID( miNode->GetID() );
      }
    }

  }

  peNode->EndModify( modifyFlag );
     
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
::CreateGlobalMetric( vtkMRMLMetricScriptNode* msNode, vtkMRMLLinearTransformNode* transformNode, std::string transformRole )
{
  // The assumption is that we have already established that the context of the metric script is "Global"
  if ( msNode == NULL || transformNode == NULL )
  {
    return;
  }

  // Check it doesn't already exist in the scene
  vtkCollection* metricInstanceNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricInstanceNode" );

  for ( int i = 0; i < metricInstanceNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( metricInstanceNodes->GetItemAsObject( i ) );
    bool scriptsEqual = miNode->GetAssociatedMetricScriptNode()->IsEqual( msNode );
    bool oneTransformRole = this->GetAllRoles( msNode->GetID(), vtkMRMLMetricInstanceNode::TransformRole ).size() == 1;
    bool transformRoleIDsEqual = miNode->GetRoleID( transformRole, vtkMRMLMetricInstanceNode::TransformRole ).compare( transformNode->GetID() ) == 0;
    if ( scriptsEqual && oneTransformRole && transformRoleIDsEqual )
    {
      return; // Get out the the function. The metric already exists, so there is nothing to do here.
    }
  }

  // Create it and add it to the scene
  vtkSmartPointer< vtkMRMLMetricInstanceNode > newMINode;
  newMINode.TakeReference( vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLMetricInstanceNode" ) ) );
  newMINode->SetScene( this->GetMRMLScene() );
	this->GetMRMLScene()->AddNode( newMINode );
  newMINode->SetName( msNode->GetName() );
  newMINode->SetAssociatedMetricScriptID( msNode->GetID() );
  newMINode->SetRoleID( transformNode->GetID(), transformRole, vtkMRMLMetricInstanceNode::TransformRole );
  newMINode->SetSaveWithScene( false ); // Don't save the global instances with the scene, since they will be automatically recreated whenever necessary
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
   this->CreateGlobalMetric( msNode, transformNode, transformRoles.at( 0 ) );
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
  vtkSmartPointer< vtkCollection > transformNodes = vtkSmartPointer< vtkCollection >::New();
  this->GetSceneVisibleTransformNodes( transformNodes );

  for ( int i = 0; i < transformNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNodes->GetItemAsObject( i ) );
    // Create a metric instance node, with this transform serving the lone transform role
    this->CreateGlobalMetric( msNode, transformNode, transformRoles.at( 0 ) );   
  }

}


void vtkSlicerPerkEvaluatorLogic
::CreateLocalMetrics( vtkMRMLPerkEvaluatorNode* peNode )
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
    bool emptySource = msNode->GetPythonSourceCode().compare( "" ) == 0;
    bool localContext = this->GetContext( msNode->GetID() ).compare( "Local" ) == 0;
    if ( emptySource || ! localContext )
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


void vtkSlicerPerkEvaluatorLogic
::MergeMetricScripts( vtkMRMLMetricScriptNode* newMetricScriptNode )
{
  // If any metric script currently in the scene already has the same source code, then move all of its metric instances to the new node
  if ( newMetricScriptNode == NULL )
  {
    return;
  }

  vtkCollection* metricScriptNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricScriptNode" );

  for ( int i = 0; i < metricScriptNodes->GetNumberOfItems(); i++ )
  {
    // Assume that we want to add any metric whose only transform role in "Any" and has no anatomy roles
    vtkMRMLMetricScriptNode* currMetricScriptNode = vtkMRMLMetricScriptNode::SafeDownCast( metricScriptNodes->GetItemAsObject( i ) );
    bool sameNode = strcmp( currMetricScriptNode->GetID(), newMetricScriptNode->GetID() ) == 0;
    bool equalSource = currMetricScriptNode->IsEqual( newMetricScriptNode );
    bool emptySource = currMetricScriptNode->GetPythonSourceCode().compare( "" ) == 0;
    if ( sameNode || ! equalSource || emptySource )
    {
      continue;
    }

    std::vector< vtkMRMLNode* > referencingNodes;
    this->GetMRMLScene()->GetReferencingNodes( currMetricScriptNode, referencingNodes );
    for ( int j = 0; j < referencingNodes.size(); j++ )
    {
      referencingNodes.at( j )->UpdateReferenceID( currMetricScriptNode->GetID(), newMetricScriptNode->GetID() );
    }

    this->GetMRMLScene()->RemoveNode( currMetricScriptNode );    
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
  this->PythonManager->executeString( QString( "PythonMetricScriptRoles = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetAllRoles( '%1', %2 )" ).arg( msNodeID.c_str() ).arg( roleType ) );
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
  this->PythonManager->executeString( QString( "PythonMetricScriptAnatomyClassName = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetAnatomyRoleClassName( '%1', '%2' )" ).arg( msNodeID.c_str() ).arg( role.c_str() ) );
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
  this->PythonManager->executeString( QString( "PythonMetricScriptContext = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetContext( '%1' )" ).arg( msNodeID.c_str() ) );
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
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( caller );


  // Perk Evaluator Node
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
    this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.UpdateRealTimeMetrics( '%1', %2 )" ).arg( transformName->c_str() ).arg( absTime ) );
    // Make sure the widget is updated to reflect the updated metric values
    peNode->GetMetricsTableNode()->Modified();
  }

  // Metric Script Node
  // Update metrics whenever the source code is changed
  // Though, note that it is recommended only to set it once (and then never touch it again)
  if ( msNode != NULL && event == vtkMRMLMetricScriptNode::PythonSourceCodeChangedEvent )
  {
    this->MergeMetricScripts( msNode );
    this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.RefreshMetricModules()" ) );
    this->UpdateGlobalMetrics( msNode );
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
    msNode->AddObserver( vtkMRMLMetricScriptNode::PythonSourceCodeChangedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
  }

}